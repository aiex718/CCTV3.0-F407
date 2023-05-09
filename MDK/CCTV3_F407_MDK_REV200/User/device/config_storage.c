#include "device/config_storage.h"

#include "bsp/platform/periph/peri_flash_prog.h"
#include "bsp/sys/dbg_serial.h"

static const void* Config_Storage_GetConfig(Config_Storage_t *storage, void* obj_instance,uint16_t *len_out);
static uint32_t _calc_crc32(const uint8_t* data,uint16_t len);

void Config_Storage_Init(Config_Storage_t *storage)
{
    Config_Storage_t *config_in_flash = (Config_Storage_t *)storage->Config_Storage_FlashAddr;

    {
        //check size and alignment
        Config_Storage_ObjConfig_t **cfg_list = storage->Config_Storage_ObjectConfig_list;
        uint16_t config_len_total = 0;
        uint8_t align_mask = storage->Config_Storage_Align-1;

        while(*cfg_list)
        {
            uint16_t len = (*cfg_list)->Obj_Config_Len;
            config_len_total+= len;
            if(len&align_mask)
                DBG_WARNING("Config %s not aligned %d\n",(*cfg_list)->Obj_Name,len);
            
            cfg_list++;
        }
        
        //check if config size is valid
        if(config_len_total>sizeof(storage->Config_Storage_Mem))
        {
            DBG_ERROR("Config too big %d > %d\n",config_len_total,sizeof(storage->Config_Storage_Mem));
            storage->_config_storage_status = CONFIG_STORAGE_STATUS_ERROR_SIZE;
            return;
        }
        else
            DBG_INFO("Config size %d\n",config_len_total);
    }

    {
        //check magic
        if(config_in_flash->Config_Storage_Magic != storage->Config_Storage_Magic )
        {
            DBG_ERROR("Magic number not match, drop all config\n");
            storage->_config_storage_status = CONFIG_STORAGE_STATUS_ERROR_MAGIC;
            return;
        }     
    }
       
    {
        //check crc
        uint32_t crc;
        crc = _calc_crc32((uint8_t*)config_in_flash->Config_Storage_Mem, sizeof(config_in_flash->Config_Storage_Mem));
        if(config_in_flash->_config_storage_crc != crc)
        {
            DBG_ERROR("CRC not match %x!=%x, drop all config\n,",config_in_flash->_config_storage_crc,crc);
            storage->_config_storage_status = CONFIG_STORAGE_STATUS_ERROR_CRC;
            return;
        }
    }

    //copy config from flash
    BSP_MEMCPY(storage->Config_Storage_Mem, config_in_flash->Config_Storage_Mem, sizeof(storage->Config_Storage_Mem));
    storage->_config_storage_status = CONFIG_STORAGE_STATUS_OK;
}


//load all config from storage->mem, if config is not valid, copy default config from 
//object and write to storage->mem, waiting for commit
void Config_Storage_LoadAll(Config_Storage_t *storage)
{
    uint8_t *config_addr = storage->Config_Storage_Mem;
    void *config;
    Config_Storage_ObjConfig_t **list = storage->Config_Storage_ObjectConfig_list;
    Config_Storage_ObjConfig_t *objcfg;

    while(*list)
    {
        objcfg = *list++;
        config = (void *)config_addr;

        if( storage->_config_storage_status != CONFIG_STORAGE_STATUS_OK ||
            objcfg->Obj_IsConfigValid_Func(objcfg->Obj_Instance,config) == false)
        {
            DBG_ERROR("%s 0x%p bad config, load default\n",objcfg->Obj_Name,objcfg->Obj_Instance);
            //export write to storage->payload directly, no need to copy
            objcfg->Obj_ConfigExport_Func(objcfg->Obj_Instance,config_addr);
            storage->_config_is_changed = true;
        }

        objcfg->Obj_ConfigSet_Func(objcfg->Obj_Instance,config);
        config_addr+=objcfg->Obj_Config_Len;

        if(config_addr > storage->Config_Storage_Mem+sizeof(storage->Config_Storage_Mem))
        {
            DBG_ERROR("Config addr overflow\n");
            return;
        }
    }
}

