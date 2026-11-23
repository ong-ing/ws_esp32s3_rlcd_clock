#pragma once
#include "HaloWeatherData.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
namespace halo {
struct WeatherResult {
  bool busy = false;
  bool success = false;
  int httpStatus = 0;
  int32_t latitude=0, longitude=0;
  char city[25] = "";
  WeatherData data;
};
bool startWeatherService(EventGroupHandle_t network, EventBits_t readyBits);
void requestWeatherRefresh();
void requestWeatherSettingsChanged(bool cityChanged);
bool takeWeatherResult(WeatherResult &result);
} // namespace halo
