> 当前版本 HUANGLE V0.44：采用确认后的篝火轮廓，并统一营地右边界。详见 V044_营地修正说明.txt。

> 当前版本 HUANGLE V0.43：系统设置操作与提交记录见《系统设置交付记录.txt》。

> 工具箱现已实装，操作和模块提交见同目录《工具箱交付记录.txt》。

> 当前版本 HUANGLE V0.39：请先阅读同目录 HUANGLE_使用说明.txt。以下为早期版本记录；当前已支持本地配置和 ADC 电量，更新脚本不再查询电脑 Wi-Fi 或钥匙串。

# HALO Home v0.3 · ESP-IDF 5.5.5

微雪 ESP32-S3-RLCD-4.2，400×300 横屏。纯 ESP-IDF，不使用 Arduino IDE。

## v0.3 连接与闪烁修复

- 配网工具不再预设旧 SSID/BSSID，默认读取电脑当前 Wi-Fi。只读取该网络的钥匙串密码；OS 拒绝时才提示手动输入。不打印或写入密码。
- 新固件开机清除旧版本保存的固定 BSSID 限制，用已有 SSID/密码尝试自动连接。默认让 ESP32-S3 自行选择同名网络的 2.4 GHz 接入点。
- 输出 WIFI_DISCONNECTED 原因码，配网脚本解释未找到网络、认证/握手失败等，不再只给通用超时。
- 检查过晴天八帧：静态山体、树林、地面完全不变，只有篝火/火星变化。驱动新增上一帧缓存，只发送变化区域；传输失败不提交缓存，下一次重试。暂停动画后，内容不变时不再重复写屏。
- 通过 ESP-IDF 编译和全部 300 列脏区范围/12列对齐测试。无法访问实机，不能保证已经消除用户看到的闪烁；若暂停动画后仍闪，需继续检查面板时序、电源或点阵光学干涉，不能当作设计效果。

## 首页能力

- ST7305 初始化从 `0x21` 反色改为 `0x20` 正常模式，修复用户报告的实机黑白颠倒。
- 营地图由 `(196,34)` 下移到 `(196,49)`，保持原尺寸 202×187，距离底部按钮仍留有空隙。
- 删除演示时钟。未联网同步时显示 `--:--` 和“等待网络校时”，不会冒充真实时间。
- Wi-Fi 配网保存到设备 NVS，重启自动重连；源码、固件及电脑脚本内不包含 Wi-Fi 密码。
- 免费 SNTP 校时：阿里云、腾讯、NTP Pool；北京时间固定 UTC+8。每小时重新同步，断网后系统继续走时。
- HTTPS Open-Meteo 当前天气，默认上海（31.2304,121.4737），每 15 分钟刷新。
- 首页显示实际天气名称、当天最低/最高气温、室外气温/湿度、天气数据时间；状态栏 TEMP/HUM 为板载 SHTC3 环境读数。
- 最低/最高温度采用北京时间当天的预报。底部显示数据更新时间；请求失败、断网或跨天缓存标记“旧数据”。
- 天气请求独立任务执行，8 秒网络超时，失败后 1/2/5 分钟退避；HTTP 429 后至少等待 15 分钟。手动刷新也有限频，避免连按刷接口。
- 校验 TLS 证书链及主机名，未成功校时不发起 HTTPS 请求；不使用不安全的证书跳过选项。

## 推荐更新方法：使用已编译固件并配网

解压本包，覆盖原来的 `HALO_Home_IDF` 项目。保留已有 `build/` 无妨。本包带有已验证的 `sdkconfig`，包含三台 NTP 服务器与 CA 证书包配置；请一并更新。

先退出占用串口的 `idf.py monitor`（Ctrl + ]）。最简单的方法是双击项目里的 **HALO_Update.command**；它使用你已有的 Conda 环境，完成烧录和当前 Wi-Fi 配网。

也可以运行：

```zsh
get_idf
cd /Users/huangle/Development/Workspace/Arduino/HALO_Home_IDF
python tools/update_device.py --flash
```

脚本默认端口 `/dev/cu.usbmodem1101`，自动读取当前 Wi-Fi，不绑定 BSSID。macOS 钥匙串可能要求允许访问该网络密码；若系统不提供网络名或密码，脚本会明确提示输入，不猜测网络名称。密码不写到文件、命令行参数或日志中。

脚本会：校验固件 → 烧录 → 发送配网信息 → 等待 Wi-Fi、NTP、天气分别报告成功。不要同时打开其他串口工具。最长观察 120 秒；失败会报告已完成的阶段，不把“发送命令”当成连接成功。

只重新配网、不烧录：

```zsh
python tools/update_device.py
```

明确指定端口或网络（自动识别不可用时）：

```zsh
python tools/update_device.py --port /dev/cu.usbmodem1101 --ssid 新网络名
```

不带 `--flash` 只配网。网络需支持 2.4 GHz。本版使用 ESP-IDF 默认 NVS 存储，未开启设备 Flash/NVS 加密；不等同于抵御物理提取的安全存储。

