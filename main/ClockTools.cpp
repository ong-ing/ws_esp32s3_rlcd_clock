#include "ClockModel.h"
#include <stdio.h>
namespace rlcd_clock {
CalendarView calendarView(const Model &m){
  CalendarView v;int y,month,day;
  if(!m.timeSynced||sscanf(m.date,"%d-%d-%d",&y,&month,&day)!=3||month<1||month>12||day<1||day>31)return v;
  int index=y*12+month-1+m.tools.calendarOffset;
  v.year=index/12;v.month=index%12+1;
  if(v.year<1600||v.year>9999)return CalendarView{};
  const int lengths[]={31,28,31,30,31,30,31,31,30,31,30,31};
  bool leap=v.year%4==0&&(v.year%100!=0||v.year%400==0);
  v.days=lengths[v.month-1]+(v.month==2&&leap?1:0);
  const int shifts[]={0,3,2,5,0,3,5,1,4,6,2,4};int year=v.year-(v.month<3);
  int sunday=(year+year/4-year/100+year/400+shifts[v.month-1]+1)%7;
  v.weekday=(sunday+6)%7;v.today=m.tools.calendarOffset==0?day:0;v.valid=true;return v;
}

void updateTools(Model &m,uint32_t now){
  auto &t=m.tools;
  if(!t.clockReady){t.clockReady=true;t.lastTick=now;return;}
  uint32_t elapsed=now-t.lastTick;t.lastTick=now;
  t.uptimeMs+=elapsed;
  if(t.stopwatchRunning)t.stopwatchMs+=elapsed;
  if(t.pomoRunning){
    if(elapsed>=t.pomoRemaining){
      t.pomoRunning=false;t.pomoDone=true;
      if(t.pomoPhase==0){t.pomoRounds++;t.pomoPhase=t.pomoRounds%4==0?2:1;}
      else t.pomoPhase=0;
      t.pomoRemaining=(t.pomoPhase==0?25:t.pomoPhase==1?5:15)*60000;
    }else t.pomoRemaining-=elapsed;
  }
  if(t.countdownRunning){
    if(elapsed>=t.countdownRemaining){t.countdownRemaining=0;t.countdownRunning=false;t.countdownDone=true;}
    else t.countdownRemaining-=elapsed;
  }
}
bool handleTools(Model &m,bool boot,Press press){
  if(m.page!=Page::Tools&&m.page!=Page::Stopwatch&&m.page!=Page::Countdown&&m.page!=Page::Pomodoro&&m.page!=Page::Calendar&&m.page!=Page::Monitor)return false;
  if(press==Press::None)return true;
  if(!boot&&press==Press::Long){
    if(m.page==Page::Tools){m.page=Page::Home;m.selected=2;}
    else {m.selected=m.page==Page::Stopwatch?0:m.page==Page::Countdown?1:m.page==Page::Pomodoro?2:m.page==Page::Calendar?3:4;m.page=Page::Tools;}
    return true;
  }
  if(press!=Press::Short)return true;
  if(boot){m.selected=(m.selected+1)%(m.page==Page::Tools?TOOL_COUNT:m.page==Page::Stopwatch?3:m.page==Page::Pomodoro?2:m.page==Page::Calendar?3:m.page==Page::Monitor?1:4);return true;}
  auto &t=m.tools;
  if(m.page==Page::Tools){m.page=m.selected==0?Page::Stopwatch:m.selected==1?Page::Countdown:m.selected==2?Page::Pomodoro:m.selected==3?Page::Calendar:m.selected==4?Page::Monitor:Page::Settings;m.selected=0;}
  else if(m.page==Page::Stopwatch){
    if(m.selected==0)t.stopwatchRunning=!t.stopwatchRunning;
    else if(m.selected==1&&t.stopwatchRunning){t.laps[0]=t.laps[1];t.laps[1]=t.laps[2];t.laps[2]=t.stopwatchMs;t.lapCount++;}
    else if(m.selected==2){t.stopwatchRunning=false;t.stopwatchMs=0;t.lapCount=0;for(auto &lap:t.laps)lap=0;}
  }else if(m.page==Page::Monitor){
    m.page=Page::Tools;m.selected=4;
  }else if(m.page==Page::Calendar){
    if(m.selected==0&&t.calendarOffset>-1200)t.calendarOffset--;
    if(m.selected==1)t.calendarOffset=0;
    if(m.selected==2&&t.calendarOffset<1200)t.calendarOffset++;
  }else if(m.page==Page::Pomodoro){
    if(m.selected==0){t.pomoDone=false;t.pomoRunning=!t.pomoRunning;}
    else {t.pomoRunning=false;t.pomoDone=false;t.pomoPhase=0;t.pomoRounds=0;t.pomoRemaining=25*60000;}
  }else {
    if(m.selected==0){
      if(t.countdownDone){t.countdownDone=false;t.countdownRemaining=t.countdownDuration;}
      t.countdownRunning=!t.countdownRunning;
    }else if(m.selected==3){t.countdownRunning=false;t.countdownDone=false;t.countdownRemaining=t.countdownDuration;}
    else if(!t.countdownRunning){
      if(m.selected==1&&t.countdownDuration>60000)t.countdownDuration-=60000;
      if(m.selected==2&&t.countdownDuration<99*60000)t.countdownDuration+=60000;
      t.countdownRemaining=t.countdownDuration;t.countdownDone=false;
    }
  }
  return true;
}
}
