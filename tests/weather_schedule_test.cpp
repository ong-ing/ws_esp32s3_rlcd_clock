#include "HaloWeatherSchedule.h"
#include <assert.h>
#include <stdio.h>
using namespace halo;
int main(){
 const unsigned periods[]={15,30,60,90};
 for(unsigned p:periods){WeatherSchedule s;s.setMinutes(p);assert(s.ready(0));s.begin(0);s.finish(100,true,200);assert(!s.ready(100+int64_t(p)*60000000-1));assert(s.ready(100+int64_t(p)*60000000));}
 WeatherSchedule s;s.begin(0);s.finish(0,true,200);s.requestManual();assert(!s.ready(59999999)&&s.ready(60000000));
 s.requestCity();assert(s.ready(1));s.begin(1);s.finish(2,false,429);s.requestCity();s.requestManual();s.setMinutes(15);assert(!s.ready(900000001)&&s.ready(900000002));
 WeatherSchedule t;t.begin(0);t.finish(0,true,200);t.setMinutes(90);assert(!t.ready(900000000));t.setMinutes(15);assert(t.ready(900000000));
 WeatherSchedule error;error.begin(0);error.finish(1,false,500);assert(!error.ready(60000000)&&error.ready(60000001));
 error.begin(60000001);error.finish(60000002,false,500);error.setMinutes(90);assert(error.ready(180000002));
 puts("PASS four update periods, immediate city request, manual cooldown, 429 protection, interval reschedule and error retry");
}
