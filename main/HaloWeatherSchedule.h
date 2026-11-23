#pragma once
#include <stdint.h>
namespace halo {
// Monotonic microseconds. City changes bypass the ordinary manual cooldown,
// but never bypass an upstream 429 backoff.
class WeatherSchedule {
  int64_t due_=0,lastAttempt_=-60000000,lastSuccess_=-1,blockedUntil_=0;
  int64_t period_=900000000;
  int failures_=0;
  bool manual_=false,city_=false,failed_=false;
public:
  void requestManual(){manual_=true;}
  void requestCity(){city_=true;}
  void setMinutes(unsigned minutes){
    if(minutes!=15&&minutes!=30&&minutes!=60&&minutes!=90)return;
    period_=int64_t(minutes)*60000000;
    if(lastSuccess_>=0&&!failed_)due_=lastSuccess_+period_;
  }
  bool ready(int64_t now)const{
    return now>=blockedUntil_&&(city_||now>=due_||(manual_&&now-lastAttempt_>=60000000));
  }
  void begin(int64_t now){lastAttempt_=now;manual_=city_=false;}
  void finish(int64_t now,bool success,int status){
    failed_=!success;
    if(success){failures_=0;lastSuccess_=now;due_=now+period_;blockedUntil_=0;}
    else {
      failures_++;
      int seconds=failures_==1?60:failures_==2?120:300;
      if(status==429){seconds=900;blockedUntil_=now+900000000;}
      due_=now+int64_t(seconds)*1000000;
    }
  }
};
}
