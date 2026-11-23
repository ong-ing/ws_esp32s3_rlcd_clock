#pragma once
#include <stdint.h>
namespace halo {
struct Model;
enum class Press;
struct ToolState {
  uint64_t uptimeMs=0;
  uint32_t heapFree=0, heapTotal=0, psramFree=0, psramTotal=0;
  int wifiRssi=0;
  bool monitorValid=false;
  char ip[16]="--";
  int16_t calendarOffset=0;
  bool clockReady=false;
  uint32_t lastTick=0;
  bool stopwatchRunning=false;
  uint64_t stopwatchMs=0;
  uint64_t laps[3]={};
  uint32_t lapCount=0;
  bool pomoRunning=false, pomoDone=false;
  uint8_t pomoPhase=0;
  uint32_t pomoRounds=0, pomoRemaining=25*60000;
  bool countdownRunning=false;
  bool countdownDone=false;
  uint32_t countdownDuration=5*60*1000;
  uint32_t countdownRemaining=5*60*1000;
};
constexpr int TOOL_COUNT=6;
struct CalendarView { bool valid=false; int year=0,month=0,today=0,days=0,weekday=0; };
CalendarView calendarView(const Model &m);
void updateTools(Model &m,uint32_t now);
bool handleTools(Model &m,bool boot,Press press);
}
