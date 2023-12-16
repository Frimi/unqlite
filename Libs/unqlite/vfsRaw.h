/***************************************************************************************************
 * @brief     VFS for unQLite.
 * @details   Tab == 4 spaces (use Tab char instead of spaces).
 * @author    Vinicius Diori
 * @date      12/2023
 ***************************************************************************************************/
#ifndef __VFS_RAW_H__
#define __VFS_RAW_H__

/*#*************************************************************************************************
* Includes
***************************************************************************************************/
#include "fs_api.h"
#include "unqlite.h"
/*#*************************************************************************************************
* Defines
***************************************************************************************************/
#define MAXPATHNAME  ( 256 )

#ifndef MAXDWORD
#define MAXDWORD     ( 0xFFFFFFFF )   /* 4294967295 */
#endif

/*
 ** Allowed values for ctrlFlags
 */
#define RAWFILE_RDONLY          0x02   /* Connection is read only */
#define RAWFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#define RAWFILE_PSOW            0x10

#ifdef DEBUG_VFS_EN
	#define VFS_DEBUG_START() uint32_t _perfCounterTick = HAL_GetTick();
	#define VFS_DEBUG_RESTART() _perfCounterTick = HAL_GetTick();
	#define VFS_DEBUG_FINALIZE(STR, ...) if(STR) { printf(STR, ##__VA_ARGS__); } printf("%ld ms\n", HAL_GetTick()-_perfCounterTick)
    #define VFS_DEBUG_MSG(STR, ...) printf(STR, ##__VA_ARGS__)
#else
	#define VFS_DEBUG_START()
	#define VFS_DEBUG_RESTART()
	#define VFS_DEBUG_FINALIZE(STR, ...)
	#define VFS_DEBUG_MSG(STR, ...)
#endif

/*#*************************************************************************************************
* Types
***************************************************************************************************/
/*
 ** The rawFile structure is a subclass of sqlite3_file* specific to the RAW-OS
 ** portability layer.
 */
typedef struct rawFile rawFile;
struct rawFile
{
	const unqlite_io_methods *pMethod; /* Base class. Must be first */
	unqlite_vfs *pVfs; /* The VFS that created this rawFile */
	FS_FILE *h; /* Pointer to access the file */
	const char *zPath; /* Name of the file */
	int szChunk; /* Configured by FCNTL_CHUNK_SIZE */
};

/*#*************************************************************************************************
* Prototypes
***************************************************************************************************/
int rawClose(unqlite_file *id);
int rawRead(unqlite_file *id, void *pBuf, unqlite_int64 amt, unqlite_int64 offset);
int rawWrite(unqlite_file *id, const void *pBuf, unqlite_int64 amt, unqlite_int64 offset);
int rawTruncate(unqlite_file *id, unqlite_int64 nByte);
int rawSync(unqlite_file *id, int flags);
int rawFileSize(unqlite_file *id, unqlite_int64 *pSize);
int rawLock(unqlite_file *id, int eLock);
int rawUnlock(unqlite_file *id, int eLock);
int rawCheckReservedLock(unqlite_file *id, int *pResOut);
int rawSectorSize(unqlite_file *id);

/** @} */ // end of Vfs_RAW_h
#endif
