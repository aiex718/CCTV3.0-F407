#include "lwip/apps/fs.h"
#include "lwip/mem.h"

#include "app/httpd/webapi.h"
#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/app/app_filesys.h"

//functions for lwip call
int fs_open_custom(struct fs_file *file, const char *name)
{
    if(strcmp(name,Webapi_Enter_Point)==0)
    {
        uint8_t* buf = (uint8_t*)mem_calloc(WEBAPI_RESPONSE_BUFFER_LEN,sizeof(uint8_t));
        if(buf)
        {
            //if set data to null ,lwip will treat this file as a volatile file
            //volatile file will trigger fs_read_custom and copy file content to lwip managed mem as buffer
            file->data=(const char*)buf;
            file->index=0;
            file->len=0;//len auto increase when writing using http_builder
            file->pextension=NULL;
            file->flags=FS_FILE_FLAGS_HEADER_INCLUDED;
            DBG_INFO("fs_open_custom: %s, buf:0x%x\n",name,buf);
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
        FileSys_File_t *fil = (FileSys_File_t*)mem_malloc(sizeof(FileSys_File_t));

        if (fil==NULL)
        {
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_WARNING, ("fil mem_malloc fail\n"));
            return 0;
        }

        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("fs_open_custom search for %s\n",name));
        
        if(FileSys_Open(App_FileSys ,fil , name, FILEMODE_READ) == false)
        {
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_WARNING,("fs_open_custom fail %s\n",name));
            mem_free(fil);
            fil=NULL;
            return 0;
        }

        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_STATE,("fs_open_custom %s at %x->%x\n",name,file,fil));

        file->data=NULL;
        file->index=0;
        file->len=FileSys_GetLen(App_FileSys,fil);
        file->pextension = (void*)fil;

        return 1;
    }

    return 0;

}

void fs_close_custom(struct fs_file *file)
{
    //is file opened from FileSys?
    if(file==NULL)
    {
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            ("fs_close_custom file NULL\n"));
        return;
    }
    else if(file->pextension != NULL)
    {
        FileSys_File_t *fil = (FileSys_File_t*)file->pextension;
        if(FileSys_Close(App_FileSys,fil) == false)
        {
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_WARNING,
                ("fs_close_custom fail %x->%x\n",file,fil));
        }
        else
        {
            LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_STATE,
                ("fs_close_custom %x->%x\n",file,fil));
        }

        mem_free(file->pextension);
        file->pextension=NULL;
    }
    //is file opened from webapi request?
    else if(file->data != NULL && file->is_custom_file)
    {
        DBG_INFO("fs_close_custom,buf:0x%x\n",file->data);
        mem_free((void*)file->data);
        file->data=NULL;
    }
    else
    {
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            ("fs_close_custom file %x pex NULL\n",file));
    }
}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
    uint32_t read_len=0;
    FileSys_File_t *fil = (FileSys_File_t*)file->pextension;

    if(fil != NULL) 
    {
        uint32_t len = BSP_MIN(count,file->len - file->index);
        read_len = FileSys_Read(App_FileSys,fil,(uint8_t*)buffer,len);
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("fs_read_custom index %d += %d\n",file->index,read_len));
        file->index += read_len;
    }
    else
    {
        LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
            ("fs_close_custom file %x pex NULL\n",file));
    }

    return read_len;
}
