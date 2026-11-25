// Included inside ClockUi.cpp after the shared label/icon helpers.
void toolButton(u8g2_t *u,const Model &m,int index,int count,const char *text){
  int width=384/count,x=8+index*width;
  if(m.selected==index){u8g2_DrawBox(u,x,241,width-4,34);u8g2_SetDrawColor(u,0);}
  else u8g2_DrawFrame(u,x,241,width-4,34);
  label(u,x+8,263,text);u8g2_SetDrawColor(u,1);
}
void durationText(char *out,size_t size,uint64_t milliseconds){
  uint64_t seconds=milliseconds/1000;
  snprintf(out,size,"%02llu:%02u:%02u",(unsigned long long)(seconds/3600),unsigned(seconds/60%60),unsigned(seconds%60));
}
void toolGlyph(u8g2_t *u,int x,int y,int kind){
  if(kind==0){u8g2_DrawCircle(u,x+7,y+8,6,U8G2_DRAW_ALL);u8g2_DrawBox(u,x+5,y,5,2);u8g2_DrawLine(u,x+7,y+8,x+7,y+4);u8g2_DrawLine(u,x+7,y+8,x+10,y+8);}
  else if(kind==1){u8g2_DrawHLine(u,x,y,15);u8g2_DrawHLine(u,x,y+14,15);u8g2_DrawLine(u,x+2,y+1,x+12,y+13);u8g2_DrawLine(u,x+12,y+1,x+2,y+13);}
  else if(kind==2){u8g2_DrawCircle(u,x+7,y+8,6,U8G2_DRAW_ALL);u8g2_DrawLine(u,x+7,y+2,x+7,y);u8g2_DrawLine(u,x+7,y+2,x+11,y);}
  else if(kind==5){u8g2_DrawCircle(u,x+7,y+7,5,U8G2_DRAW_ALL);u8g2_DrawCircle(u,x+7,y+7,2,U8G2_DRAW_ALL);u8g2_DrawVLine(u,x+7,y,3);u8g2_DrawVLine(u,x+7,y+12,3);u8g2_DrawHLine(u,x,y+7,3);u8g2_DrawHLine(u,x+12,y+7,3);}
  else {u8g2_DrawFrame(u,x,y+2,15,11);u8g2_DrawHLine(u,x+2,y+5,11);if(kind==3){u8g2_DrawVLine(u,x+4,y,4);u8g2_DrawVLine(u,x+10,y,4);u8g2_DrawBox(u,x+4,y+8,3,3);}else{u8g2_DrawVLine(u,x+7,y+13,2);u8g2_DrawHLine(u,x+3,y+15,9);}}
}
void toolsUi(u8g2_t *u,const Model &m){
  const auto &t=m.tools;char text[64];
  if(m.page==Page::Tools){
    label(u,12,55,"工具箱",u8g2_font_wqy16_t_gb2312);
    const char *names[]={"秒表 / 计圈","倒计时","番茄钟","日历","设备监控","系统设置"};
    int start=(m.selected/5)*5;
    for(int i=start;i<TOOL_COUNT&&i<start+5;i++){
      int y=73+(i-start)*37;
      if(m.selected==i){u8g2_DrawBox(u,8,y,384,34);u8g2_SetDrawColor(u,0);}
      toolGlyph(u,19,y+8,i);label(u,44,y+23,names[i],u8g2_font_wqy16_t_gb2312);u8g2_SetDrawColor(u,1);
    }
    label(u,12,267,start?"2 / 2  ·  系统设置":"1 / 2  ·  继续选择可进入系统设置");return;
  }
  if(m.page==Page::Monitor){
    label(u,12,55,"设备监控",u8g2_font_wqy16_t_gb2312);
    char elapsed[32];durationText(elapsed,sizeof(elapsed),t.uptimeMs);
    snprintf(text,sizeof(text),"运行时间  %s",elapsed);label(u,16,86,text);
    if(!t.monitorValid)label(u,16,116,"等待设备采样");
    else {
      snprintf(text,sizeof(text),"SRAM 可用 %lu / %lu KB",(unsigned long)(t.heapFree/1024),(unsigned long)(t.heapTotal/1024));label(u,16,112,text);
      snprintf(text,sizeof(text),"PSRAM 可用 %lu / %lu KB",(unsigned long)(t.psramFree/1024),(unsigned long)(t.psramTotal/1024));label(u,16,138,t.psramTotal?text:"PSRAM 未启用");
      snprintf(text,sizeof(text),"Wi-Fi 信号 %d dBm",t.wifiRssi);label(u,16,164,m.connected?text:"Wi-Fi 未连接");
      snprintf(text,sizeof(text),"IP  %s",m.connected?t.ip:"--");label(u,16,190,text);
      snprintf(text,sizeof(text),"电池 %.2f V / %u%% (估算)",m.batteryVoltage,unsigned(m.batteryPercent));label(u,16,216,m.batteryValid?text:"电池数据不可用");
    }
    toolButton(u,m,0,1,"返回工具箱");return;
  }
  if(m.page==Page::Calendar){
    CalendarView v=calendarView(m);
    label(u,12,55,"日历",u8g2_font_wqy16_t_gb2312);
    if(!v.valid)label(u,16,120,"联网校时后显示北京日期");
    else {
      snprintf(text,sizeof(text),"%04d / %02d",v.year,v.month);label(u,265,55,text,u8g2_font_helvR14_tf);
      const char *week[]={"一","二","三","四","五","六","日"};
      for(int i=0;i<7;i++)label(u,24+i*54,80,week[i]);
      for(int day=1;day<=v.days;day++){
        int cell=v.weekday+day-1,x=12+cell%7*54,y=88+cell/7*24;
        if(day==v.today){u8g2_DrawBox(u,x,y,45,22);u8g2_SetDrawColor(u,0);}
        snprintf(text,sizeof(text),"%2d",day);label(u,x+10,y+16,text,u8g2_font_helvR14_tf);u8g2_SetDrawColor(u,1);
      }
    }
    toolButton(u,m,0,3,"上月");toolButton(u,m,1,3,"本月");toolButton(u,m,2,3,"下月");return;
  }
  if(m.page==Page::Pomodoro){
    label(u,12,55,"番茄钟",u8g2_font_wqy16_t_gb2312);
    label(u,250,55,t.pomoPhase==0?"专注 25 分钟":t.pomoPhase==1?"短休息 5 分钟":"长休息 15 分钟");
    durationText(text,sizeof(text),uint64_t(t.pomoRemaining)+999);label(u,24,123,text,u8g2_font_logisoso50_tn);
    snprintf(text,sizeof(text),"已完成 %lu 次专注",(unsigned long)t.pomoRounds);label(u,16,164,text);
    label(u,16,193,t.pomoDone?"阶段完成 / 按开始进入下一阶段":t.pomoRunning?"计时中 / 可离开此页":"每完成四次专注安排长休息");
    toolButton(u,m,0,2,t.pomoRunning?"暂停":"开始");toolButton(u,m,1,2,"重置本轮");return;
  }
  bool stopwatch=m.page==Page::Stopwatch;
  label(u,12,55,stopwatch?"秒表":"倒计时",u8g2_font_wqy16_t_gb2312);
  uint64_t value=stopwatch?t.stopwatchMs:uint64_t(t.countdownRemaining+999);
  durationText(text,sizeof(text),value);
  label(u,24,123,text,u8g2_font_logisoso50_tn);
  if(stopwatch){
    for(int i=0;i<3;i++)if(t.lapCount>unsigned(2-i)){
      char time[32];durationText(time,sizeof(time),t.laps[i]);
      snprintf(text,sizeof(text),"#%lu   %s",(unsigned long)(t.lapCount-2+i),time);label(u,18,157+i*24,text);
    }
    toolButton(u,m,0,3,t.stopwatchRunning?"暂停":"开始");toolButton(u,m,1,3,"计圈");toolButton(u,m,2,3,"重置");
  }else{
    label(u,16,164,t.countdownDone?"时间到！":t.countdownRunning?"计时中 / 可离开此页":"暂停时可调整分钟数");
    snprintf(text,sizeof(text),"设定 %lu 分钟",(unsigned long)(t.countdownDuration/60000));label(u,16,193,text);
    toolButton(u,m,0,4,t.countdownRunning?"暂停":"开始");toolButton(u,m,1,4,"减1分");toolButton(u,m,2,4,"加1分");toolButton(u,m,3,4,"重置");
  }
}
