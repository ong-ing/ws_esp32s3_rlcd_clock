#include "CardStorage.h"
#include "DefaultConfig.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string>
namespace rlcd_clock {
namespace {
BootConfig active;bool fromCard=false;
const char *status="使用默认配置";

bool readFile(const char *name,std::string &out){
 FILE *f=fopen(name,"rb");if(!f)return false;char buf[1024];out.clear();size_t n;
 while((n=fread(buf,1,sizeof(buf),f))){out.append(buf,n);if(out.size()>8192)break;}
 bool ok=!ferror(f)&&out.size()<=8192&&out.find('\0')==std::string::npos;fclose(f);return ok;
}
}
void loadBootConfig(){
 const std::string filePath=std::string(cardRoot())+"/boot/setting.json";
 const std::string backupPath=filePath+".bak",tempPath=filePath+".tmp";
 const char *path=filePath.c_str(),*backup=backupPath.c_str();

 active=BootConfig{};parseBootConfig(DEFAULT_CONFIG_JSON,active);fromCard=false;
 if(!mountCard()){status="TF 未就绪，使用默认配置";return;}
 std::string json;
 if(access(path,F_OK)!=0){
  // A power cut between the two renames leaves the previous valid file here.
  if(access(backup,F_OK)==0)rename(backup,path);
 }
 if(access(path,F_OK)!=0){status="未找到 TF 配置，使用默认配置";return;}
 if(readFile(path,json)&&parseBootConfig(json.c_str(),active)){fromCard=true;status="已加载 TF 开机配置";}
 else status="TF 配置无效，使用默认配置";

}
const BootConfig &bootConfig(){return active;}
bool bootConfigFromCard(){return fromCard;}
const char *bootConfigStatus(){return status;}
bool saveCardCity(const char *name,int32_t latitude,int32_t longitude){
 const std::string filePath=std::string(cardRoot())+"/boot/setting.json";
 const std::string backupPath=filePath+".bak",tempPath=filePath+".tmp";
 const char *path=filePath.c_str(),*backup=backupPath.c_str(),*temp=tempPath.c_str();

 if(!cardReady())return false;
 std::string json; cJSON *root=nullptr;
 if(access(path,F_OK)==0){BootConfig check=active;if(!readFile(path,json)||!parseBootConfig(json.c_str(),check))return false;const char *s=json.c_str();if(strncmp(s,"\xef\xbb\xbf",3)==0)s+=3;root=cJSON_Parse(s);}
 else root=cJSON_Parse(DEFAULT_CONFIG_JSON);
 if(!root)return false;
 cJSON *weather=cJSON_GetObjectItemCaseSensitive(root,"weather");
 if(!weather){weather=cJSON_CreateObject();cJSON_AddItemToObject(root,"weather",weather);}
 cJSON_DeleteItemFromObjectCaseSensitive(weather,"city");cJSON_DeleteItemFromObjectCaseSensitive(weather,"latitude");cJSON_DeleteItemFromObjectCaseSensitive(weather,"longitude");
 cJSON_AddStringToObject(weather,"city",name);cJSON_AddNumberToObject(weather,"latitude",latitude/1000000.0);cJSON_AddNumberToObject(weather,"longitude",longitude/1000000.0);

 char *encoded=cJSON_Print(root);cJSON_Delete(root);if(!encoded)return false;
 if(strlen(encoded)>8192){cJSON_free(encoded);return false;}
 if(mkdir((std::string(cardRoot())+"/boot").c_str(),0777)!=0&&errno!=EEXIST){cJSON_free(encoded);return false;}
 FILE *f=fopen(temp,"wb");bool ok=false;
 if(f){size_t n=strlen(encoded);ok=fwrite(encoded,1,n,f)==n;if(fflush(f)!=0)ok=false;if(fsync(fileno(f))!=0)ok=false;if(fclose(f)!=0)ok=false;}
 cJSON_free(encoded);
 if(!ok){unlink(temp);return false;}
 bool had=access(path,F_OK)==0;
 if(had){unlink(backup);if(rename(path,backup)!=0){unlink(temp);return false;}}
 if(rename(temp,path)!=0){if(had)rename(backup,path);unlink(temp);return false;}
 unlink(backup);snprintf(active.city,sizeof(active.city),"%s",name);active.latitude=latitude;active.longitude=longitude;fromCard=true;return true;
}
}
