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
#include <time.h>
#include <stdio.h>
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
* @param[in,out] Write the unqlite_file handle here
* @param[in]     Open mode flags
* @param[out]    Status return flags
* @return        UNQLITE_OK or some other error code on failure
***************************************************************************************************/
static int rawOpen(unqlite_vfs *pVfs, const char *zName, unqlite_file *id, unsigned int flags)
{
	rawFile *pFile = (rawFile*)id;
	FS_FILE *h; /* File descriptor returned by open() */
    char openFlags[3]; /* Flags to pass to open() */

    int isExclusive = (flags & UNQLITE_OPEN_EXCLUSIVE);
    int isDelete = (flags & UNQLITE_OPEN_TEMP_DB);
    int isCreate = (flags & UNQLITE_OPEN_CREATE);
    int isReadonly = (flags & UNQLITE_OPEN_READONLY);
    int isReadWrite = (flags & UNQLITE_OPEN_READWRITE);
	int isWholeFileMap = (flags & UNQLITE_OPEN_MMAP);
	int	isMemoryDatabase = (flags & UNQLITE_OPEN_IN_MEMORY);
	int isOmitJournaling = (flags & UNQLITE_OPEN_OMIT_JOURNALING);
	int isNoMutex = (flags & UNQLITE_OPEN_NOMUTEX);

	VFS_DEBUG_MSG("isReadonly: %s\n", isReadonly ? "1" : "0");
	VFS_DEBUG_MSG("isReadWrite: %s\n", isReadWrite ? "1" : "0");
	VFS_DEBUG_MSG("isCreate: %s\n", isCreate ? "1" : "0");
	VFS_DEBUG_MSG("isDelete: %s\n", isDelete ? "1" : "0");
	VFS_DEBUG_MSG("isExclusive: %s\n", isExclusive ? "1" : "0");
	VFS_DEBUG_MSG("isWholeFileMap: %s\n", isWholeFileMap ? "1" : "0");
	VFS_DEBUG_MSG("isMemoryDatabase: %s\n", isMemoryDatabase ? "1" : "0");
	VFS_DEBUG_MSG("isOmitJournaling: %s\n", isOmitJournaling ? "1" : "0");
	VFS_DEBUG_MSG("isNoMutex: %s\n", isNoMutex ? "1" : "0");

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
		VFS_DEBUG_MSG("OPEN name=%s, rc=UNQLITE_IOERR\n", zName);
		return UNQLITE_IOERR;
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

	if(isCreate)
	{
		h = fs_fopen(zName, openFlags);
		VFS_DEBUG_MSG("OPEN name=%s, access=%s\n", zName, openFlags);
		if (h == NULL)
		{
			VFS_DEBUG_MSG("OPEN name=%s, CANTOPEN\n", zName);
			return UNQLITE_CANTOPEN;
		}
		else
		{
			fs_fclose(h);
			h = fs_fopen(zName, "r+");
			VFS_DEBUG_MSG("OPEN name=%s, access=%s\n", zName, openFlags);
			if (h == NULL)
			{
				VFS_DEBUG_MSG("OPEN name=%s, CANTOPEN\n", zName);
				return UNQLITE_CANTOPEN;
			}

		}
	}
	else
	{
		h = fs_fopen(zName, openFlags);
		VFS_DEBUG_MSG("OPEN name=%s, access=%s\n", zName, openFlags);
		if (h == NULL)
		{
			VFS_DEBUG_MSG("OPEN name=%s, CANTOPEN\n", zName);
			return UNQLITE_CANTOPEN;
		}
	}

	VFS_DEBUG_MSG("OPEN name=%s, pFile=%p, flags=%x\n", zName, id, flags);

	memset(pFile, 0, sizeof(rawFile));
	pFile->pMethod = &rawIoMethod;
	pFile->pVfs = pVfs;
	pFile->h = h;
	pFile->zPath = zName;

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief     Delete the named file
* @param     unqlite_vfs* Not used
* @param[in] char* Name of file to delete (UTF-8)
* @param     int Not used
* @return    UNQLITE_OK or some other error code on failure
***************************************************************************************************/
static int rawDelete(unqlite_vfs *pVfs,const char *zFilename, int syncDir)
{
	int rc;
	// Delete file
	rc = fs_remove(zFilename);
	if (rc == 0)
	{
		return UNQLITE_OK;
	}
	else
	{
		VFS_DEBUG_MSG("rawDelete: UNQLITE_IOERR zFilename: %s, syncDir: %d\n",zFilename, syncDir );
		return UNQLITE_IOERR;
	}
}

/***************************************************************************************************
* @brief      Test the existence of or access permissions of file zPath
* @param      unqlite_vfs* Not used on
* @param[in]  char* Name of file to check (UTF-8)
* @param[in]  int Type of test to make on this file
* @param[out] int* Result
* @return     UNQLITE_OK
***************************************************************************************************/
static int rawAccess(unqlite_vfs *pVfs, const char *zFilename, int flags, int *pResOut)
{
	int rc;
	struct fs_stat p_info;

	rc = fs_stat(zFilename, &p_info);
	*pResOut = (rc == 0) ? 1 : 0;

	VFS_DEBUG_MSG("RawAccess: %d\r\n", *pResOut);

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Turn a relative pathname into a full pathname.
* @details       Write the full pathname into zOut[].
*                zOut[] will be at least pVfs->mxPathname bytes in size.
* @param[in,out] unqlite_vfs* Pointer to vfs object
* @param[in]     char* Possibly relative input path
* @param[in]     int Size of output buffer in bytes
* @param[out]    char* Output nbuffer
* @return        UNQLITE_OK
***************************************************************************************************/
static int rawFullPathname(unqlite_vfs *pVfs, const char *zPath, int nPathOut, char *zPathOut)
{
	sprintf(zPathOut, "\\%s", zPath);
	zPathOut[nPathOut-1] = '\0';

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Sleep for a little while.  Return the amount of time slept.
* @param[in,out] unqlite_vfs* Pointer to vfs object
* @param[in]     int sleep delay in microseconds
* @return        sleep time.
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
* @param[in,out] unqlite_vfs* Pointer to vfs object
* @param[out]    double* prNow
* @return        UNQLITE_OK
* @return        Return 1 if the time and date cannot be found.
***************************************************************************************************/
static int rawCurrentTime(unqlite_vfs *pVfs, Sytm *prNow)
{
	struct tm structTM;
	time_t timestamp = Clk_GetTS();
	gmtime_r(&timestamp, &structTM);

	prNow->tm_sec = structTM.tm_sec;
	prNow->tm_min = structTM.tm_min;
	prNow->tm_hour = structTM.tm_hour;
	prNow->tm_mday = structTM.tm_mday;
	prNow->tm_mon = structTM.tm_mon;
	prNow->tm_year = structTM.tm_year;
	prNow->tm_wday = structTM.tm_wday;
	prNow->tm_yday = structTM.tm_yday;
	prNow->tm_isdst = structTM.tm_isdst;
  	prNow->tm_zone = "GMT-3";
  	prNow->tm_gmtoff = timestamp;

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief      Initializes the unqlite library
* @return     UNQLITE_OK on success
***************************************************************************************************/
const unqlite_vfs * unqliteExportBuiltinVfs(void)
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
