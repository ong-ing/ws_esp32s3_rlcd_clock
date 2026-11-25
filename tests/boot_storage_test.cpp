#include "CardStorage.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
namespace rlcd_clock {static std::string root;static bool ready=true;const char *cardRoot(){return root.c_str();}bool mountCard(){return ready;}bool cardReady(){return ready;}}
int main(){
 using namespace rlcd_clock;char tmp[]="/tmp/rlcd-config-XXXXXX";root=mkdtemp(tmp);assert(!root.empty());mkdir((root+"/boot").c_str(),0777);
 auto write=[](const char *s){FILE*f=fopen((root+"/boot/setting.json").c_str(),"wb");assert(f);fwrite(s,1,strlen(s),f);fclose(f);};
 loadBootConfig();assert(!bootConfigFromCard());auto defaults=bootConfig();
 write("{\"device\":{\"name\":\"旅途\"},\"custom\":{\"keep\":true},\"wifi\":{\"ssid\":\"TEST\",\"password\":\"12345678\"},\"weather\":{\"city\":\"北京\",\"latitude\":39.9,\"longitude\":116.4}}");
 loadBootConfig();assert(bootConfigFromCard()&&strcmp(bootConfig().city,"北京")==0);assert(strcmp(bootConfig().name,"旅途")==0);
 assert(saveCardCity("杭州",30293650,120161420));loadBootConfig();assert(strcmp(bootConfig().city,"杭州")==0&&strcmp(bootConfig().ssid,"TEST")==0);
 FILE*f=fopen((root+"/boot/setting.json").c_str(),"rb");char buf[4096]={};fread(buf,1,sizeof(buf)-1,f);fclose(f);assert(strstr(buf,"custom")&&strstr(buf,"keep"));
 assert(rename((root+"/boot/setting.json").c_str(),(root+"/boot/setting.json.bak").c_str())==0);loadBootConfig();assert(bootConfigFromCard()&&strcmp(bootConfig().city,"杭州")==0);
 write("{bad json");loadBootConfig();assert(!bootConfigFromCard()&&strcmp(bootConfig().name,defaults.name)==0);assert(!saveCardCity("南京",32000000,118000000));
 ready=false;loadBootConfig();assert(!bootConfigFromCard());assert(!saveCardCity("南京",32000000,118000000));
 unlink((root+"/boot/setting.json").c_str());rmdir((root+"/boot").c_str());rmdir(root.c_str());puts("Boot config: defaults, TF override, field preservation, backup recovery, malformed file and absent card passed");
}
