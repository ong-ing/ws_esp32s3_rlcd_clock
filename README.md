# RLCD Clock · ESP-IDF 5.5.5

微雪 ESP32-S3-RLCD-4.2，400×300 单色横屏，U8g2 / ST7305。

**开发和编译环境：Apple M1 MacOS_27.0.1** 

**Windows环境未提供**

- 本地开发和编译环境准备：[本次交付记录](docs/macos_esp32开发环境初始化操作手册.md)

设备名称可通过 `/boot/setting.json` 的 `device.name` 自定义。

- 更新入口：`Device_Update.command` MacOS直接双击执行该文件即可
- 编译默认配置：`device.config.json` 包含默认设备名，天气城市，wifi配置等(port改为实际电脑能读到的串口号)
```
{
  "port": "/dev/cu.usbmodem1101",
  "wifi": {
    "ssid": "ZTE_LE",
    "password": "huangle@1994"
  },
  "weather": {
    "city": "芜湖",
    "latitude": 31.3536,
    "longitude": 118.4277
  },
  "device": {
    "name": "HUANGLE"
  }
}
```
- TF 配置模板：`examples/boot/setting.json` 配置优先级高于device.config.json，每次开机从TF卡中读取。
```
{
  "device": { "name": "HUANGLE" },
  "wifi": { "ssid": "YOUR_WIFI", "password": "YOUR_PASSWORD" },
  "weather": { "city": "芜湖", "latitude": 31.3536, "longitude": 118.4277 }
}

```
- 操作说明、配置优先级、限制与实机验收：[使用说明](docs/使用说明.md)

加载 ESP-IDF 5.5.5 环境后，在项目目录执行 `idf.py build`。更改默认配置后应重新编译，双击更新入口只烧录 `firmware/` 中已经打包的固件。

运行全部测试：`sh tests/run_all_tests.sh`，需要已设置 `IDF_PATH` 和可用的本机 C++ 编译器。

主页营地、天气动画、状态栏尺寸与按键约定沿用已确认版本。旧内部城市和时钟设置可继续读取；读取 TF 卡失败不会格式化卡。

## 数据与许可

天气：Open-Meteo。城市数据及出处：`design/cities-national.json`。硬件接线参考微雪官方 [SD 卡示例](https://github.com/waveshareteam/ESP32-S3-RLCD-4.2/tree/main/examples/esp-idf/06_SD_Card)。

Waveshare Apache-2.0 许可位于 `licenses/`，U8g2 BSD 许可保留在组件目录与源文件。营地图像和用户确认的资源保留在 `design/`。早期版本文档保留用于追溯，配置与更新操作以本页和《使用说明》为准。
