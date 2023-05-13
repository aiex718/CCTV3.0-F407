#include "device/config_storage.h"

#include "bsp/platform/periph/peri_flash_prog.h"
#include "bsp/sys/dbg_serial.h"

static uint32_t _calc_crc32(const uint8_t *data, uint16_t len);
static bool _get_config_addr_offset(Config_Storage_t *self,
                                    void *obj_instance, uint16_t *offset_out, uint16_t *len_out);

void Config_Storage_Init(Config_Storage_t *self)
{
    // init
    self->_config_status = CONFIG_STORAGE_STATUS_ERR_INIT;
    self->_config_size_sum = 0;
    self->_config_wbuf = NULL;

    {
        // check config size and alignment
        Config_Storage_ObjConfig_t **cfg_list = self->Config_Storage_ObjectConfig_list;
        uint8_t align_mask = self->Config_Storage_Align - 1;
        uint16_t len;

        while (*cfg_list)
        {
            len = (*cfg_list)->Obj_Config_Len;
            self->_config_size_sum += len;
            if (len & align_mask)
                DBG_WARNING("Config %s not aligned %d\n", (*cfg_list)->Obj_Name, len);

            cfg_list++;
        }

        if (self->_config_size_sum > self->Config_Storage_FlashSize)
        {
            DBG_ERROR("Config size %d exceed limit %d\n", self->_config_size_sum, self->Config_Storage_FlashSize);
            self->_config_status = CONFIG_STORAGE_STATUS_ERR_SIZE;
            return;
        }
        else if (self->_config_size_sum == 0)
        {
            DBG_WARNING("Config empty\n");
            self->_config_status = CONFIG_STORAGE_STATUS_ERR_EMPTY;
            return;
        }
        else
            DBG_INFO("Config size %d\n", self->_config_size_sum);
    }

    {
        // check if magic and crc matched
        Config_Storage_CrcFile_t *cfg = (Config_Storage_CrcFile_t *)Config_Storage_Read(self, self, NULL);
        if (cfg == NULL)
        {
            DBG_ERROR("Verify not fount, drop all config\n");
            self->_config_status = CONFIG_STORAGE_STATUS_ERR_NOTFOUND;
            return;
        }
        else if (Config_Storage_IsCrc32Valid(self, cfg) == false)
        {
            DBG_ERROR("Verify failed, drop all config\n");
            self->_config_status = CONFIG_STORAGE_STATUS_ERR_VERIFY_FAIL;
            return;
        }
    }

    self->_config_status = CONFIG_STORAGE_STATUS_OK;
}
#if CONFIG_STORAGE_DEBUG
void Config_Storage_Random(Config_Storage_t *self)
{
    uint16_t i, offset;

    if (_get_config_addr_offset(self, self, &offset, NULL) == false)
    {
        DBG_ERROR("Config not found\n");
        return;
    }

    self->_config_wbuf = (uint8_t *)BSP_MALLOC(self->_config_size_sum);

    if (self->_config_wbuf == NULL)
    {
        DBG_ERROR("Random malloc failed\n");
        return;
    }

    for (i = 0; i < self->_config_size_sum; i++)
        self->_config_wbuf[i] = (uint8_t)rand();

    Config_Storage_CalcCrc32(self,(Config_Storage_CrcFile_t *)(self->_config_wbuf + offset));

    self->Config_Storage_FlashAddr = (uint32_t)self->_config_wbuf;
    DBG_ERROR("Config random OK\n");
}
#endif

// calculate crc and write magic to storage
void Config_Storage_CalcCrc32(const Config_Storage_t *self, Config_Storage_CrcFile_t *crc)
{
    if (Config_Storage_IsChanged(self) == false)
    {
        DBG_WARNING("Config not changed, skip export\n");
        return;
    }

    crc->Config_Storage_Magic = self->Config_Storage_Magic;
    // crc calculate exclude magic and crc
    crc->Config_Storage_Crc32 = _calc_crc32(self->_config_wbuf, self->_config_size_sum - sizeof(*crc));
}

