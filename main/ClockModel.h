#pragma once
#include "ClockWeatherData.h"
#include "FileBrowser.h"
#include "ClockTools.h"
#include "ClockPreferences.h"
#include <stddef.h>
#include <stdint.h>

namespace rlcd_clock {
constexpr uint8_t WEATHER_COUNT = 20;
constexpr uint16_t FRAME_COUNT = 3360;
enum class Page { Files, Text, Home, Menu, Info, Tools, Stopwatch, Countdown, Pomodoro, Calendar, Monitor, Settings, Provinces, Cities };
struct Model {
  char deviceName[49]="HUANGLE";
  FileState files;
  ToolState tools;
  Preferences preferences;
  bool preferencesDirty=false;
  uint8_t cityProvince=0;
  int16_t cityRequest=-1;
  char settingsMessage[64]="设置自动保存，重启后保留";
  char seconds[3]="--";
  Page page = Page::Home;
  uint8_t selected = 0;
  uint8_t weather = 0;
  uint16_t frame = 0;
  bool animated = true;
  bool batteryValid = false;
  uint8_t batteryPercent = 0;
  float batteryVoltage = 0;
  char city[49] = "上海";
  bool sensorValid = false;
  float temperature = 0;
  float humidity = 0;
  bool timeSynced = false;
  bool connected = false;
  bool refreshRequested = false;
  bool weatherValid = false;
  bool weatherBusy = false;
  bool weatherFailed = false;
  bool weatherStale = false;
  WeatherData outdoor;
  char weatherUpdated[6] = "--:--";
  char networkStatus[64] = "等待连接 Wi-Fi";
  char time[6] = "--:--";
  char date[40] = "等待网络校时";
  char ssid[33] = "OFF";
};
// Debounce and hold logic has no Arduino dependency, so bounce/rollover can be
// tested.
enum class Press { None, Short, Long };
class Button {
public:
  void begin(bool down, uint32_t now) {
    raw_ = stable_ = down;
    changed_ = pressed_ = now;
    longSent_ = down;
  }
  Press poll(bool down, uint32_t now) {
    if (down != raw_) {
      raw_ = down;
      changed_ = now;
    }
    if (raw_ != stable_ && uint32_t(now - changed_) >= 30) {
      stable_ = raw_;
      if (stable_) {
        pressed_ = now;
        longSent_ = false;
      } else if (!longSent_)
        return Press::Short;
    }
    if (stable_ && !longSent_ && uint32_t(now - pressed_) >= 700) {
      longSent_ = true;
      return Press::Long;
    }
    return Press::None;
  }

private:
  bool raw_ = false, stable_ = false, longSent_ = false;
  uint32_t changed_ = 0, pressed_ = 0;
};
void navigate(Model &model, bool boot, Press press);
void shortSsid(const char *source, char *out, size_t capacity);
} // namespace rlcd_clock
