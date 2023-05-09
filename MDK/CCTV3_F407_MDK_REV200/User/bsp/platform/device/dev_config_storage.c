#include "bsp/platform/device/dev_config_storage.h"

#include "bsp/platform/platform_inst.h"

Config_Storage_t Dev_ConfigStorage_Inst={
    .Config_Storage_FlashAddr = 0x08004000,
    .Config_Storage_FlashSector = FLASH_Sector_1,
    .Config_Storage_Magic = 0X103B5D7F,
    .Config_Storage_Align = 4,
    .Config_Storage_ObjectConfig_list = __CONST_ARRAY_CAST_VAR(Config_Storage_ObjConfig_t*)
    {
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "NetTime",
            .Obj_Instance = &App_NetTime_Inst, 
            .Obj_Config_Len = sizeof(NetTime_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))NetTime_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))NetTime_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))NetTime_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "Mjpegd",
            .Obj_Instance = &App_Mjpegd_Inst, 
            .Obj_Config_Len = sizeof(Mjpegd_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Mjpegd_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Mjpegd_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Mjpegd_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "Cam_OV2640",
            .Obj_Instance = &Dev_Cam_OV2640_Inst, 
            .Obj_Config_Len = sizeof(Device_CamOV2640_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Device_CamOV2640_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Device_CamOV2640_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Device_CamOV2640_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "FlashLight_Top",
            .Obj_Instance = &Dev_FlashLight_Top_Inst, 
            .Obj_Config_Len = sizeof(Device_FlashLight_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Device_FlashLight_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Device_FlashLight_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Device_FlashLight_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "FlashLight_Bot",
            .Obj_Instance = &Dev_FlashLight_Bottom_Inst, 
            .Obj_Config_Len = sizeof(Device_FlashLight_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Device_FlashLight_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Device_FlashLight_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Device_FlashLight_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "Ethernetif_Default",
            .Obj_Instance = &Dev_Ethernetif_Default_Inst, 
            .Obj_Config_Len = sizeof(Ethernetif_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Ethernetif_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Ethernetif_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Ethernetif_IsConfigValid,
        },
        __CONST_CAST_VAR(Config_Storage_ObjConfig_t){
            .Obj_Name = "Current_Trig",
            .Obj_Instance = &Dev_CurrentTrig_Inst, 
            .Obj_Config_Len = sizeof(Device_CurrentTrig_ConfigFile_t),
            .Obj_ConfigSet_Func =(void(*)(void*,const void*))Device_CurrentTrig_ConfigSet,
            .Obj_ConfigExport_Func =  (void(*)(const void*,void*))Device_CurrentTrig_ConfigExport,
            .Obj_IsConfigValid_Func = (bool(*)(void*,const void*))Device_CurrentTrig_IsConfigValid,
        },
        NULL,
    },//Config_Storage_ObjectConfig_list
};

Config_Storage_t *Dev_ConfigStorage = &Dev_ConfigStorage_Inst;
