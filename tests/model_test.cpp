#include "ClockModel.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace rlcd_clock;
int main(){
 Button b;b.begin(false,0);
 assert(b.poll(true,10)==Press::None);assert(b.poll(false,20)==Press::None);assert(b.poll(true,25)==Press::None);assert(b.poll(true,54)==Press::None);assert(b.poll(true,55)==Press::None);
 assert(b.poll(false,100)==Press::None);assert(b.poll(false,130)==Press::Short);assert(b.poll(false,200)==Press::None);
 b.poll(true,300);b.poll(true,330);assert(b.poll(true,1029)==Press::None);assert(b.poll(true,1030)==Press::Long);assert(b.poll(true,1050)==Press::None);b.poll(false,1100);assert(b.poll(false,1130)==Press::None);
 b.begin(false,0xffffff00);b.poll(true,0xffffff10);b.poll(true,0xffffff30);assert(b.poll(true,0x300)==Press::Long);
 b.begin(true,0);assert(b.poll(true,1000)==Press::None);b.poll(false,1100);assert(b.poll(false,1130)==Press::None);
 Model m;for(int i=0;i<3;i++)navigate(m,true,Press::Short);assert(m.selected==0);navigate(m,false,Press::Long);assert(m.page==Page::Menu);for(int i=0;i<3;i++)navigate(m,true,Press::Short);navigate(m,false,Press::Short);assert(m.page==Page::Info);navigate(m,false,Press::Short);assert(m.page==Page::Home&&m.selected<3);
 for(int i=0;i<20;i++)navigate(m,true,Press::Long);assert(m.refreshRequested);assert(m.weather==0);navigate(m,false,Press::Short);assert(m.page==Page::Info);navigate(m,false,Press::Long);assert(m.page==Page::Home);
 char out[17];shortSsid("长三角营地网络",out,sizeof(out));assert(strcmp(out,"长三角营")==0);shortSsid("CLOCK_CAMP",out,sizeof(out));assert(strcmp(out,"CLOC")==0);shortSsid("abc",out,2);assert(strcmp(out,"a")==0);
 printf("PASS debounce, short/long presses, held-at-boot suppression, millis rollover, two-button navigation, weather refresh request, UTF-8 SSID truncation.\n");
}
