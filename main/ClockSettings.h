#pragma once
#include "nvs.h"
#include "ClockPreferences.h"
#include <stdint.h>
#include <string.h>
namespace rlcd_clock {
struct LegacySettings {uint32_t version;char city[25];int32_t latitude,longitude;};
struct Settings {
 uint32_t version=2;
 char city[49]="上海";
 int32_t latitude=31230400,longitude=121473700;
};
inline Settings readSettings(bool *found=nullptr){
 if(found)*found=false;
 Settings value,stored; nvs_handle_t h;
 if(nvs_open("huangle",NVS_READONLY,&h)==ESP_OK){
  size_t n=sizeof(stored);
  if(nvs_get_blob(h,"settings",&stored,&n)==ESP_OK&&n==sizeof(stored)&&stored.version==2&&memchr(stored.city,0,sizeof(stored.city))&&stored.latitude>=-90000000&&stored.latitude<=90000000&&stored.longitude>=-180000000&&stored.longitude<=180000000){value=stored;if(found)*found=true;}
  if(n==sizeof(LegacySettings)){LegacySettings old={};size_t bytes=sizeof(old);if(nvs_get_blob(h,"settings",&old,&bytes)==ESP_OK&&old.version==1&&memchr(old.city,0,sizeof(old.city))&&old.latitude>=-90000000&&old.latitude<=90000000&&old.longitude>=-180000000&&old.longitude<=180000000){if(found)*found=true;strcpy(value.city,old.city);value.latitude=old.latitude;value.longitude=old.longitude;}}
  nvs_close(h);
 }
 return value;
}
inline bool hasSavedSettings(){bool found=false;readSettings(&found);return found;}
inline bool saveSettings(const Settings &value){
 nvs_handle_t h;if(nvs_open("huangle",NVS_READWRITE,&h)!=ESP_OK)return false;
 esp_err_t e=nvs_set_blob(h,"settings",&value,sizeof(value));if(e==ESP_OK)e=nvs_commit(h);nvs_close(h);return e==ESP_OK;
}
inline Preferences readPreferences(){
 Preferences value,stored;nvs_handle_t h;
 if(nvs_open("huangle",NVS_READONLY,&h)==ESP_OK){
  size_t n=sizeof(stored);
  if(nvs_get_blob(h,"preferences",&stored,&n)==ESP_OK&&n==sizeof(stored)&&validPreferences(stored))value=stored;
  nvs_close(h);
 }
 return value;
}
inline bool savePreferences(const Preferences &value){
 if(!validPreferences(value))return false;
 nvs_handle_t h;if(nvs_open("huangle",NVS_READWRITE,&h)!=ESP_OK)return false;
 esp_err_t e=nvs_set_blob(h,"preferences",&value,sizeof(value));
 if(e==ESP_OK)e=nvs_commit(h);
 nvs_close(h);return e==ESP_OK;
}

}
