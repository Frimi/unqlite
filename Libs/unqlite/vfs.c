/***************************************************************************************************
 * @brief     VFS for unQLite.
 * @details   Tab == 4 spaces (use Tab char instead of spaces).
 * @author    Vinicius Diori
 * @date      12/2023
 ***************************************************************************************************/

/***************************************************************************************************
* Includes
***************************************************************************************************/
// Library
#include <string.h>

// System
#include "stm32u575xx.h"
#include "main.h"

// Software
#include "vfs.h"
#include "vfsRaw.h"


/***************************************************************************************************
* Externs
***************************************************************************************************/
extern const unqlite_io_methods rawIoMethod;

/***************************************************************************************************
* Vars
***************************************************************************************************/

/***************************************************************************************************
* Prototypes
***************************************************************************************************/

/***************************************************************************************************
* @brief         Open a file
* @param[in,out] The VFS for which this is the xOpen method
* @param[in]     Pathname of file to be opened (UTF-8)
* @param[in,out] Write the SQLite file handle here
* @param[in]     Open mode flags
* @param[out]    Status return flags
* @return        UNQLITE_OK or some other error code on failure
***************************************************************************************************/
static int rawOpen(unqlite_vfs *pVfs, const char *zName, unqlite_file *id, int flags)
{
	rawFile *pFile = (rawFile*)id;
	FS_FILE *h; /* File descriptor returned by open() */
    int rc = UNQLITE_OK; /* Function Return Code */
    char openFlags[3]; /* Flags to pass to open() */

    int isExclusive = (flags & UNQLITE_OPEN_EXCLUSIVE);
    int isDelete = (flags & UNQLITE_OPEN_TEMP_DB);
    int isCreate = (flags & UNQLITE_OPEN_CREATE);
    int isReadonly = (flags & UNQLITE_OPEN_READONLY);
    int isReadWrite = (flags & UNQLITE_OPEN_READWRITE);

	VFS_DEBUG_MSG("isReadonly: %s\n", isReadonly ? "1" : "0");
	VFS_DEBUG_MSG("isReadWrite: %s\n", isReadWrite ? "1" : "0");
	VFS_DEBUG_MSG("isCreate: %s\n", isCreate ? "1" : "0");
	VFS_DEBUG_MSG("isDelete: %s\n", isDelete ? "1" : "0");
	VFS_DEBUG_MSG("isExclusive: %s\n", isExclusive ? "1" : "0");

	char zTmpname[MAXPATHNAME + 2];

	/* Check the following statements are true:
	 **
	 **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
	 **   (b) if CREATE is set, then READWRITE must also be set, and
	 **   (c) if EXCLUSIVE is set, then CREATE must also be set.
	 **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
	 */
	assert((isReadonly == 0 || isReadWrite == 0) && (isReadWrite || isReadonly));
	assert(isCreate == 0 || isReadWrite);
	assert(isExclusive == 0 || isCreate);
	assert(isDelete == 0 || isCreate);

	if (!zName)
	{
		rc = osGetTempname(MAXPATHNAME + 2, zTmpname);
		if (rc != UNQLITE_OK)
		{

			VFS_DEBUG_MSG("OPEN name=%s, rc=UNQLITE_IOERR\n", zName);
			return UNQLITE_IOERR;
		}
		zName = zTmpname;
	}

	if (isReadWrite)
	{
		strcpy(openFlags, "r+");
	}
	else
	{
		strcpy(openFlags, "r");
	}

	if (isExclusive)
	{
		strcpy(openFlags, "a");
	}
	else if (isCreate)
	{
		strcpy(openFlags, "a+");
	}
	else
	{
		strcpy(openFlags, "r+");
	}
	VFS_DEBUG_MSG("OPEN name=%s, pFile=%p, flags=%x\n", zName, id, flags);

	h = fs_fopen(zName, openFlags);
	if (h == NULL)
	{
		VFS_DEBUG_MSG("OPEN name=%s, CANTOPEN\n", zName);
		return UNQLITE_CANTOPEN;
	}
	else
	{
		fs_fclose(h);
		h = fs_fopen(zName, "r+");
		if (h == NULL)
		{
			VFS_DEBUG_MSG("OPEN name=%s, CANTOPEN\n", zName);
			return UNQLITE_CANTOPEN;
		}
	}

	VFS_DEBUG_MSG("OPEN name=%s, access=%s\n", zName, openFlags);

	memset(pFile, 0, sizeof(rawFile));
	pFile->pMethod = &rawIoMethod;
	pFile->pVfs = pVfs;
	pFile->h = h;
	pFile->zPath = zName;
	if (isReadonly)
	{
		pFile->ctrlFlags |= RAWFILE_RDONLY;
	}

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief     Delete the named file
* @param     sqlite3_vfs* Not used
* @param[in] char* Name of file to delete (UTF-8)
* @param     int Not used
* @return    SQLITE_OK or some other error code on failure
***************************************************************************************************/
static int rawDelete(unqlite_vfs *pVfs,const char *zFilename, int syncDir)
{
	int rc;
	// Delete file
	rc = fs_remove(zFilename);
	if (rc == 0)
	{
		VFS_DEBUG_MSG("DELETE name=%s, UNQLITE_OK\n", zFilename);
		return UNQLITE_OK;
	}
	else
	{
		VFS_DEBUG_MSG("DELETE name=%s, UNQLITE_IOERR\n", zFilename);
		return UNQLITE_IOERR;
	}
}

/***************************************************************************************************
* @brief      Test the existence of or access permissions of file zPath
* @param      sqlite3_vfs* Not used on
* @param[in]  char* Name of file to check (UTF-8)
* @param[in]  int Type of test to make on this file
* @param[out] int* Result
* @return     SQLITE_OK
***************************************************************************************************/
static int rawAccess(unqlite_vfs *pVfs, const char *zFilename, int flags, int *pResOut)
{
	int rc;
	struct fs_stat p_info;

	VFS_DEBUG_MSG("ACCESS name=%s, flags=%x, pResOut=%p\n", zFilename, flags, pResOut);

	rc = fs_stat(zFilename, &p_info);
	*pResOut = (rc == 0) ? 1 : 0;
	VFS_DEBUG_MSG("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n", zFilename, pResOut, *pResOut);

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Turn a relative pathname into a full pathname.
* @details       Write the full pathname into zOut[].
*                zOut[] will be at least pVfs->mxPathname bytes in size.
* @param[in,out] sqlite3_vfs* Pointer to vfs object
* @param[in]     char* Possibly relative input path
* @param[in]     int Size of output buffer in bytes
* @param[out]    char* Output nbuffer
* @return        SQLITE_OK
***************************************************************************************************/
static int rawFullPathname(unqlite_vfs *pVfs, const char *zPath, int nPathOut, char *zPathOut)
{
	sprintf(zPathOut, "\\%s", zPath);
	zPathOut[nPathOut-1] = '\0';

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Sleep for a little while.  Return the amount of time slept.
* @param[in,out] sqlite3_vfs* Pointer to vfs object
* @param[in]     int sleep delay in microseconds
* @return        SQLITE_OK
***************************************************************************************************/
static int rawSleep(unqlite_vfs *pVfs, int microsec)
{
	int milliseconds = (microsec + 999) / 1000;
	HAL_Delay(milliseconds);
	return milliseconds * 1000;
}

/***************************************************************************************************
* @brief         Find the current time (in Universal Coordinated Time)
* @details       Write the current time and date as a Julian Day number into *prNow and return 0.
* @param[in,out] sqlite3_vfs* Pointer to vfs object
* @param[out]    double* prNow
* @return        SQLITE_OK
* @return        Return 1 if the time and date cannot be found.
***************************************************************************************************/
static int rawCurrentTime(unqlite_vfs *pVfs, double *prNow)
{
	uint32_t ts_unix = Clk_GetTS();
	*prNow = ts_unix;

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief      Initializes the SQLite library
* @return     SQLITE_OK on success
***************************************************************************************************/
UNQLITE_PRIVATE const unqlite_vfs *unqliteExportBuiltinVfs(void)
{
    static unqlite_vfs MicriumFS_VFS =
        {
            "MicriumFS",
            1,
            sizeof(rawFile),
            MAXPATHNAME,
            rawOpen,
            rawDelete,
            rawAccess,
            rawFullPathname,
            0,
            rawSleep,
            rawCurrentTime,
            0,
        };
    return &MicriumFS_VFS;
}
