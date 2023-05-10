#ifndef DEV_CONFIG_STORAGE_H
#define DEV_CONFIG_STORAGE_H

#include "bsp/platform/platform_defs.h"
#include "device/config_storage.h"



typedef struct Example_ConfigFile_s
{
    uint32_t val_config;
    float fval_config;
    char str_config[32];
}Example_ConfigFile_t;

typedef struct Example_Object_s
{
    uint32_t val;
    float fval;
    char str[32];
}Example_Object_t;

extern Config_Storage_t Dev_ConfigStorage_Inst;
extern Config_Storage_t *Dev_ConfigStorage;

#endif