// check if crc and magic valid
bool Config_Storage_IsCrc32Valid(Config_Storage_t *self, const Config_Storage_CrcFile_t *crc)
{
    return crc != NULL && crc->Config_Storage_Magic == self->Config_Storage_Magic &&
           crc->Config_Storage_Crc32 == _calc_crc32((const uint8_t *)self->Config_Storage_FlashAddr,
                                                    self->_config_size_sum - sizeof(*crc));
}

// load all config from storage, if config is not valid, copy default config from
// object and write to storage, waiting for commit
void Config_Storage_Load(Config_Storage_t *self)
{
    Config_Storage_ObjConfig_t **list = self->Config_Storage_ObjectConfig_list;
    Config_Storage_ObjConfig_t *objcfg;
    bool skip;
    const uint8_t *config_r_ptr = (const uint8_t *)self->Config_Storage_FlashAddr;

    while (*list)
    {
        objcfg = *list++;
        skip = false;

        if (objcfg->Obj_Instance == self)
            return; // skip self, crc will calculate inside commit

        // check func exist
        if (objcfg->Obj_IsConfigValid_Func == NULL)
        {
            DBG_WARNING("%s 0x%p config has no valid check func\n", objcfg->Obj_Name, objcfg->Obj_Instance);
            skip = true;
        }
        if (objcfg->Obj_ConfigExport_Func == NULL)
        {
            DBG_WARNING("%s 0x%p config has no export func\n", objcfg->Obj_Name, objcfg->Obj_Instance);
            skip = true;
        }
        if (objcfg->Obj_ConfigSet_Func == NULL)
        {
            DBG_WARNING("%s 0x%p config has no set func\n", objcfg->Obj_Name, objcfg->Obj_Instance);
            skip = true;
        }
        if (skip)
        {
            DBG_WARNING("%s 0x%p config load skipped\n", objcfg->Obj_Name, objcfg->Obj_Instance);
            continue;
        }

        // check if config is valid
        if (self->_config_status != CONFIG_STORAGE_STATUS_OK ||
            objcfg->Obj_IsConfigValid_Func(objcfg->Obj_Instance, config_r_ptr) == false)
        {
            void *buf = BSP_MALLOC(objcfg->Obj_Config_Len);
            if (buf == false)
            {
                DBG_ERROR("malloc failed for export config %s, len %d\n", objcfg->Obj_Name, objcfg->Obj_Config_Len);
                continue;
            }

            DBG_ERROR("%s 0x%p bad config, load default\n", objcfg->Obj_Name, objcfg->Obj_Instance);

            // export to buf
            objcfg->Obj_ConfigExport_Func(objcfg->Obj_Instance, buf);
            Config_Storage_Write(self, objcfg->Obj_Instance, buf);
            BSP_FREE(buf);
        }
        else // config is valid, apply to object
            objcfg->Obj_ConfigSet_Func(objcfg->Obj_Instance, config_r_ptr);

        // move r_ptr to next config
        config_r_ptr += objcfg->Obj_Config_Len;
    }
}

bool Config_Storage_Erase(Config_Storage_t *self)
{
    bool result;

    self->_config_status = CONFIG_STORAGE_STATUS_ERR_ERASED;
    if (self->_config_wbuf != NULL)
    {
        BSP_FREE(self->_config_wbuf);
        self->_config_wbuf = NULL;
    }

    HAL_FlashProg_Unlock(Peri_FlashProg);
    result = HAL_FlashProg_Erase(Peri_FlashProg, self->Config_Storage_FlashSector);
    HAL_FlashProg_Lock(Peri_FlashProg);

    return result;
}

const void *Config_Storage_Read(Config_Storage_t *self,
                                void *obj_instance, uint16_t *len_out)
{
    uint16_t offset;
    if (_get_config_addr_offset(self, obj_instance, &offset, len_out))
        return (const void *)(self->Config_Storage_FlashAddr + offset);
    else
        return NULL;
}

