#include "bsp/platform/device/dev_config_storage.h"

#include "bsp/sys/dbg_serial.h"

void Example_ConfigFile_Set(Example_Object_t *obj ,const Example_ConfigFile_t *cfg)
{
    DBG_INFO("Example_ConfigFile_Set, %u, %f\n",cfg->val_config,cfg->fval_config);
    DBG_INFO("string: %s\n",cfg->str_config);

    obj->val = cfg->val_config;
    obj->fval = cfg->fval_config;
    BSP_MEMCPY(obj->str,cfg->str_config,sizeof(obj->str));
}
void Example_ConfigFile_Export(const Example_Object_t *obj , Example_ConfigFile_t *cfg)
{
    DBG_INFO("Example_ConfigFile_Export\n");
    cfg->val_config = obj->val;
    cfg->fval_config = obj->fval;
    BSP_MEMCPY(cfg->str_config,obj->str,sizeof(obj->str));
}
bool Example_ConfigFile_IsValid(Example_Object_t *obj ,const Example_ConfigFile_t *cfg)
{
    DBG_INFO("Example_ConfigFile_IsValid\n");

    return cfg!=NULL && 
        cfg->val_config !=0 && cfg->val_config != (uint32_t)-1 && 
        cfg->str_config[0]!=0xff;
}

Example_Object_t Example_Object_Inst = {
    .val = 12345678,
    .fval = 3.14159,
    .str = __DATE__" "__TIME__,
};

Config_Storage_t Dev_ConfigStorage_Inst={
    .Config_Storage_FlashAddr = 0x080E0000,
    .Config_Storage_FlashSector = FLASH_Sector_11,
    .Config_Storage_FlashSize = 128*1024,
    .Config_Storage_Align = 4,
    .Config_Storage_Magic = 0X103B5D7A,
    .Config_Storage_ObjectConfig_list = __CONST_ARRAY_CAST_VAR(Config_Storage_ObjConfig_t*)
    {
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t)
        {
            .Obj_Name = "Example_ConfigFile",
            .Obj_Instance = &Example_Object_Inst, 
            .Obj_Config_Len = sizeof(Example_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Example_ConfigFile_Set,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Example_ConfigFile_Export,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Example_ConfigFile_IsValid,
        },
        //VerifyFile MUST be the last one in the list
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t)
        {
            .Obj_Name = "Config_VerifyFile",
            .Obj_Instance = &Dev_ConfigStorage_Inst, 
            .Obj_Config_Len = sizeof(Config_Storage_CrcFile_t),
            .Obj_ConfigSet_Func = NULL,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Config_Storage_CalcCrc32,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Config_Storage_IsCrc32Valid,
        },
        NULL,//must NULL terminated
    },//Config_Storage_ObjectConfig_list
};

Config_Storage_t *Dev_ConfigStorage = &Dev_ConfigStorage_Inst;
