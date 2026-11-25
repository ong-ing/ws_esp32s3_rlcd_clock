#include "ClockWeatherClient.h"
#include "ClockSettings.h"
#include "ClockWeatherSchedule.h"
#include <stdio.h>
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <new>
#include <string.h>
#include <time.h>
namespace rlcd_clock {
namespace {
QueueHandle_t results = nullptr;
TaskHandle_t worker = nullptr;
EventGroupHandle_t network = nullptr;
EventBits_t ready = 0;
struct Response {
  char bytes[8193];
  size_t size = 0;
  bool overflow = false;
};
esp_err_t onHttp(esp_http_client_event_t *event) {
  auto *response = static_cast<Response *>(event->user_data);
  if (event->event_id == HTTP_EVENT_ON_DATA) {
    if (event->data_len < 0 ||
        response->size + static_cast<size_t>(event->data_len) >
            sizeof(response->bytes) - 1) {
      response->overflow = true;
      return ESP_FAIL;
    }
    memcpy(response->bytes + response->size, event->data, event->data_len);
    response->size += event->data_len;
    response->bytes[response->size] = 0;
  }
  return ESP_OK;
}
WeatherResult fetch() {
  WeatherResult result;
  Settings settings=readSettings();
  result.latitude=settings.latitude;result.longitude=settings.longitude;
  snprintf(result.city,sizeof(result.city),"%s",settings.city);
  char url[512];
  snprintf(url,sizeof(url),"https://api.open-meteo.com/v1/forecast?latitude=%.6f&longitude=%.6f&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m&daily=temperature_2m_min,temperature_2m_max&forecast_days=1&temperature_unit=celsius&wind_speed_unit=kmh&timezone=Asia%%2FShanghai&timeformat=unixtime",settings.latitude/1000000.0,settings.longitude/1000000.0);
  auto *response = new (std::nothrow) Response;
  if (!response)
    return result;
  esp_http_client_config_t config = {};
  config.url = url;
  config.timeout_ms = 8000;
  config.event_handler = onHttp;
  config.user_data = response;
  config.crt_bundle_attach = esp_crt_bundle_attach;
  config.disable_auto_redirect = true;
  config.buffer_size = 1024;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client) {
    esp_http_client_set_header(client, "User-Agent",
                               "RLCD_CLOCK-ESP32/0.2 (personal weather display)");
    esp_http_client_set_header(client, "Accept-Encoding", "identity");
    esp_err_t error = esp_http_client_perform(client);
    result.httpStatus = esp_http_client_get_status_code(client);
    result.success = error == ESP_OK && result.httpStatus == 200 &&
                     !response->overflow &&
                     parseWeatherJson(response->bytes, response->size,
                                      time(nullptr), result.data);
    ESP_LOGI("RLCD_CLOCK_WEATHER", "request %s, HTTP %d, parsed=%d",
             esp_err_to_name(error), result.httpStatus, result.success);
    esp_http_client_cleanup(client);
  }
  delete response;
  return result;
}
void run(void *) {
  WeatherSchedule schedule;
  schedule.setMinutes(readPreferences().weatherMinutes);
  while(true){
    uint32_t flags=0;
    xTaskNotifyWait(0,UINT32_MAX,&flags,pdMS_TO_TICKS(1000));
    if(flags&1)schedule.requestManual();
    if(flags&2)schedule.requestCity();
    if(flags&4)schedule.setMinutes(readPreferences().weatherMinutes);
    if((xEventGroupGetBits(network)&ready)!=ready)continue;
    int64_t now=esp_timer_get_time();
    if(!schedule.ready(now))continue;
    schedule.begin(now);
    WeatherResult busy;busy.busy=true;xQueueOverwrite(results,&busy);
    auto response=fetch();xQueueOverwrite(results,&response);
    schedule.finish(esp_timer_get_time(),response.success,response.httpStatus);
  }
}
} // namespace
bool startWeatherService(EventGroupHandle_t group, EventBits_t bits) {
  if (worker)
    return true;
  network = group;
  ready = bits;
  results = xQueueCreate(1, sizeof(WeatherResult));
  if (!results)
    return false;
  if (xTaskCreate(run, "rlcd_clock_weather", 8192, nullptr, 3, &worker) != pdPASS) {
    vQueueDelete(results);
    results = nullptr;
    worker = nullptr;
    return false;
  }
  return true;
}
void requestWeatherRefresh() {
  if (worker)
    xTaskNotify(worker,1,eSetBits);
}
void requestWeatherSettingsChanged(bool cityChanged){
  if(worker)xTaskNotify(worker,cityChanged?2:4,eSetBits);
}
bool takeWeatherResult(WeatherResult &result) {
  return results && xQueueReceive(results, &result, 0) == pdTRUE;
}
} // namespace rlcd_clock
