#pragma once
#include <stddef.h>
#include <stdint.h>
namespace rlcd_clock {
struct WeatherData {
  int code = -1;
  uint8_t scene = 0;
  float temperature = 0;
  float humidity = 0;
  float windKmh = 0;
  float low = 0;
  float high = 0;
  int64_t observedAt = 0;
  int64_t forecastDay = 0;
};
int weatherScene(int code);
const char *weatherDescription(int code);
bool parseWeatherJson(const char *json, size_t length, int64_t now,
                      WeatherData &out);
} // namespace rlcd_clock
