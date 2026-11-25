#include "ClockUi.h"
#include "ClockCamp.h"
#include "ClockStatusIcons.h"
#include "ClockCities.h"
#include <stdio.h>
#include <string.h>

namespace rlcd_clock {
namespace {
const char *const entries[] = {"无线网络", "文件管理", "工具箱"};
void label(u8g2_t *u, int x, int y, const char *s,
           const uint8_t *font = u8g2_font_wqy12_t_gb2312) {
  u8g2_SetFont(u, font);
  u8g2_DrawUTF8(u, x, y, s);
}
void icon(u8g2_t *u, int x, int y, int kind) {
  static const uint8_t patterns[4][7] = {
      {0x1c, 0x22, 0x41, 0x1c, 0x22, 0x08, 0x1c},
      {0x1e, 0x22, 0x7f, 0x41, 0x41, 0x41, 0x7f},
      {0x1c, 0x14, 0x7f, 0x49, 0x7f, 0x49, 0x7f},
      {0x08, 0x1c, 0x3e, 0x7f, 0x22, 0x2a, 0x3e}};
  for (int row = 0; row < 7; row++)
    for (int col = 0; col < 7; col++)
      if (patterns[kind % 4][row] & (1 << col))
        u8g2_DrawBox(u, x + col * 2, y + row * 2, 2, 2);
}
void top(u8g2_t *u, const Model &m) {
  u8g2_SetDrawColor(u, 1);
  u8g2_DrawBox(u, 0, 0, 400, 23);
  u8g2_SetDrawColor(u, 0);
  char values[3][24];
  snprintf(values[0],24,m.sensorValid?"TEMP %.1fC":"TEMP --.-C",m.temperature);
  snprintf(values[1],24,m.sensorValid?"HUM %.0f%%":"HUM --%%",m.humidity);
  char displayTime[6],period[3];clockText(m,displayTime,sizeof(displayTime),period,sizeof(period));
  snprintf(values[2],24,"TIME %s%s",displayTime,period);
  const int x[]={4,107,194};
  for(int i=0;i<3;i++)label(u,x[i],16,values[i],u8g2_font_wqy12_t_gb2312);
  u8g2_DrawXBMP(u,333,6,14,11,WIFI_MENU);
  if(!m.connected)u8g2_DrawLine(u,333,5,346,17);
  u8g2_DrawHLine(u,355,5,34);
  u8g2_DrawHLine(u,355,18,34);
  u8g2_DrawVLine(u,355,5,14);
  u8g2_DrawVLine(u,388,5,4);
  u8g2_DrawVLine(u,388,14,5);
  u8g2_DrawHLine(u,388,8,4);
  u8g2_DrawVLine(u,391,8,7);
  u8g2_DrawHLine(u,388,14,4);
  char battery[8];
  if(m.batteryValid)snprintf(battery,sizeof(battery),"%u%%",unsigned(m.batteryPercent));
  else snprintf(battery,sizeof(battery),"--%%");
  u8g2_SetFont(u,u8g2_font_wqy12_t_gb2312);
  int textWidth=u8g2_GetStrWidth(u,battery);
  label(u,355+(34-textWidth)/2,16,battery);

  u8g2_SetDrawColor(u, 1);
}
void home(u8g2_t *u, const Model &m) {
  camp::draw(u, m.weather, m.frame);
  label(u, 12, 46, m.timeSynced ? "BEIJING / 北京时间" : "BEIJING / 等待校时");
  char displayTime[6],period[3];clockText(m,displayTime,sizeof(displayTime),period,sizeof(period));
  label(u, 10, 108, displayTime, u8g2_font_logisoso50_tn);
  if(period[0]){
    u8g2_DrawBox(u,161,63,23,14);u8g2_SetDrawColor(u,0);
    label(u,165,74,period);u8g2_SetDrawColor(u,1);
  }
  if(m.preferences.showSeconds)label(u,162,108,m.seconds,u8g2_font_helvR14_tf);
  label(u, 12, 129, m.date);
  u8g2_DrawHLine(u, 12, 140, 172);
  const char *condition = m.weatherValid ? weatherDescription(m.outdoor.code)
                          : m.connected  ? "天气加载中"
                                         : "等待联网";
  label(u, 12, 175, condition, u8g2_font_wqy16_t_gb2312);
  char detail[128],displayCity[16];
  shortSsid(m.city,displayCity,sizeof(displayCity));
  if (m.weatherValid)
    snprintf(detail, sizeof(detail), "%s 低%.0f° / 高%.0f°", displayCity, m.outdoor.low,
             m.outdoor.high);
  else
    snprintf(detail, sizeof(detail), "%s 低--° / 高--°", displayCity);
  label(u, 12, 197, detail);
  char outdoor[40];
  if (m.weatherValid)
    snprintf(outdoor, sizeof(outdoor), "%.1f C / %.0f %%",
             m.outdoor.temperature, m.outdoor.humidity);
  else
    snprintf(outdoor, sizeof(outdoor), "--.- C / -- %%");
  label(u, 12, 219, outdoor, u8g2_font_helvR14_tf);
  char update[64];
  if (m.weatherValid)
    snprintf(update, sizeof(update), "%s %s", m.weatherUpdated,
             m.weatherStale  ? "旧数据"
             : m.weatherBusy ? "正在更新"
                             : "更新");
  else
    snprintf(update, sizeof(update), "%s",
             !m.connected      ? "等待联网"
             : !m.timeSynced   ? "等待校时"
             : m.weatherFailed ? "请求失败 / 将重试"
                               : "正在获取天气");
  label(u, 12, 236, update);
  for (int i = 0; i < 3; i++) {
    int x = 8 + i * 130;
    u8g2_DrawFrame(u,x,241,124,35);
    if (m.selected == i) {
      u8g2_DrawBox(u, x, 241, 124, 35);
      u8g2_SetDrawColor(u, 0);
    }
    if(i==0)u8g2_DrawXBMP(u,x+12,253,14,11,WIFI_MENU);
    else icon(u, x + 12, 251, i);
    label(u, x + 33, 264, entries[i], u8g2_font_wqy16_t_gb2312);
    u8g2_SetDrawColor(u, 1);
  }
}
void menu(u8g2_t *u, const Model &m) {
  label(u, 12, 55, m.deviceName, u8g2_font_wqy16_t_gb2312);
  const char *names[] = {"回到首页", "刷新网络天气",
                         m.animated ? "暂停营地动画" : "播放营地动画",
                         "按键与数据说明"};
  for (int i = 0; i < 4; i++) {
    int y = 72 + i * 44;
    if (i == m.selected) {
      u8g2_DrawBox(u, 8, y, 384, 38);
      u8g2_SetDrawColor(u, 0);
    }
    icon(u, 19, y + 12, i);
    label(u, 45, y + 26, names[i], u8g2_font_wqy16_t_gb2312);
    u8g2_SetDrawColor(u, 1);
  }
  label(u, 12, 271, "天气来源 Open-Meteo / CC BY 4.0");
}
void info(u8g2_t *u, const Model &m) {
  const char *title = m.selected < 3 ? entries[m.selected] : "HUANGLE / 操作说明";
  label(u, 12, 55, title, u8g2_font_wqy16_t_gb2312);
  if (m.selected == 0) {
    label(u, 12, 91, m.connected ? "Wi-Fi 已连接" : "Wi-Fi 未连接");
    label(u, 12, 118, m.connected ? m.ssid : m.networkStatus);
    label(u, 12, 145, "联网后自动同步北京时间");
    char interval[64];snprintf(interval,sizeof(interval),"天气 Open-Meteo / 每%u分钟更新",unsigned(m.preferences.weatherMinutes));
    label(u,12,172,interval);
  } else if (m.selected < 3) {
    label(u, 12, 91, "首页时间与天气已联网");
    label(u, 12, 118, "该功能将在后续固件中实现");
  } else {
    label(u, 12, 91, "BOOT 短按：循环选择");
    label(u, 12, 118, "BOOT 长按：首页刷新天气");
    label(u, 12, 145, "KEY 短按：确认");
    label(u, 12, 172, "KEY 长按：菜单或返回");
    label(u, 12, 199, "PWR：保留硬件开关机功能");
  }
  label(u, 12, 252, "KEY 短按或长按回到首页");
}
#include "ClockToolsUi.h"
#include "ClockSettingsUi.h"
#include "FileBrowserUi.h"
} // namespace
void render(u8g2_t *u, const Model &m) {
  u8g2_ClearBuffer(u);
  u8g2_SetFontMode(u, 1);
  u8g2_SetBitmapMode(u, 1);
  u8g2_SetDrawColor(u, 1);
  top(u, m);
  if(m.page==Page::Files||m.page==Page::Text)filesUi(u,m);
  else if (m.page == Page::Home)
    home(u, m);
  else if (m.page == Page::Menu)
    menu(u, m);
  else if(m.page==Page::Settings||m.page==Page::Provinces||m.page==Page::Cities)settingsUi(u,m);
  else if(m.page==Page::Tools||m.page==Page::Stopwatch||m.page==Page::Countdown||m.page==Page::Pomodoro||m.page==Page::Calendar||m.page==Page::Monitor)
    toolsUi(u,m);
  else
    info(u, m);
  u8g2_DrawHLine(u, 0, 281, 400);
  label(u, 9, 296, m.page==Page::Text?"BOOT 下一页 KEY 上一页 长按返回":m.page==Page::Files?"BOOT 选择 KEY 打开 长按返回":m.tools.countdownDone?"倒计时结束 / 进入工具箱查看":m.tools.pomoDone?"番茄钟阶段完成 / 进入工具箱查看":"BOOT 选择  KEY 确认  长按 KEY 返回");
  char name[16]; shortSsid(m.deviceName,name,sizeof(name));
  if(strcmp(m.deviceName,"HUANGLE")==0)label(u,358,295,m.deviceName,u8g2_font_5x8_tf);
  else label(u,348,295,name);
}
} // namespace rlcd_clock
