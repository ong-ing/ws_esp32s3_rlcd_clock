#include "HaloWeatherData.h"
#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
using namespace halo;
static const int64_t NOW=1795320000;
static const char *sample="{\"utc_offset_seconds\":28800,\"current\":{\"time\":1795320000,\"temperature_2m\":12.5,\"relative_humidity_2m\":66,\"wind_speed_10m\":8.2,\"weather_code\":73},\"daily\":{\"time\":[1795276800],\"temperature_2m_min\":[8.0],\"temperature_2m_max\":[15.0]}}";
static bool parseTree(cJSON *tree,WeatherData &out){char *body=cJSON_PrintUnformatted(tree);bool result=parseWeatherJson(body,strlen(body),NOW,out);cJSON_free(body);return result;}
int main(){
 WeatherData out;assert(parseWeatherJson(sample,strlen(sample),NOW,out));assert(out.scene==9&&out.code==73&&out.low==8&&out.high==15&&out.temperature==12.5);
 int codes[]={0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99};
 for(int code:codes){assert(weatherScene(code)>=0&&weatherScene(code)<20);assert(strcmp(weatherDescription(code),"天气未知")!=0);}
 assert(weatherScene(71)==8&&weatherScene(73)==9&&weatherScene(75)==10&&weatherScene(86)==10);assert(weatherScene(99)==16);assert(weatherScene(42)==-1);
 const char *invalid[]={"{}","null","[]","<html>Error</html>","{bad", "{\"current\":null}"};
 for(const char *body:invalid){WeatherData keep=out;assert(!parseWeatherJson(body,strlen(body),NOW,out));assert(out.observedAt==keep.observedAt);}
 assert(!parseWeatherJson(nullptr,0,NOW,out));assert(!parseWeatherJson(sample,strlen(sample),0,out));
 assert(!parseWeatherJson(sample,strlen(sample),NOW+14400,out));assert(!parseWeatherJson(sample,strlen(sample),NOW-3600,out));
 const char *fields[]={"temperature_2m","relative_humidity_2m","wind_speed_10m","weather_code","time"};
 for(const char *field:fields){auto *tree=cJSON_Parse(sample);auto *current=cJSON_GetObjectItemCaseSensitive(tree,"current");cJSON_ReplaceItemInObject(current,field,cJSON_CreateNull());assert(!parseTree(tree,out));cJSON_Delete(tree);}
 auto *tree=cJSON_Parse(sample);auto *daily=cJSON_GetObjectItemCaseSensitive(tree,"daily");cJSON_ReplaceItemInArray(cJSON_GetObjectItemCaseSensitive(daily,"temperature_2m_min"),0,cJSON_CreateNumber(30));assert(!parseTree(tree,out));cJSON_Delete(tree);
 tree=cJSON_Parse(sample);daily=cJSON_GetObjectItemCaseSensitive(tree,"daily");cJSON_ReplaceItemInArray(cJSON_GetObjectItemCaseSensitive(daily,"time"),0,cJSON_CreateNumber(1795190400));assert(!parseTree(tree,out));cJSON_Delete(tree);
 tree=cJSON_Parse(sample);cJSON_ReplaceItemInObject(tree,"utc_offset_seconds",cJSON_CreateNumber(0));assert(!parseTree(tree,out));cJSON_Delete(tree);
 char trailing[1024];snprintf(trailing,sizeof(trailing),"%sINVALID",sample);assert(!parseWeatherJson(trailing,strlen(trailing),NOW,out));
 puts("PASS: 28 WMO codes, snow mapping, current/daily values, malformed/null fields, stale/future timestamps, wrong forecast day, invalid min/max and UTC offset, unchanged cache on parse failure.");
}
