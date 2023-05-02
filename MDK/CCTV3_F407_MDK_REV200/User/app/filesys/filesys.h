#ifndef FILESYS_H
#define FILESYS_H

#include "bsp/platform/platform_defs.h"

#include "ff.h"

typedef enum {
    FILEMODE_READ			=FA_READ			,
    FILEMODE_WRITE			=FA_WRITE			,
    FILEMODE_OPEN_EXISTING	=FA_OPEN_EXISTING	,
    FILEMODE_CREATE_NEW		=FA_CREATE_NEW		,
    FILEMODE_CREATE_ALWAYS	=FA_CREATE_ALWAYS	,
    FILEMODE_OPEN_ALWAYS	=FA_OPEN_ALWAYS		,
    FILEMODE_OPEN_APPEND	=FA_OPEN_APPEND		,
}FileMode_t;

typedef struct FileSys_File_s {
    FIL fp;
}FileSys_File_t;

typedef struct FileSys_s {
    FATFS *fs;
}FileSys_t;


void FileSys_Init(FileSys_t *self);
bool FileSys_Mount(FileSys_t *self,const char* path);
bool FileSys_Unmount(FileSys_t *self,const char* path);
bool FileSys_Format(FileSys_t *self,const char* path);

bool FileSys_Open(FileSys_t *self,FileSys_File_t* file,const char* path,FileMode_t mode);
uint32_t FileSys_GetLen(FileSys_t *self,FileSys_File_t* file);
uint32_t FileSys_Read(FileSys_t *self,FileSys_File_t *file,uint8_t* buff,uint32_t len);
bool FileSys_Close(FileSys_t *self,FileSys_File_t *file);



#endif