bool Config_Storage_Write(Config_Storage_t *self,
                          void *obj_instance, void *obj_config)
{
    uint16_t len = 0, offset = 0;

    if (_get_config_addr_offset(self, obj_instance, &offset, &len) == false || len == 0)
    {
        DBG_ERROR("Object 0x%p config not found\n", obj_instance);
        return false;
    }

    // alloc buf for write if not allocated yet
    if (self->_config_wbuf == NULL)
    {
        self->_config_wbuf = BSP_MALLOC(self->_config_size_sum);
        if (self->_config_wbuf == NULL)
        {
            DBG_ERROR("Config write buf malloc failed\n");
            return false;
        }
        // copy original config to wbuf
        BSP_MEMCPY(self->_config_wbuf, (void *)self->Config_Storage_FlashAddr, self->_config_size_sum);
    }

    // write config to wbuf
    // must commit to flash when all config is written
    BSP_MEMCPY(self->_config_wbuf + offset, (void *)obj_config, len);
    return true;
}

bool Config_Storage_Commit(Config_Storage_t *self)
{
    bool result;
    uint16_t offset = 0;

    if (Config_Storage_IsChanged(self) == false)
        return false; // not changed, no need to save
    if (self->_config_wbuf == NULL)
        return false; // no wbuf to save

    if (_get_config_addr_offset(self, self, &offset, NULL) == false)
    {
        DBG_ERROR("Config not found\n");
        return false;
    }

    self->_config_status = CONFIG_STORAGE_STATUS_ERR_COMMIT;

    Config_Storage_CalcCrc32(self,
        (Config_Storage_CrcFile_t *)(self->_config_wbuf + offset));

    HAL_FlashProg_Unlock(Peri_FlashProg);
    do
    {
        if (HAL_FlashProg_Erase(Peri_FlashProg, self->Config_Storage_FlashSector) == false)
        {
            DBG_ERROR("FlashProg_Erase failed %d \n", self->Config_Storage_FlashSector);
            result = false;
            break;
        }

        if (HAL_FlashProg_Write(Peri_FlashProg,
                                self->Config_Storage_FlashAddr,
                                self->_config_wbuf,
                                self->_config_size_sum) == false)
        {
            DBG_ERROR("FlashProg_Write failed %d, len %d\n",
                      self->Config_Storage_FlashAddr, self->_config_size_sum);
            result = false;
            break;
        }

        // verify
        if (BSP_MEMCMP(self->_config_wbuf,
                       (void *)self->Config_Storage_FlashAddr,
                       self->_config_size_sum) != 0)
        {
            DBG_ERROR("Verify failed 0x%x\n", self->Config_Storage_FlashAddr);
            result = false;
            break;
        }

        BSP_FREE(self->_config_wbuf);
        self->_config_wbuf = NULL;

        result = true;
        self->_config_status = CONFIG_STORAGE_STATUS_OK;
    } while (0);

    HAL_FlashProg_Lock(Peri_FlashProg);

    return result;
}

static bool _get_config_addr_offset(Config_Storage_t *self,
                                    void *obj_instance, uint16_t *offset_out, uint16_t *len_out)
{
    Config_Storage_ObjConfig_t **list = self->Config_Storage_ObjectConfig_list;
    Config_Storage_ObjConfig_t *objcfg;
    uint16_t offset = 0;

    while (*list)
    {
        objcfg = *list++;

        if (objcfg->Obj_Instance == obj_instance)
        {
            if (offset_out)
                *offset_out = offset;
            if (len_out)
                *len_out = objcfg->Obj_Config_Len;
            return true;
        }
        else
            offset += objcfg->Obj_Config_Len;
    }

    return false;
}

static uint32_t _calc_crc32(const uint8_t *data, uint16_t len)
{
    uint32_t mask, crc = 0xFFFFFFFF;
    uint16_t i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}