bool Config_Storage_EraseAll(Config_Storage_t *storage)
{
    bool result;

    storage->_config_storage_status = CONFIG_STORAGE_STATUS_ERR_NOTFOUND;
    storage->_config_is_changed = false;
    BSP_MEMSET(storage->Config_Storage_Mem,0xFF,sizeof(storage->Config_Storage_Mem));
    
    HAL_FlashProg_Unlock(Peri_FlashProg);
    result = HAL_FlashProg_Erase(Peri_FlashProg,storage->Config_Storage_FlashSector);
    HAL_FlashProg_Lock(Peri_FlashProg);

    return result;
}

const void* Config_Storage_Read(Config_Storage_t *storage, void* obj_instance, uint16_t* len_out)
{
    if(storage->_config_storage_status == CONFIG_STORAGE_STATUS_OK)
    {
        Config_Storage_t *config_in_flash = (Config_Storage_t *)storage->Config_Storage_FlashAddr;
        return Config_Storage_GetConfig(config_in_flash,obj_instance,len_out);
    }

    return NULL;
}

bool Config_Storage_Write(Config_Storage_t *storage, void* obj_instance,void* obj_config)
{
    if(storage->_config_storage_status != CONFIG_STORAGE_STATUS_OK)
        return false;
    else
    {
        void* addr;
        uint16_t len = 0;
        addr = (void*)Config_Storage_GetConfig(storage,obj_instance,&len);

        if(addr == NULL || len == 0)
            return false;

        //write config in ram , and mark it as changed
        //must commit to flash when all config is written
        BSP_MEMMOVE(addr,(void*)obj_config,len);
        storage->_config_is_changed = true;

        return true;
    }
}

bool Config_Storage_Commit(Config_Storage_t *storage)
{
    bool result;

    if(storage->_config_is_changed == false)
        return false; //not changed, no need to save

    storage->_config_storage_crc = _calc_crc32((uint8_t*)storage->Config_Storage_Mem, sizeof(storage->Config_Storage_Mem));
    
    HAL_FlashProg_Unlock(Peri_FlashProg);
    do
    {
        if(HAL_FlashProg_Erase(Peri_FlashProg,storage->Config_Storage_FlashSector)==false)
        {
            DBG_ERROR("FlashProg_Erase failed %d \n",storage->Config_Storage_FlashSector);
            result=false;
            break;
        }

        if(HAL_FlashProg_Write(Peri_FlashProg,storage->Config_Storage_FlashAddr,storage,sizeof(*storage))==false)
        {
            DBG_ERROR("FlashProg_Write failed %d, len %d\n",storage->Config_Storage_FlashAddr,sizeof(storage));
            result=false;
            break;
        }

        //verify
        if(BSP_MEMCMP(storage,(void*)storage->Config_Storage_FlashAddr,sizeof(*storage))!=0)
        {
            DBG_ERROR("Verify failed 0x%x\n",storage->Config_Storage_FlashAddr);
            result=false;
            break;
        }

        result=true;
        storage->_config_storage_status = CONFIG_STORAGE_STATUS_OK;
    }while(0);

    HAL_FlashProg_Lock(Peri_FlashProg);

    return result;
}

static const void* Config_Storage_GetConfig(Config_Storage_t *storage, void* obj_instance,uint16_t *len_out)
{
    Config_Storage_ObjConfig_t **list = storage->Config_Storage_ObjectConfig_list;
    Config_Storage_ObjConfig_t *objcfg;
    uint32_t addr = (uint32_t)storage->Config_Storage_Mem;

    while(*list)
    {
        objcfg = *list++;

        if(objcfg->Obj_Instance == obj_instance)
        {
            if(len_out)
                *len_out = objcfg->Obj_Config_Len;
            return (const void*) addr;
        }
        else
            addr+=objcfg->Obj_Config_Len;
    }
    
    return NULL;
}

static uint32_t _calc_crc32(const uint8_t* data,uint16_t len)
{
    uint32_t mask,crc = 0xFFFFFFFF;
    uint16_t i,j;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}


