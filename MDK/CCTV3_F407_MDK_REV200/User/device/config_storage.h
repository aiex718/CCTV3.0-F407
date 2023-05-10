#ifndef CONFIG_STORAGE_H
#define CONFIG_STORAGE_H

#include "bsp/platform/platform_defs.h"
#ifndef CONFIG_STORAGE_MAX_SIZE
    #define CONFIG_STORAGE_MAX_SIZE 256
#endif

#ifndef CONFIG_STORAGE_OBJ_NAME_SIZE
    #define CONFIG_STORAGE_OBJ_NAME_SIZE 24
#endif
/* Object which want to use config storage function must implement 3 function
and it's own config data struct. Register to Config_Storage_ObjectConfig_list 
in dev_config_storage.c

Function list:
1. void (*Obj_ConfigSet_Func)(void* obj,const void* obj_config)
2. void (*Obj_ConfigExport_Func)(const void* obj,void* obj_config)
3. bool (*Obj_IsConfigValid_Func)(void* obj,const void* obj_config)

How it works:
Config_Storage_Init must be called before any other function.
Normally we'll call Config_Storage_LoadAll() to load all stored config.

If crc or magic number check failed, Obj_ConfigExport_Func will be called 
for all object in the list, exported config will overwrite all config that stored.

If crc and magic number check passed, it will load all stored config and call
Obj_IsConfigValid_Func to check if the config is valid. 
If valid, it will call Obj_ConfigSet_Func to set config to object.
If not valid, Obj_ConfigExport_Func will be called to overwrite the config.

After all config loaded or exported, config must be commit if changed.
User code can call Config_Storage_IsChanged() to check if config is changed.
Then call Config_Storage_Commit() to commit all config to storage.

There are some limitation:
1. Ths sizeof(config struct) must be multiple of 4.
2. Every field in config file must be aligned to your memory alignment.
   Normally your compiler will do this for you, but some compiler may not.

*/
typedef struct Config_Storage_ObjConfig_s
{
    void *Obj_Instance;
    void (*Obj_ConfigSet_Func)(void* obj,const void* obj_config);
    void (*Obj_ConfigExport_Func)(const void* obj,void* obj_config);
    bool (*Obj_IsConfigValid_Func)(void* obj,const void* obj_config);
    char  Obj_Name[CONFIG_STORAGE_OBJ_NAME_SIZE];
    uint16_t Obj_Config_Len;//len must be multiple of 4
}Config_Storage_ObjConfig_t;


typedef enum
{
    CONFIG_STORAGE_STATUS_ERR_NOTFOUND=0,
    CONFIG_STORAGE_STATUS_ERROR_CRC,
    CONFIG_STORAGE_STATUS_ERROR_MAGIC,
    CONFIG_STORAGE_STATUS_ERROR_SIZE,
    CONFIG_STORAGE_STATUS_OK,//Data verified and copied to ram
}Config_Storage_Status_t;

typedef struct Config_Storage_s
{
    uint8_t Config_Storage_Mem[CONFIG_STORAGE_MAX_SIZE];
    Config_Storage_ObjConfig_t **Config_Storage_ObjectConfig_list;
    const uint32_t Config_Storage_Magic;
    uint32_t Config_Storage_FlashAddr;
    uint16_t Config_Storage_FlashSector;
    uint8_t Config_Storage_Align;
    
    //private runtime variables
    bool _config_is_changed;
    uint32_t _config_storage_crc;
    Config_Storage_Status_t _config_storage_status;
}Config_Storage_t;

__STATIC_INLINE bool Config_Storage_IsChanged(Config_Storage_t *self)
{
    return self->_config_is_changed;
}

void Config_Storage_Init(Config_Storage_t *storage);
void Config_Storage_LoadAll(Config_Storage_t *storage);
bool Config_Storage_EraseAll(Config_Storage_t *storage);
const void* Config_Storage_Read(Config_Storage_t *storage, void* obj_instance, uint16_t* len_out);
bool Config_Storage_Write(Config_Storage_t *storage, void* obj_instance,void* obj_config);
bool Config_Storage_Commit(Config_Storage_t *storage);

/*Example Uasge


Config_Storage_t Dev_ConfigStorage_Inst={
    .Config_Storage_FlashAddr = 0x08004000,
    .Config_Storage_FlashSector = FLASH_Sector_1,
    .Config_Storage_Magic = 0X103B5D7F,
    .Config_Storage_ObjectConfig_list = __CONST_ARRAY_CAST_VAR(Config_Storage_ObjConfig_t*)
    {
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "Ethernetif_Default",
            .Obj_Instance = &Dev_Ethernetif_Default_Inst, 
            .Obj_Config_Len = sizeof(Ethernetif_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Ethernetif_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Ethernetif_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Ethernetif_IsConfigValid,
        },
        NULL,
    },//Config_Storage_ObjectConfig_list
};

*/


#endif
