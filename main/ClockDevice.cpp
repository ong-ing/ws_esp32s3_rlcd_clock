#include "ClockDevice.h"
#include "CardStorage.h"
#include "ClockSettings.h"
#include "ClockCities.h"
#include "ClockBattery.h"
#include "ClockWeatherClient.h"
#include "cJSON.h"
#include "driver/i2c_master.h"
#include "driver/usb_serial_jtag.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace rlcd_clock {
namespace {
const char *TAG = "RLCD_CLOCK";
Battery battery;
i2c_master_bus_handle_t i2cBus = nullptr;
i2c_master_dev_handle_t sensor = nullptr;
EventGroupHandle_t network = nullptr;
constexpr EventBits_t CONNECTED = BIT0, TIME_READY = BIT1;
bool wifiReady = false, credentialsSet = false, ntpStarted = false,
     serialReady = false;
uint32_t lastSensor = 0, lastClock = 0, sensorAt = 0, lastRetry = 0;
uint8_t sensorStep = 0;
char command[512];
size_t commandLength = 0;
bool overflow = false;
void onNetwork(void *, esp_event_base_t base, int32_t event, void *data) {
  if (base == IP_EVENT && event == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(network, CONNECTED);
    ESP_LOGI(TAG, "WIFI_CONNECTED");
  } else if (base == WIFI_EVENT && event == WIFI_EVENT_STA_DISCONNECTED) {
    xEventGroupClearBits(network, CONNECTED);
    const auto *disconnected =
        static_cast<const wifi_event_sta_disconnected_t *>(data);
    ESP_LOGW(TAG, "WIFI_DISCONNECTED reason=%u",
             disconnected ? disconnected->reason : 0);
  }
}
uint8_t crc(const uint8_t *bytes) {
  uint8_t c = 0xff;
  for (int i = 0; i < 2; i++) {
    c ^= bytes[i];
    for (int b = 0; b < 8; b++)
      c = (c & 0x80) ? (c << 1) ^ 0x31 : c << 1;
  }
  return c;
}
bool sensorCommand(uint16_t c) {
  uint8_t bytes[] = {uint8_t(c >> 8), uint8_t(c)};
  return sensor && i2c_master_transmit(sensor, bytes, 2, 25) == ESP_OK;
}
void pollSensor(Model &m, uint32_t now) {
  if (!sensor)
    return;
  // Avoid holding the UI loop during the sensor's conversion interval.
  if (sensorStep == 0 && uint32_t(now - lastSensor) >= 5000) {
    lastSensor = now;
    if (sensorCommand(0x3517)) {
      sensorStep = 1;
      sensorAt = now;
    } else
      m.sensorValid = false;
  } else if (sensorStep == 1 && uint32_t(now - sensorAt) >= 1) {
    if (sensorCommand(0x7866)) {
      sensorStep = 2;
      sensorAt = now;
    } else {
      sensorStep = 0;
      m.sensorValid = false;
      sensorCommand(0xb098);
    }
  } else if (sensorStep == 2 && uint32_t(now - sensorAt) >= 20) {
    uint8_t d[6] = {};
    m.sensorValid = false;
    if (i2c_master_receive(sensor, d, 6, 25) == ESP_OK && crc(d) == d[2] &&
        crc(d + 3) == d[5]) {
      m.temperature = -45 + 175.0f * ((d[0] << 8) | d[1]) / 65536.0f;
      m.humidity = 100.0f * ((d[3] << 8) | d[4]) / 65536.0f;
      m.sensorValid =
          isfinite(m.temperature) && m.humidity >= 0 && m.humidity <= 100;
    }
    sensorCommand(0xb098);
    sensorStep = 0;
  }
}
bool configureWifi(const char *ssid, const char *password, const char *bssid) {
  if (!wifiReady || !ssid || !password)
    return false;
  size_t ssidLength = strlen(ssid), passwordLength = strlen(password);
  if (!ssidLength || ssidLength > 32 ||
      (passwordLength != 0 && (passwordLength < 8 || passwordLength > 63)))
    return false;
  wifi_config_t config = {};
  memcpy(config.sta.ssid, ssid, ssidLength);
  memcpy(config.sta.password, password, passwordLength);
  if (bssid && *bssid) {
    unsigned int bytes[6];
    int consumed = 0;
    if (sscanf(bssid, "%2x:%2x:%2x:%2x:%2x:%2x%n", &bytes[0], &bytes[1],
               &bytes[2], &bytes[3], &bytes[4], &bytes[5], &consumed) != 6 ||
        bssid[consumed] != 0)
      return false;
    for (int i = 0; i < 6; i++)
      config.sta.bssid[i] = bytes[i];
    config.sta.bssid_set = true;
  }
  esp_wifi_disconnect();
  xEventGroupClearBits(network, CONNECTED);
  esp_err_t error = esp_wifi_set_config(WIFI_IF_STA, &config);
  memset(&config, 0, sizeof(config));
  if (error != ESP_OK) {
    ESP_LOGW(TAG, "WIFI_SAVE_FAILED %s", esp_err_to_name(error));
    return false;
  }
  credentialsSet = true;
  ntpStarted = false;
  requestWeatherRefresh();
  ESP_LOGI(TAG, "WIFI_CONFIG_SAVED");
  error = esp_wifi_connect();
  if (error != ESP_OK)
    ESP_LOGW(TAG, "Wi-Fi connect: %s", esp_err_to_name(error));
  return true;
}
void processCommand(Model &m) {
  if (strncmp(command, "SETTINGSJSON ", 13) == 0) {
    cJSON *root=cJSON_Parse(command+13);
    const cJSON *city=cJSON_GetObjectItemCaseSensitive(root,"city"), *lat=cJSON_GetObjectItemCaseSensitive(root,"latitude"), *lon=cJSON_GetObjectItemCaseSensitive(root,"longitude");
    bool ok=cJSON_IsString(city)&&strlen(city->valuestring)>0&&strlen(city->valuestring)<49&&cJSON_IsNumber(lat)&&cJSON_IsNumber(lon)&&isfinite(lat->valuedouble)&&isfinite(lon->valuedouble)&&lat->valuedouble>=-90&&lat->valuedouble<=90&&lon->valuedouble>=-180&&lon->valuedouble<=180;
    if(ok){Settings value;snprintf(value.city,sizeof(value.city),"%s",city->valuestring);value.latitude=lround(lat->valuedouble*1000000);value.longitude=lround(lon->valuedouble*1000000);ok=saveSettings(value);if(ok){snprintf(m.city,sizeof(m.city),"%s",value.city);m.weatherValid=false;m.weatherFailed=false;requestWeatherSettingsChanged(true);}}
    ESP_LOGI(TAG,"%s",ok?"SETTINGS_SAVED":"SETTINGS_REJECTED");cJSON_Delete(root);
  } else if (strncmp(command, "WIFIJSON ", 9) == 0) {
    cJSON *root = cJSON_Parse(command + 9);
    const cJSON *ssid = cJSON_GetObjectItemCaseSensitive(root, "ssid"),
                *password = cJSON_GetObjectItemCaseSensitive(root, "password"),
                *bssid = cJSON_GetObjectItemCaseSensitive(root, "bssid");
    if (!cJSON_IsString(ssid) || !cJSON_IsString(password) ||
        !configureWifi(ssid->valuestring, password->valuestring,
                       cJSON_IsString(bssid) ? bssid->valuestring : nullptr))
      ESP_LOGW(TAG, "WIFI_CONFIG_REJECTED");
    if (cJSON_IsString(password))
      memset(password->valuestring, 0, strlen(password->valuestring));
    cJSON_Delete(root);
  } else if (strncmp(command, "WIFI ", 5) == 0) {
    char *separator = strchr(command + 5, '|');
    if (separator) {
      *separator = 0;
      if (!configureWifi(command + 5, separator + 1, nullptr))
        ESP_LOGW(TAG, "WIFI_CONFIG_REJECTED");
    } else
      ESP_LOGW(TAG, "Use WIFI ssid|password");
  } else if (strcmp(command, "REFRESH") == 0) {
    m.refreshRequested = true;
  } else if (strcmp(command, "PAUSE") == 0)
    m.animated = !m.animated;
  else if (strcmp(command, "HELP") == 0)
    printf("WIFI ssid|password ; REFRESH ; PAUSE\n");
  else
    ESP_LOGW(TAG, "Unknown command; send HELP");
}
void pollSerial(Model &m) {
  if (!serialReady)
    return;
  for (int n = 0; n < 64; n++) {
    char c;
    if (usb_serial_jtag_read_bytes(&c, 1, 0) != 1)
      break;
    if (c == '\r')
      c = '\n';
    if (c == '\n') {
      if (!overflow && commandLength) {
        command[commandLength] = 0;
        processCommand(m);
      } else if (overflow)
        ESP_LOGW(TAG, "Command too long; discarded");
      memset(command, 0, sizeof(command));
      commandLength = 0;
      overflow = false;
    } else if (c == '\b' || c == 127) {
      if (commandLength)
        command[--commandLength] = 0;
    } else if (commandLength + 1 < sizeof(command) && !overflow)
      command[commandLength++] = c;
    else
      overflow = true;
  }
}
void initSensor() {
  i2c_master_bus_config_t bus = {};
  bus.i2c_port = I2C_NUM_0;
  bus.sda_io_num = GPIO_NUM_13;
  bus.scl_io_num = GPIO_NUM_14;
  bus.clk_source = I2C_CLK_SRC_DEFAULT;
  bus.glitch_ignore_cnt = 7;
  bus.flags.enable_internal_pullup = true;
  esp_err_t err = i2c_new_master_bus(&bus, &i2cBus);
  if (err == ESP_OK) {
    i2c_device_config_t device = {};
    device.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device.device_address = 0x70;
    device.scl_speed_hz = 100000;
    err = i2c_master_bus_add_device(i2cBus, &device, &sensor);
  }
  if (err != ESP_OK)
    ESP_LOGW(TAG, "SHTC3 init: %s", esp_err_to_name(err));
}
void initNetwork() {
  // Do not erase NVS if initialization fails; home display remains usable
  // offline.
  esp_err_t err = nvs_flash_init();
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "NVS: %s; Wi-Fi disabled", esp_err_to_name(err));
    return;
  }
  const auto &bootDefaults=bootConfig();
  if(bootConfigFromCard()||!hasSavedSettings()){
    Settings initial;snprintf(initial.city,sizeof(initial.city),"%s",bootDefaults.city);initial.latitude=bootDefaults.latitude;initial.longitude=bootDefaults.longitude;
    if(!saveSettings(initial))ESP_LOGW(TAG,"Boot city persistence failed");
  }
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  network = xEventGroupCreate();
  if (!network) {
    ESP_LOGE(TAG, "No network event memory");
    return;
  }
  esp_netif_t *station=esp_netif_create_default_wifi_sta();
  if (!station) {
    ESP_LOGE(TAG, "No network interface memory");
    return;
  }
  const char *host=bootConfig().name;bool hostname=strlen(host)<=32&&host[0]!='-'&&host[strlen(host)-1]!='-';
  for(const char *c=host;*c;c++)if(!((*c>='A'&&*c<='Z')||(*c>='a'&&*c<='z')||(*c>='0'&&*c<='9')||*c=='-'))hostname=false;
  ESP_ERROR_CHECK(esp_netif_set_hostname(station,hostname?host:"rlcd-clock"));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  err = esp_wifi_init(&cfg);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Wi-Fi init: %s", esp_err_to_name(err));
    return;
  }
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, onNetwork, nullptr));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             onNetwork, nullptr));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  wifiReady = true;
  const auto &boot=bootConfig();
  if(boot.ssid[0])configureWifi(boot.ssid,boot.password,nullptr);
  wifi_config_t saved = {};
  if (!boot.ssid[0] && esp_wifi_get_config(WIFI_IF_STA, &saved) == ESP_OK && saved.sta.ssid[0]) {
    // Old v0.2 provisioning pinned a BSSID; clear this legacy restriction on
    // boot.
    if (saved.sta.bssid_set) {
      saved.sta.bssid_set = false;
      memset(saved.sta.bssid, 0, 6);
      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &saved));
    }
    credentialsSet = true;
    esp_wifi_connect();
  }
  memset(&saved, 0, sizeof(saved));
  esp_sntp_config_t ntp = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(
      3, ESP_SNTP_SERVER_LIST("ntp.aliyun.com", "ntp.tencent.com",
                              "pool.ntp.org"));
  ntp.start = false;
  ntp.sync_cb = [](struct timeval *value) {
    if (value && value->tv_sec > 1700000000) {
      xEventGroupSetBits(network, TIME_READY);
      ESP_LOGI(TAG, "SNTP_SYNC_OK");
    }
  };
  ESP_ERROR_CHECK(esp_netif_sntp_init(&ntp));
  if (!startWeatherService(network, CONNECTED | TIME_READY))
    ESP_LOGE(TAG, "Weather task allocation failed");
}
} // namespace
void beginDevice(Model &m) {
  loadBootConfig();
  ESP_LOGI(TAG,"%s",bootConfigStatus());
  snprintf(m.deviceName,sizeof(m.deviceName),"%s",bootConfig().name);
  snprintf(m.settingsMessage,sizeof(m.settingsMessage),"%s",bootConfigStatus());
  setenv("TZ", "CST-8", 1);
  tzset();
  usb_serial_jtag_driver_config_t serial =
      USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
  serial.rx_buffer_size = 256;
  serial.tx_buffer_size = 256;
  serialReady = usb_serial_jtag_driver_install(&serial) == ESP_OK;
  battery.begin();
  initSensor();
  initNetwork();
}
void updateDevice(Model &m, uint32_t now) {
  static bool loaded=false;
  if(!loaded){Settings s=readSettings();m.preferences=readPreferences();snprintf(m.city,sizeof(m.city),"%s",s.city);loaded=true;}
  pollSerial(m);
  if(m.preferencesDirty){
    m.preferencesDirty=false;
    Preferences previous=readPreferences();
    bool saved=savePreferences(m.preferences);
    if(saved&&previous.weatherMinutes!=m.preferences.weatherMinutes)requestWeatherSettingsChanged(false);
    if(!saved)m.preferences=previous;
    snprintf(m.settingsMessage,sizeof(m.settingsMessage),"%s",saved?"设置已保存":"保存失败，已恢复原设置");
  }
  if(m.cityRequest>=0){
    int index=m.cityRequest;m.cityRequest=-1;
    bool saved=false;
    if(index<CITY_COUNT){
      const auto &city=CITIES[index];Settings settings;
      snprintf(settings.city,sizeof(settings.city),"%s",city.name);
      settings.latitude=city.latitude;settings.longitude=city.longitude;
      Settings previous=readSettings();
      saved=saveSettings(settings);
      if(saved){
        bool persist=cardReady()?saveCardCity(settings.city,settings.latitude,settings.longitude):!bootConfigFromCard();
        if(!persist){saveSettings(previous);saved=false;}
      }
      if(saved){
        snprintf(m.city,sizeof(m.city),"%s",city.name);
        m.weatherValid=false;m.weatherFailed=false;m.weatherBusy=true;
        requestWeatherSettingsChanged(true);
      }
    }
    snprintf(m.settingsMessage,sizeof(m.settingsMessage),"%s",saved?"城市已保存，联网后刷新天气":"城市保存失败，请重试");
  }
  battery.poll(m,now);
  pollSensor(m, now);
  if (m.refreshRequested) {
    m.refreshRequested = false;
    requestWeatherRefresh();
  }
  WeatherResult result;
  if(takeWeatherResult(result)){
    if(result.busy)m.weatherBusy=true;
    else {
      Settings current=readSettings();
      bool matches=strcmp(result.city,m.city)==0&&result.latitude==current.latitude&&result.longitude==current.longitude;
      // A request can finish after the user changes city. Discard that result,
      // including its failure/busy status, rather than relabelling old data.
      if(matches){
        m.weatherBusy=false;m.weatherFailed=!result.success;
        if(result.success){
          m.outdoor=result.data;m.weather=result.data.scene;m.weatherValid=true;
          time_t observed=result.data.observedAt;struct tm local;localtime_r(&observed,&local);
          strftime(m.weatherUpdated,sizeof(m.weatherUpdated),"%H:%M",&local);
        }
      }
    }
  }
  if (uint32_t(now - lastClock) < 250) {
    return;
  }
  lastClock = now;
  auto &monitor=m.tools;
  monitor.heapFree=heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
  monitor.heapTotal=heap_caps_get_total_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
  monitor.psramFree=heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  monitor.psramTotal=heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
  monitor.monitorValid=true;
  snprintf(monitor.ip,sizeof(monitor.ip),"--");
  esp_netif_t *station=esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  esp_netif_ip_info_t ip={};
  if(station&&esp_netif_get_ip_info(station,&ip)==ESP_OK&&ip.ip.addr)
    snprintf(monitor.ip,sizeof(monitor.ip),IPSTR,IP2STR(&ip.ip));
  m.connected = network && (xEventGroupGetBits(network) & CONNECTED);
  if (m.connected) {
    wifi_ap_record_t ap = {};
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
      monitor.wifiRssi=ap.rssi;
      snprintf(m.ssid, sizeof(m.ssid), "%.32s",
               reinterpret_cast<const char *>(ap.ssid));
    }
    if (!ntpStarted) {
      if (esp_netif_sntp_start() == ESP_OK)
        ntpStarted = true;
    }
  } else if (wifiReady && credentialsSet &&
             uint32_t(now - lastRetry) >= 10000) {
    lastRetry = now;
    esp_wifi_connect();
  }
  snprintf(m.networkStatus, sizeof(m.networkStatus), "%s",
           m.connected      ? "Wi-Fi 已连接"
           : credentialsSet ? "Wi-Fi 连接中"
                            : "请配置 Wi-Fi");
  time_t nowEpoch = time(nullptr);
  m.weatherStale =
      m.weatherValid &&
      (!m.connected || m.weatherFailed ||
       nowEpoch - m.outdoor.observedAt > (m.preferences.weatherMinutes>45?m.preferences.weatherMinutes*60+900:3600) ||
       (nowEpoch + 28800) / 86400 != (m.outdoor.forecastDay + 28800) / 86400);
  m.timeSynced = network && (xEventGroupGetBits(network) & TIME_READY) &&
                 nowEpoch > 1700000000;
  if (m.timeSynced) {
    struct tm local;
    localtime_r(&nowEpoch, &local);
    strftime(m.time, sizeof(m.time), "%H:%M", &local);
    strftime(m.seconds,sizeof(m.seconds),"%S",&local);
    char date[16];strftime(date,sizeof(date),"%Y-%m-%d",&local);
    snprintf(m.date,sizeof(m.date),"%s  %s",date,weekdayName(local.tm_wday));
  } else {
    snprintf(m.time, sizeof(m.time), "--:--");
    snprintf(m.seconds,sizeof(m.seconds),"--");
    snprintf(m.date, sizeof(m.date), "等待网络校时");
  }
}
} // namespace rlcd_clock