## 按现有方案自行编译

```zsh
get_idf
cd /Users/huangle/Development/Workspace/Arduino/HALO_Home_IDF
idf.py build
idf.py -p /dev/cu.usbmodem1101 flash monitor
```

首次配网仍可在退出 monitor 后运行 `python tools/update_device.py`。以后正常更新固件不擦除 NVS，无需重复配网。请勿执行 `erase-flash`，否则网络配置会丢失。

所有显示组件随项目提供，无需下载远程组件。受限环境如组件管理器不能访问进程信息，可用 `IDF_COMPONENT_MANAGER=0 idf.py build`，不跳过编译。

## 按键

| 按键 | 操作 |
|---|---|
| BOOT 短按 | 循环选择入口/菜单 |
| BOOT 长按 ≥700 ms | 首页请求刷新真实天气（受最短 60 秒限频约束） |
| KEY 短按 | 确认/打开 |
| KEY 长按 ≥700 ms | 首页打开菜单，其他页返回首页 |
| PWR | 保留硬件电源功能 |

BOOT/KEY 分别是 GPIO0/GPIO18。PWR 不是第三个可编程导航键。长按释放不会再触发短按。

菜单可刷新天气或暂停动画。文件与工具入口仍为占位说明页，尚未将 HTML 全部工具移植。电量仍显示 `--%`，没有编造百分比。

## 数据来源与可用性

- 时间：标准 SNTP，通过 ESP-IDF `esp_netif_sntp` 使用免费 NTP 服务器。成功同步后才显示“北京时间”。
- 天气：[Open-Meteo Forecast API](https://open-meteo.com/en/docs)，无 API Key；免费服务适用于个人非商业用途。天气为其数值天气模型提供的 current 数据，不是板载传感器的户外实测。
- 天气数据遵循 CC BY 4.0；菜单保留 Open-Meteo 来源信息。[免费服务条款](https://open-meteo.com/en/terms)
- 查询参数包含 current.temperature_2m / relative_humidity_2m / weather_code / wind_speed_10m，以及 daily.temperature_2m_min / max，timezone=Asia/Shanghai。
- WMO 天气码映射至原 20 种艺术场景，名称保持实际语义。例如毛毛雨复用雨景，阵雪复用雪景，雷雨冰雹使用冰雹场景；无有效天气数据时显示等待状态。
- 断网缓存仅保留在本次运行的内存；重新上电后重新获取。

## 联网故障定位

配网脚本会显示四阶段结果：保存配置、Wi-Fi 连接、北京时间同步、天气解析成功。

- Wi-Fi 未连接：核对密码、2.4 GHz、BSSID；可以不绑定 BSSID 重试。
- Wi-Fi 成功但时间未同步：检查路由器是否允许 UDP 123 和 DNS。
- 时间成功但天气失败：查看串口 `HALO_WEATHER` 的 HTTP 状态；网络需能访问 `api.open-meteo.com:443`。接口不可达时保留旧数据并重试，不伪造实时天气。
- SHTC3 读取失败：状态栏显示 `--`，不影响网络天气。

串口命令：`HELP`、`REFRESH`、`PAUSE`。兼容 `WIFI 网络名|密码`，但建议使用脚本，避免终端本地回显密码。

## 结构与验证

- `HaloModel.*`：按键状态、消抖、刷新请求。
- `HaloUi.*`：原生 U8g2 绘制。
- `HaloDevice.cpp`：Wi-Fi/NVS、SNTP、SHTC3、USB 配网。
- `HaloWeatherClient.*`：后台 HTTPS、限长响应缓冲、重试、UI 消息队列。
- `HaloWeatherData.*`：独立 JSON 验证及 WMO 映射。
- `components/u8g2_st7305`：微雪驱动，修正黑白极性并仅传输变化区域。
- `main/CampFrames.h`：原 HTML 插画生成的 20×8 帧黑白资源。

已通过：ESP-IDF 5.5.5 / ESP32-S3 目标编译、分区检查、原生 400×300 布局渲染、按键测试、28 个 WMO 码与雪景映射、缺失字段、错误/过期时间戳、跨天日预报、最低温大于最高温、解析失败不覆盖旧数据测试。

```zsh
sh tests/run_model_tests.sh
sh tests/run_weather_tests.sh
```

本次会话没有实机烧录/联网验证：USB 串口访问权限被拒绝，天气接口实连检查也受网络权限限制。布局预览中的天气数值是测试样例，不是已获取的实时上海天气。

## 资源与许可

`firmware/` 提供应用、引导、分区表、SHA256 与构建信息；脚本不会执行整片擦除。应用位于 0x10000，4 MB 分区；不是 OTA 包。

Waveshare Apache-2.0 许可位于 `licenses/`，U8g2 BSD 许可保留在组件目录与源文件。插画生成参考 `design/halo_v09.html`；`tools/capture_scene.cjs` 与 `tools/raster_assets.py` 可重建资源，需要 Node.js 和 Pillow。固件中的最终文字与位置以 `HaloUi.cpp` 为准。
