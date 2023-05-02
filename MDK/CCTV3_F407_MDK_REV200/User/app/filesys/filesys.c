#include "app/filesys/filesys.h"

#include "bsp/sys/dbg_serial.h"

void FileSys_Init(FileSys_t *self)
{
    BSP_MEMSET(self->fs , 0 , sizeof(FATFS));
}

bool FileSys_Mount(FileSys_t *self,const char* path)
{
    FRESULT fs_result;
    fs_result = f_mount(self->fs,path,1);

    if(fs_result==FR_NO_FILESYSTEM)
	{
		DBG_WARNING("No file system, formating...\n");
		if(FileSys_Format(self,path))
		{
			DBG_INFO("format successfully\n");
			//unmount and remount
			fs_result = f_mount(NULL, path, 1);
			fs_result = f_mount(self->fs, path, 1);
		}
		else
		{
			DBG_ERROR("format failed\n");
		}       
	}

    return fs_result==FR_OK;
}

bool FileSys_Unmount(FileSys_t *self,const char* path)
{
    FRESULT fs_result;
    fs_result = f_mount(NULL, path, 1);

    if(fs_result!=FR_OK)
        DBG_ERROR("Unmount failed %d\n",fs_result);

    return fs_result==FR_OK;
}

bool FileSys_Format(FileSys_t *self,const char* path)
{
    uint8_t buf[FF_MAX_SS];
    FRESULT fs_result;
    fs_result = f_mkfs(path, NULL ,(void*)buf,sizeof(buf));
    if(fs_result!=FR_OK)
        DBG_ERROR("Format failed %d\n",fs_result);
    return fs_result==FR_OK;
}

bool FileSys_Open(FileSys_t *self,FileSys_File_t* file,const char* path,FileMode_t mode)
{
    FRESULT fs_result;
    fs_result = f_open(&file->fp, path, mode);
    if(fs_result!=FR_OK)
        DBG_ERROR("Open failed %d,path:%s\n",fs_result,path);
    return fs_result==FR_OK;
}

uint32_t FileSys_Read(FileSys_t *self,FileSys_File_t *file,uint8_t* buff,uint32_t len)
{
    uint32_t read_len=0;
    FRESULT fs_result;
    fs_result = f_read(&file->fp, buff, len, &read_len);
    if(fs_result!=FR_OK)
        DBG_ERROR("Read failed %d,file:%p\n",fs_result,file);
    return read_len;
}

uint32_t FileSys_Write(FileSys_t *self,FileSys_File_t *file,uint8_t* buff,uint32_t len)
{
    uint32_t write_len=0;
    FRESULT fs_result;
    fs_result = f_write(&file->fp, buff, len, &write_len);
    if(fs_result!=FR_OK)
        DBG_ERROR("Write failed %d,file:%p\n",fs_result,file);
    return write_len;
}

bool FileSys_Close(FileSys_t *self,FileSys_File_t *file)
{
    FRESULT fs_result;
    fs_result = f_close(&file->fp);
    if(fs_result!=FR_OK)
        DBG_ERROR("Close failed %d,file:%p\n",fs_result,file);
    return fs_result==FR_OK;
}

