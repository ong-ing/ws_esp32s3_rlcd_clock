#include "DeviceConfig.h"
#include <cassert>
#include <cstring>
int main(){
 rlcd_clock::BootConfig c;assert(rlcd_clock::parseBootConfig("{}",c));
 assert(rlcd_clock::parseBootConfig("{\"device\":{\"name\":\"我的时钟\"},\"weather\":{\"city\":\"芜湖\",\"latitude\":31.3536,\"longitude\":118.4277}}",c));
 assert(c.latitude==31353600&&strcmp(c.name,"我的时钟")==0);
 auto before=c;assert(!rlcd_clock::parseBootConfig("{\"device\":{\"name\":\"new\"},\"weather\":{\"latitude\":91}}",c));assert(memcmp(&c,&before,sizeof(c))==0);
 assert(!rlcd_clock::parseBootConfig("{\"weather\":{\"city\":\"北京\"}}",c));
 assert(!rlcd_clock::parseBootConfig("{\"wifi\":{\"password\":\"123\"}}",c));
 assert(!rlcd_clock::parseBootConfig("{} junk",c));assert(!rlcd_clock::parseBootConfig("[]",c));
 assert(rlcd_clock::parseBootConfig("\xef\xbb\xbf{}",c));
}
