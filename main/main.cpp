#include "HaloDevice.h"
#include "HaloUi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "u8g2_st7305.h"

static constexpr gpio_num_t BOOT_PIN = GPIO_NUM_0, KEY_PIN = GPIO_NUM_18;
extern "C" void app_main(void) {
  gpio_config_t buttons = {};
  buttons.pin_bit_mask = (1ULL << BOOT_PIN) | (1ULL << KEY_PIN);
  buttons.mode = GPIO_MODE_INPUT;
  buttons.pull_up_en = GPIO_PULLUP_ENABLE;
  ESP_ERROR_CHECK(gpio_config(&buttons));
  halo::Model model;
  halo::Button boot, key;
  auto milliseconds = []() { return uint32_t(esp_timer_get_time() / 1000); };
  boot.begin(gpio_get_level(BOOT_PIN) == 0, milliseconds());
  key.begin(gpio_get_level(KEY_PIN) == 0, milliseconds());
  static u8g2_st7305_t lcd;
  auto config = u8g2_st7305_default_config();
  config.rotation = U8G2_R1;
  ESP_ERROR_CHECK(u8g2_st7305_init(&lcd, &config));
  u8g2_t *screen = u8g2_st7305_get_u8g2(&lcd);
  halo::render(screen, model);
  u8g2_SendBuffer(screen);
  halo::beginDevice();
  ESP_LOGI("HALO", "Home ready. BOOT: next / hold: refresh. KEY: open / hold: "
                   "back or menu. PWR: hardware power.");
  uint32_t lastFrame = milliseconds();
  while (true) {
    uint32_t now = milliseconds();
    halo::updateTools(model,now);
    halo::navigate(model, true, boot.poll(gpio_get_level(BOOT_PIN) == 0, now));
    halo::navigate(model, false, key.poll(gpio_get_level(KEY_PIN) == 0, now));
    halo::updateDevice(model, now);
    if (uint32_t(now - lastFrame) >= 167) {
      lastFrame = now;
      if (model.animated && model.page == halo::Page::Home)
        model.frame = (model.frame + 1) % halo::FRAME_COUNT;
      halo::render(screen, model);
      u8g2_SendBuffer(screen);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
