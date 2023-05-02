#include "lwip/apps/fs.h"
#include "lwip/mem.h"

#include "string.h"
#include "bsp/sys/dbg_serial.h"

//functions for lwip call
int fs_open_custom(struct fs_file *file, const char *name)
{
    //TODO: find file in FileSys(fat_fs) and attach to file->pextension

    return 0;
}

void fs_close_custom(struct fs_file *file)
{
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
