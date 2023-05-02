#include "bsp/platform/app/app_filesys.h"

#include "ff.h"

FileSys_t App_FileSys_Inst = {
    //Alloc mem for fs
    .fs = __VAR_CAST_VAR(FATFS) {
        0 
    },
};


FileSys_t *App_FileSys = &App_FileSys_Inst;
