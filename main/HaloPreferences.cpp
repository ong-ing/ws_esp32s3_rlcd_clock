#include "HaloModel.h"
#include "HaloCities.h"
#include <stdio.h>
namespace halo {
const char *weekdayName(int day){
  static const char *names[]={"星期日","星期一","星期二","星期三","星期四","星期五","星期六"};
  return day>=0&&day<7?names[day]:"";
}
void clockText(const Model &m,char *time,size_t size,char *period,size_t periodSize){
  snprintf(period,periodSize,"%s","");
  int hour,minute;
  if(!m.timeSynced||sscanf(m.time,"%d:%d",&hour,&minute)!=2||hour<0||hour>23||minute<0||minute>59){snprintf(time,size,"--:--");return;}
  if(m.preferences.hour12){snprintf(period,periodSize,"%s",hour<12?"AM":"PM");hour=hour%12?hour%12:12;}
  snprintf(time,size,"%02d:%02d",hour,minute);
}
bool handlePreferences(Model &m,bool boot,Press press){
  if(m.page!=Page::Settings&&m.page!=Page::Provinces&&m.page!=Page::Cities)return false;
  if(press==Press::None)return true;
  if(!boot&&press==Press::Long){
    if(m.page==Page::Settings){m.page=Page::Tools;m.selected=5;}
    else if(m.page==Page::Provinces){m.page=Page::Settings;m.selected=2;}
    else {m.page=Page::Provinces;m.selected=m.cityProvince;}
    return true;
  }
  if(press!=Press::Short)return true;
  unsigned count=m.page==Page::Settings?4:m.page==Page::Provinces?4:CITY_COUNTS[m.cityProvince];
  if(boot){m.selected=(m.selected+1)%count;return true;}
  if(m.page==Page::Provinces){m.cityProvince=m.selected;m.selected=0;m.page=Page::Cities;return true;}
  if(m.page==Page::Cities){
    m.cityRequest=CITY_STARTS[m.cityProvince]+m.selected;
    m.page=Page::Settings;m.selected=2;
    snprintf(m.settingsMessage,sizeof(m.settingsMessage),"正在保存城市");return true;
  }
  if(m.selected==2){m.page=Page::Provinces;m.selected=0;return true;}
  if(m.selected==0)m.preferences.hour12=!m.preferences.hour12;
  else if(m.selected==1)m.preferences.showSeconds=!m.preferences.showSeconds;
  else {
    const uint8_t periods[]={15,30,60,90};
    for(unsigned i=0;i<4;i++)if(m.preferences.weatherMinutes==periods[i]){m.preferences.weatherMinutes=periods[(i+1)%4];break;}
  }
  m.preferencesDirty=true;
  snprintf(m.settingsMessage,sizeof(m.settingsMessage),"正在保存设置");
  return true;
}
}
