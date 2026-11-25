idf_component_register(
    SRCS "FileBrowser.cpp" "DeviceConfig.cpp" "CardStorage.cpp" "BootConfigStore.cpp" "ClockPreferences.cpp" "ClockTools.cpp" "main.cpp" "ClockUi.cpp" "ClockModel.cpp" "ClockDevice.cpp" "ClockWeatherData.cpp" "ClockWeatherClient.cpp"
    INCLUDE_DIRS "." "${CMAKE_CURRENT_BINARY_DIR}"
    REQUIRES fatfs sdmmc esp_driver_sdmmc esp_adc u8g2_st7305 esp_timer esp_driver_gpio esp_driver_i2c
             esp_driver_usb_serial_jtag esp_wifi esp_event esp_netif nvs_flash lwip esp_http_client mbedtls json)

set(DEFAULT_CONFIG "${PROJECT_DIR}/device.config.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${DEFAULT_CONFIG}" "${PROJECT_DIR}/tools/embed_config.py")
execute_process(COMMAND "${PYTHON}" "${PROJECT_DIR}/tools/embed_config.py" "${DEFAULT_CONFIG}" "${CMAKE_CURRENT_BINARY_DIR}/DefaultConfig.h" RESULT_VARIABLE config_result)
if(NOT config_result EQUAL 0)
  message(FATAL_ERROR "Invalid default device configuration")
endif()
