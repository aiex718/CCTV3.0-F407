#include "lwip/apps/fs.h"
#include "eth/apps/webapi/webapi.h"
#include "lwip/mem.h"

#include "string.h"


//functions for lwip call
int fs_open_custom(struct fs_file *file, const char *name)
{
    if(strcmp(name,WEBAPI_ENTRY_POINT)==0)
    {
        char* buf = (char*)mem_calloc(WEBAPI_RESPONSE_BUFFER_LEN,sizeof(char));
        if(buf)
        {
            //if set data to null ,lwip will treat this file as a volatile file
            //volatile file will trigger fs_read_custom and copy file content to lwip managed mem as buffer
            file->data=(const char*)buf;
            file->index=0;
            file->len=0;//len auto increase when writing using http_builder
            file->pextension=NULL;
            file->flags=FS_FILE_FLAGS_HEADER_INCLUDED;
            printf("fs_open_custom: %s, buf:0x%x\n",name,buf);
            return 1;
        }
        else
        {
            //fs_open(file,"/503.html");
            //TODO:fs_open 503 message 
        }
    }
    else
    {
        //TODO: find file in FileSys(fat_fs) and attach to file->pextension
    }

    return 0;
}

void fs_close_custom(struct fs_file *file)
{
    if(file!=NULL && file->data != NULL && file->is_custom_file)
    {
        printf("fs_close_custom,buf:0x%x\n",file->data);
        mem_free((void*)file->data);
        file->data=NULL;
    }
    //TODO: release file in FileSys(fat_fs) if file->pextension is not null
}

// int fs_read_custom(struct fs_file *file, char *buffer, int count)
// {
//     int read;
//     if (file->index == file->len) 
//         return FS_READ_EOF;

//     read = file->len - file->index;
//     if (read > count) 
//         read = count;

//     MEMCPY(buffer, (char*)(file->pextension) + file->index, read);
//     file->index += read;

//     return read;
// }
