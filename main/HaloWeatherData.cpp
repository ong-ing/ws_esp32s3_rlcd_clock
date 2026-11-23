#include "HaloWeatherData.h"
#include "cJSON.h"
#include <math.h>
namespace halo {
int weatherScene(int code) {
  switch (code) {
  case 0:
    return 0;
  case 1:
  case 2:
    return 1;
  case 3:
    return 4;
  case 45:
  case 48:
    return 12;
  case 51:
  case 53:
  case 61:
    return 2;
  case 55:
  case 63:
    return 5;
  case 56:
  case 57:
  case 66:
  case 67:
    return 17;
  case 65:
    return 6;
  case 71:
  case 77:
  case 85:
    return 8;
  case 73:
    return 9;
  case 75:
  case 86:
    return 10;
  case 80:
  case 81:
    return 7;
  case 82:
    return 18;
  case 95:
    return 3;
  case 96:
  case 99:
    return 16;
  default:
    return -1;
  }
}
const char *weatherDescription(int code) {
  switch (code) {
  case 0:
    return "晴朗";
  case 1:
    return "晴间多云";
  case 2:
    return "多云";
  case 3:
    return "阴天";
  case 45:
    return "雾";
  case 48:
    return "雾凇";
  case 51:
  case 53:
  case 55:
    return "毛毛雨";
  case 56:
  case 57:
  case 66:
  case 67:
    return "冻雨";
  case 61:
    return "小雨";
  case 63:
    return "中雨";
  case 65:
    return "大雨";
  case 71:
    return "小雪";
  case 73:
    return "中雪";
  case 75:
    return "大雪";
  case 77:
    return "雪粒";
  case 80:
  case 81:
    return "阵雨";
  case 82:
    return "强阵雨";
  case 85:
  case 86:
    return "阵雪";
  case 95:
    return "雷阵雨";
  case 96:
  case 99:
    return "雷雨冰雹";
  default:
    return "天气未知";
  }
}
static bool number(const cJSON *o, const char *key, double low, double high,
                   double &out) {
  const cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
  if (!cJSON_IsNumber(v) || !isfinite(v->valuedouble) || v->valuedouble < low ||
      v->valuedouble > high)
    return false;
  out = v->valuedouble;
  return true;
}
static bool dailyNumber(const cJSON *daily, const char *key, double low,
                        double high, double &out) {
  const cJSON *array = cJSON_GetObjectItemCaseSensitive(daily, key);
  const cJSON *value = cJSON_GetArrayItem(array, 0);
  if (!cJSON_IsArray(array) || !cJSON_IsNumber(value) ||
      !isfinite(value->valuedouble) || value->valuedouble < low ||
      value->valuedouble > high)
    return false;
  out = value->valuedouble;
  return true;
}
bool parseWeatherJson(const char *json, size_t length, int64_t now,
                      WeatherData &out) {
  if (!json || !length || length > 8192 || now < 1700000000)
    return false;
  const char *end = nullptr;
  cJSON *root = cJSON_ParseWithLengthOpts(json, length, &end, false);
  if (!root)
    return false;
  bool trailing = false;
  for (const char *p = end; p < json + length; p++)
    if (*p != ' ' && *p != '\n' && *p != '\r' && *p != '\t' && *p != '\0') {
      trailing = true;
      break;
    }
  const cJSON *current = cJSON_GetObjectItemCaseSensitive(root, "current");
  const cJSON *daily = cJSON_GetObjectItemCaseSensitive(root, "daily");
  double low = 0, high = 0, day = 0, offset = 0;
  double temperature = 0, humidity = 0, wind = 0, code = 0, observed = 0;
  bool valid = !trailing && cJSON_IsObject(current) &&
               number(current, "temperature_2m", -90, 65, temperature) &&
               number(current, "relative_humidity_2m", 0, 100, humidity) &&
               number(current, "wind_speed_10m", 0, 500, wind) &&
               number(current, "weather_code", 0, 99, code) &&
               code == floor(code) && weatherScene((int)code) >= 0 &&
               number(current, "time", 1700000000, 4102444800, observed) &&
               observed == floor(observed) && observed <= now + 900 &&
               now - observed <= 10800 &&
               number(root, "utc_offset_seconds", 28800, 28800, offset) &&
               dailyNumber(daily, "temperature_2m_min", -90, 65, low) &&
               dailyNumber(daily, "temperature_2m_max", -90, 65, high) &&
               low <= high &&
               dailyNumber(daily, "time", 1700000000, 4102444800, day) &&
               int64_t(day + offset) / 86400 == (now + int64_t(offset)) / 86400;
  if (valid) {
    WeatherData result;
    result.code = code;
    result.scene = weatherScene(code);
    result.temperature = temperature;
    result.humidity = humidity;
    result.windKmh = wind;
    result.observedAt = observed;
    result.forecastDay = day;
    result.low = low;
    result.high = high;
    out = result;
  }
  cJSON_Delete(root);
  return valid;
}
} // namespace halo
