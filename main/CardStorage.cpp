#include "CardStorage.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
namespace rlcd_clock {
namespace {sdmmc_card_t *card=nullptr;}
const char *cardRoot(){return "/sdcard";}
bool cardReady(){return card&&sdmmc_get_status(card)==ESP_OK;}
bool mountCard(){
 if(card)return cardReady();
 esp_vfs_fat_sdmmc_mount_config_t cfg={};cfg.format_if_mount_failed=false;cfg.max_files=5;cfg.allocation_unit_size=16*1024;
 sdmmc_host_t host=SDMMC_HOST_DEFAULT();host.max_freq_khz=SDMMC_FREQ_DEFAULT;
 sdmmc_slot_config_t slot=SDMMC_SLOT_CONFIG_DEFAULT();slot.width=1;slot.clk=GPIO_NUM_38;slot.cmd=GPIO_NUM_21;slot.d0=GPIO_NUM_39;
 esp_err_t err=esp_vfs_fat_sdmmc_mount(cardRoot(),&host,&slot,&cfg,&card);
 if(err!=ESP_OK){card=nullptr;ESP_LOGW("CARD","mount failed: %s (card not formatted)",esp_err_to_name(err));}
 return card!=nullptr;
}
}
