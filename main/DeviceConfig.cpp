#include "DeviceConfig.h"
#include "cJSON.h"
#include <cmath>
#include <cstring>
namespace rlcd_clock {
namespace {
bool text(const cJSON *obj,const char *key,char *out,size_t cap,bool empty=false){
 const cJSON *v=cJSON_GetObjectItemCaseSensitive(obj,key);if(!v)return true;
 if(!cJSON_IsString(v))return false;
 size_t n=strlen(v->valuestring);if(n>=cap||(!empty&&!n))return false;
 for(size_t i=0;i<n;i++)if((unsigned char)v->valuestring[i]<32)return false;
 memcpy(out,v->valuestring,n+1);return true;
}
bool coordinate(const cJSON *obj,const char *key,int32_t &out,double limit){
 const cJSON *v=cJSON_GetObjectItemCaseSensitive(obj,key);if(!v)return true;
 if(!cJSON_IsNumber(v)||!std::isfinite(v->valuedouble)||fabs(v->valuedouble)>limit)return false;
 out=lround(v->valuedouble*1000000);return true;
}
}
bool parseBootConfig(const char *json,BootConfig &value){
 if(!json||strlen(json)>8192||strstr(json,"\\u0000"))return false;
 if(strncmp(json,"\xef\xbb\xbf",3)==0)json+=3;
 cJSON *root=cJSON_ParseWithOpts(json,nullptr,true);if(!cJSON_IsObject(root)){cJSON_Delete(root);return false;}
 BootConfig next=value;bool ok=true;
 auto device=cJSON_GetObjectItemCaseSensitive(root,"device");
 auto wifi=cJSON_GetObjectItemCaseSensitive(root,"wifi");
 auto weather=cJSON_GetObjectItemCaseSensitive(root,"weather");
 if(device)ok=cJSON_IsObject(device)&&text(device,"name",next.name,sizeof(next.name));
 if(wifi)ok=ok&&cJSON_IsObject(wifi)&&text(wifi,"ssid",next.ssid,sizeof(next.ssid),true)&&text(wifi,"password",next.password,sizeof(next.password),true);
 if(weather){
  // A different city must include its coordinates; do not silently use the old city's location.
  auto city=cJSON_GetObjectItemCaseSensitive(weather,"city");
  ok=ok&&cJSON_IsObject(weather)&&text(weather,"city",next.city,sizeof(next.city))&&coordinate(weather,"latitude",next.latitude,90)&&coordinate(weather,"longitude",next.longitude,180);
  if(city&&cJSON_IsString(city)&&strcmp(next.city,value.city)!=0)
   ok=ok&&cJSON_GetObjectItemCaseSensitive(weather,"latitude")&&cJSON_GetObjectItemCaseSensitive(weather,"longitude");
 }
 size_t pw=strlen(next.password);ok=ok&&(!pw||pw>=8);
 cJSON_Delete(root);if(ok)value=next;return ok;
}
}
