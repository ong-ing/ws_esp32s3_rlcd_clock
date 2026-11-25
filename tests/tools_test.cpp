#include "ClockModel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
using namespace rlcd_clock;
int main(){
 Model m; m.selected=2;navigate(m,false,Press::Short);assert(m.page==Page::Tools);
 navigate(m,false,Press::Short);assert(m.page==Page::Stopwatch);
 updateTools(m,0xfffffff0);navigate(m,false,Press::Short);updateTools(m,20);assert(m.tools.stopwatchMs==36);
 m.selected=1;navigate(m,false,Press::Short);assert(m.tools.lapCount==1&&m.tools.laps[2]==36);
 navigate(m,false,Press::Long);assert(m.page==Page::Tools);updateTools(m,120);assert(m.tools.stopwatchMs==136);
 m.selected=1;navigate(m,false,Press::Short);assert(m.page==Page::Countdown);
 m.tools.countdownRemaining=100;navigate(m,false,Press::Short);updateTools(m,220);assert(m.tools.countdownDone&&!m.tools.countdownRunning);
 navigate(m,false,Press::Short);assert(m.tools.countdownRunning&&m.tools.countdownRemaining==300000);
 m.selected=2;navigate(m,false,Press::Short);assert(m.tools.countdownDuration==300000);
 navigate(m,false,Press::Long);navigate(m,false,Press::Long);assert(m.page==Page::Home&&m.selected==2);
 m.page=Page::Tools;m.selected=2;navigate(m,false,Press::Short);assert(m.page==Page::Pomodoro);
 uint32_t now=220;
 for(int round=1;round<=4;round++){
  navigate(m,false,Press::Short);assert(m.tools.pomoRunning);
  now+=25*60000;updateTools(m,now);assert(m.tools.pomoDone&&!m.tools.pomoRunning&&m.tools.pomoRounds==unsigned(round));
  assert(m.tools.pomoRemaining==unsigned((round==4?15:5)*60000));
  navigate(m,false,Press::Short);now+=(round==4?15:5)*60000;updateTools(m,now);assert(m.tools.pomoPhase==0);
 }
 m.selected=1;navigate(m,false,Press::Short);assert(!m.tools.pomoDone&&m.tools.pomoRounds==0);
 puts("PASS four pomodoro cycles, short/long rest, acknowledgement and reset");
 m.page=Page::Calendar;m.timeSynced=false;assert(!calendarView(m).valid);
 m.timeSynced=true;strcpy(m.date,"2024-02-29");auto cal=calendarView(m);assert(cal.days==29&&cal.weekday==3&&cal.today==29);
 strcpy(m.date,"2100-02-01");assert(calendarView(m).days==28);
 strcpy(m.date,"2026-12-31");m.selected=2;navigate(m,false,Press::Short);cal=calendarView(m);assert(cal.year==2027&&cal.month==1&&cal.today==0);
 m.selected=1;navigate(m,false,Press::Short);assert(calendarView(m).today==31);
 puts("PASS calendar leap years, century boundary, month rollover and today");
 m.page=Page::Tools;m.selected=4;navigate(m,false,Press::Short);assert(m.page==Page::Monitor);
 navigate(m,true,Press::Short);assert(m.selected==0);navigate(m,false,Press::Long);assert(m.page==Page::Tools&&m.selected==4);
 navigate(m,false,Press::Short);navigate(m,false,Press::Short);assert(m.page==Page::Tools&&m.selected==4);
 puts("PASS monitor short/long return and single-action selection");
 puts("PASS toolbox navigation, monotonic rollover, lap, background timers, expiry, restart, locked duration while running");
}
