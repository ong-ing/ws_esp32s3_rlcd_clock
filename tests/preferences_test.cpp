#include "HaloModel.h"
#include "HaloCities.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace halo;
int main(){
 Model m;char time[6],period[3];
 m.preferences.hour12=1;clockText(m,time,6,period,3);assert(strcmp(time,"--:--")==0&&!period[0]);
 m.timeSynced=true;
 const char *inputs[]={"00:00","11:59","12:00","23:59"};
 const char *outputs[]={"12:00","11:59","12:00","11:59"};
 for(int i=0;i<4;i++){strcpy(m.time,inputs[i]);clockText(m,time,6,period,3);assert(strcmp(time,outputs[i])==0);assert(strcmp(period,i<2?"AM":"PM")==0);}
 m.preferences.hour12=0;clockText(m,time,6,period,3);assert(strcmp(time,"23:59")==0&&!period[0]);
 assert(strcmp(weekdayName(0),"星期日")==0&&strcmp(weekdayName(1),"星期一")==0);
 m.page=Page::Tools;m.selected=5;navigate(m,false,Press::Short);assert(m.page==Page::Settings);
 navigate(m,false,Press::Short);assert(m.preferences.hour12&&m.preferencesDirty);
 navigate(m,true,Press::Short);navigate(m,false,Press::Short);assert(m.preferences.showSeconds);
 navigate(m,false,Press::Long);assert(m.page==Page::Tools&&m.selected==5);
 Preferences p;assert(validPreferences(p));p.weatherMinutes=20;assert(!validPreferences(p));
 m.page=Page::Settings;m.selected=2;navigate(m,false,Press::Short);assert(m.page==Page::Provinces);
 m.selected=3;navigate(m,false,Press::Short);assert(m.page==Page::Cities&&m.cityProvince==3);
 for(int i=0;i<15;i++)navigate(m,true,Press::Short);
 navigate(m,false,Press::Short);assert(m.cityRequest==40&&m.page==Page::Settings&&strcmp(CITIES[m.cityRequest].name,"宣城")==0);
 m.cityRequest=-1;m.selected=2;navigate(m,false,Press::Short);navigate(m,false,Press::Long);assert(m.cityRequest==-1&&m.selected==2);
 assert(CITY_COUNT==41&&CITY_COUNTS[0]+CITY_COUNTS[1]+CITY_COUNTS[2]+CITY_COUNTS[3]==41);
 assert(strcmp(CITIES[8].name,"泰州")==0&&strcmp(CITIES[23].name,"台州")==0&&CITIES[8].latitude>CITIES[23].latitude);
 puts("PASS all-city paging, confirmation, cancellation and province separation");
 m.page=Page::Settings;m.selected=3;
 const int sequence[]={30,60,90,15};
 for(int expected:sequence){navigate(m,false,Press::Short);assert(m.preferences.weatherMinutes==expected&&m.preferencesDirty);}
 puts("PASS four weather intervals and wraparound");
 puts("PASS clock midnight/noon, no-time state, weekdays, settings navigation and validation");
}
