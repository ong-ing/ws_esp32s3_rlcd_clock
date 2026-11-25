#pragma once
#include <stdint.h>
#include <stddef.h>
namespace rlcd_clock {
struct Model;
enum class Press;
struct Preferences {
  uint8_t version=1;
  uint8_t hour12=0;
  uint8_t showSeconds=0;
  uint8_t weatherMinutes=15;
};
inline bool validPreferences(const Preferences &p){
  return p.version==1&&p.hour12<=1&&p.showSeconds<=1&&
      (p.weatherMinutes==15||p.weatherMinutes==30||p.weatherMinutes==60||p.weatherMinutes==90);
}
bool handlePreferences(Model &m,bool boot,Press press);
void clockText(const Model &m,char *time,size_t capacity,char *period,size_t periodCapacity);
const char *weekdayName(int sundayBased);
}
