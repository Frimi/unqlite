/***************************************************************************************************
 * @brief     VFS Raw for unQLite.
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
#include "vfsRaw.h"

/***************************************************************************************************
* Externs
***************************************************************************************************/

/***************************************************************************************************
* Vars
***************************************************************************************************/

/***************************************************************************************************
* Prototypes
***************************************************************************************************/

const unqlite_io_methods rawIoMethod =
{
    1,
    rawClose,
    rawRead,
    rawWrite,
    rawTruncate,
    rawSync,
    rawFileSize,
    rawLock,
    rawUnlock,
    rawCheckReservedLock,
    rawSectorSize
};

/***************************************************************************************************
 * @brief Close a File.
 *
 * @param id - File to close.
 * @return int - ERROR Code.
***************************************************************************************************/
int rawClose(unqlite_file *id)
{
    int rc = UNQLITE_OK;
    if (id)
    {
        rawFile *pFile = (rawFile *)id;
        rc = fs_fclose(pFile->h);
        if (rc == 0)
        {
            pFile->h = NULL;
        }
    }

    return (rc == 0) ? UNQLITE_OK : UNQLITE_IOERR;
}

/***************************************************************************************************
 * @brief Read data from a file into a buffer
 *
 * @param id  File to read from
 * @param pBuf Write content into this buffer
 * @param amt Number of bytes to read
 * @param offset Begin reading at this offset
 * @return int ERROR CODE.
***************************************************************************************************/
int rawRead(unqlite_file *id, void *pBuf, unqlite_int64 amt, unqlite_int64 offset)
{
	int rc;
	rawFile *pFile = (rawFile*)id;
	fs_size_t itemsRead;

	VFS_DEBUG_START();

	uint32_t fileSize = 0;
	struct fs_stat info;
	fs_fstat(pFile->h, &info);
	fileSize = info.st_size;
	if (offset > fileSize)
	{
		VFS_DEBUG_MSG("\nSEEK over EOF fileSize=%lu, offset=%lld\n\n", fileSize, offset);
	}

    // Positions the cursor only if the file size is greater than the chosen offset.
    // This behavior is necessary because fs_fseek modifies the file size if
    // a seek beyond the end of the file is performed.
	if (offset < fileSize)
	{
		rc = fs_fseek(pFile->h, (long int)offset, SEEK_SET);
		if (rc != 0)
		{
			VFS_DEBUG_MSG("READ file=%p, rc=UNQLITE_FULL\n", pFile->h);
			return UNQLITE_FULL;
		}
	}

	itemsRead = fs_fread(pBuf, 1, amt, pFile->h);

	if (itemsRead < (long int)amt)
	{
		/* Unread parts of the buffer must be zero-filled */
		memset(&((char*)pBuf)[itemsRead], 0, amt - itemsRead);
		VFS_DEBUG_FINALIZE("READ file=%p, rc=UNQLITE_IOERR -> ", pFile->h);

		return UNQLITE_IOERR;
	}

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Write data from a buffer into a file.
* @param[in,out] File to write into
* @param[out]    The bytes to be written
* @param[in]     Number of bytes to write
* @param[in]     Offset into the file to begin writing at
* @return        ERROR CODE
***************************************************************************************************/
int rawWrite(unqlite_file *id, const void *pBuf, unqlite_int64 amt, unqlite_int64 offset)
{
	int rc;
	rawFile *pFile = (rawFile*)id;
	fs_size_t wrote = 0;

	VFS_DEBUG_START();
	uint32_t fileSize = 0;
	struct fs_stat info;
	fs_fstat(pFile->h, &info);
	fileSize = info.st_size;
	if (offset > fileSize)
	{
		VFS_DEBUG_MSG("\nSEEK over EOF fileSize=%lu, offset=%lld\n\n", fileSize, offset);
	}

	/* seek pos */
	rc = fs_fseek(pFile->h, (long int)offset, SEEK_SET);
	if (rc != 0)
	{
		VFS_DEBUG_FINALIZE("WRITE file=%p, rc=UNQLITE_FULL\n", pFile->h);
		return UNQLITE_FULL;
	}

	VFS_DEBUG_RESTART();

	/* do write file */
	wrote = fs_fwrite(pBuf, 1, amt, pFile->h);
	if (rc != 0)
	{
		VFS_DEBUG_FINALIZE("rc != 0 UNQLITE_IOERR\n", pFile->h);
		return UNQLITE_IOERR;
	}

	if ((amt > (int)wrote))
	{
		VFS_DEBUG_FINALIZE("(amt > (int)wrote) rc=UNQLITE_FULL\n", pFile->h);
		return UNQLITE_FULL;
	}

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Truncate an open file to a specified size
* @param[in,out] File to truncate
* @param[in]     number of bytes to truncate
* @return        ERROR CODE
***************************************************************************************************/
int rawTruncate(unqlite_file *id, unqlite_int64 nByte)
{
	rawFile *pFile = (rawFile*)id;
	int rc;

	VFS_DEBUG_MSG("TRUNCATE file=%p, size=%lld\n", pFile->h, nByte);

	/* If the user has configured a chunk-size for this file, truncate the
	 ** file so that it consists of an integer number of chunks (i.e. the
	 ** actual file size after the operation may be larger than the requested
	 ** size).
	 */
	if (pFile->szChunk > 0)
	{
		nByte = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
	}

	rc = fs_fseek(pFile->h, (long int)nByte, SEEK_SET);
	if (rc != 0)
	{
		return UNQLITE_IOERR;
	}

	rc = fs_ftruncate(pFile->h, (fs_off_t)nByte);
	if (rc != 0)
	{
		return UNQLITE_IOERR;
	}

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Make sure all writes to a particular file are committed to disk.
* @param[in,out] File to sync
* @param[in]     int FileSystem flags
* @return        ERROR CODE
***************************************************************************************************/
int rawSync(unqlite_file *id, int flags)
{
//	if( fs_cache_flush() )
//	{
//		return UNQLITE_IOERR;
//	}
//	if( fs_cache_assign() )
//	{
//		return UNQLITE_IOERR;
//	}
	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         Determine the current size of a file in bytes
* @param[in,out] File pointer to file struct.
* @param[out]    pointer to return File Size
* @return        ERROR CODE
***************************************************************************************************/
int rawFileSize(unqlite_file *id, unqlite_int64 *pSize)
{
	rawFile *pFile = (rawFile*)id;
	struct fs_stat info;

	if( fs_fstat(pFile->h, &info) != 0 )
	{
		VFS_DEBUG_MSG("fs_fstat(pFile->h, &info) != 0");
		return UNQLITE_IOERR;
	}
	*pSize = info.st_size;

	VFS_DEBUG_MSG("SIZE file=%p, pSize=%p\n", pFile->h, pSize);

	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief  Locking function. This method are both no-ops.
* @return UNQLITE_OK
***************************************************************************************************/
int rawLock(unqlite_file *id, int eLock)
{
	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief  Unlocking function. This method are both no-ops.
* @return UNQLITE_OK
***************************************************************************************************/
int rawUnlock(unqlite_file *id, int eLock)
{
	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief         The xCheckReservedLock() always indicates that no other process holds
*                a reserved lock on the database file. This ensures that if a hot-journal
*                file is found in the file-system it is rolled back.
* @param[in,out] File Pointer
* @param[out]    pResOut
* @return        UNQLITE_OK
***************************************************************************************************/
int rawCheckReservedLock(unqlite_file *id, int *pResOut)
{
	*pResOut = 0;
	return UNQLITE_OK;
}

/***************************************************************************************************
* @brief     Return the sector size in bytes of the underlying block device for the specified file.
* @details   This is almost always 512 bytes, but may be larger for some devices.
* @param[in] sqlite3_file* File pointer
* @return    int Ssector Size
***************************************************************************************************/
int rawSectorSize(unqlite_file *id)
{
	return 4096;
}
