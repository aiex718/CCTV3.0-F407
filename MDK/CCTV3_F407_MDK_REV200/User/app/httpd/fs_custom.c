#include "lwip/apps/fs.h"
#include "lwip/mem.h"
#include "lwip/debug.h"

#include "bsp/platform/app/app_filesys.h"


//functions for lwip call
int fs_open_custom(struct fs_file *file, const char *name)
{
    FileSys_File_t *fil;

    fil = (FileSys_File_t*)mem_malloc(sizeof(FileSys_File_t));

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

    LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE,("fs_open_custom %s at %x->%x\n",name,file,fil));

    file->data=NULL;
    file->index=0;
    file->len=FileSys_GetLen(App_FileSys,fil);
    file->pextension = (void*)fil;

    return 1;
}

void fs_close_custom(struct fs_file *file)
{
    if(file->pextension != NULL)
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
