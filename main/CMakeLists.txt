idf_component_register(
    SRCS "HaloPreferences.cpp" "HaloTools.cpp" "main.cpp" "HaloUi.cpp" "HaloModel.cpp" "HaloDevice.cpp" "HaloWeatherData.cpp" "HaloWeatherClient.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_adc u8g2_st7305 esp_timer esp_driver_gpio esp_driver_i2c
             esp_driver_usb_serial_jtag esp_wifi esp_event esp_netif nvs_flash lwip esp_http_client mbedtls json)
