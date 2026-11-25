#include "ClockSettings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char stored[2][64]={},pending[2][64]={};
static size_t sizes[2]={},pendingSizes[2]={};
static bool fail=false;
static int slot(const char *key){return strcmp(key,"settings")==0?0:1;}
esp_err_t nvs_open(const char*,int mode,nvs_handle_t *h){*h=1;if(mode==NVS_READWRITE){memcpy(pending,stored,sizeof(stored));memcpy(pendingSizes,sizes,sizeof(sizes));}return ESP_OK;}
esp_err_t nvs_get_blob(nvs_handle_t,const char *key,void *out,size_t *size){int i=slot(key);if(!sizes[i])return -1;if(!out){*size=sizes[i];return ESP_OK;}if(*size<sizes[i])return -1;memcpy(out,stored[i],sizes[i]);*size=sizes[i];return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t,const char *key,const void *value,size_t size){int i=slot(key);if(size>64)return -1;memcpy(pending[i],value,size);pendingSizes[i]=size;return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t){if(fail)return -1;memcpy(stored,pending,sizeof(stored));memcpy(sizes,pendingSizes,sizeof(sizes));return ESP_OK;}
void nvs_close(nvs_handle_t){}
int main(){
 using namespace rlcd_clock;
 auto defaults=readPreferences();assert(!defaults.hour12&&!defaults.showSeconds&&defaults.weatherMinutes==15);
 LegacySettings legacy={1,"杭州",30293650,120161420};memcpy(stored[0],&legacy,sizeof(legacy));sizes[0]=sizeof(legacy);assert(hasSavedSettings());assert(readSettings().version==2);
 Preferences p;p.hour12=1;p.showSeconds=1;p.weatherMinutes=90;assert(savePreferences(p));
 auto loaded=readPreferences();assert(loaded.hour12&&loaded.showSeconds&&loaded.weatherMinutes==90);
 assert(strcmp(readSettings().city,"杭州")==0&&readSettings().latitude==30293650);
 fail=true;p.weatherMinutes=30;assert(!savePreferences(p));assert(readPreferences().weatherMinutes==90);fail=false;
 p.hour12=2;assert(!savePreferences(p));assert(readPreferences().hour12==1);
 stored[1][0]=9;assert(readPreferences().weatherMinutes==15);
 puts("PASS legacy city preservation, default preferences, persistence reload, failed commit and corrupt-value fallback");
}
