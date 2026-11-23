// Shared labels and icons are provided by HaloUi.cpp.
void settingsRow(u8g2_t *u,const Model &m,int index,int y,const char *name,const char *value,int glyph){
  if(m.selected==index){u8g2_DrawBox(u,8,y,384,36);u8g2_SetDrawColor(u,0);}
  toolGlyph(u,19,y+9,glyph);label(u,44,y+25,name,u8g2_font_wqy16_t_gb2312);
  label(u,245,y+24,value);u8g2_SetDrawColor(u,1);
}
void settingsUi(u8g2_t *u,const Model &m){
  if(m.page==Page::Provinces||m.page==Page::Cities){
    bool province=m.page==Page::Provinces;
    label(u,12,55,province?"选择省份":PROVINCES[m.cityProvince],u8g2_font_wqy16_t_gb2312);
    int count=province?4:CITY_COUNTS[m.cityProvince],start=m.selected/5*5;
    for(int i=start;i<count&&i<start+5;i++){
      const char *name=province?PROVINCES[i]:CITIES[CITY_STARTS[m.cityProvince]+i].name;
      const char *chosen=!province&&strcmp(name,m.city)==0?"已选择":"";
      settingsRow(u,m,i,72+(i-start)*37,name,chosen,3);
    }
    char pagination[64];snprintf(pagination,sizeof(pagination),"%d / %d  ·  KEY 确认，长按返回",m.selected+1,count);label(u,12,267,pagination);return;
  }
  label(u,12,55,"系统设置",u8g2_font_wqy16_t_gb2312);
  settingsRow(u,m,0,75,"时钟格式",m.preferences.hour12?"12 小时 / AM-PM":"24 小时",0);
  settingsRow(u,m,1,119,"显示秒数",m.preferences.showSeconds?"开启":"关闭",0);
  settingsRow(u,m,2,163,"天气城市",m.city,3);
  char period[24];snprintf(period,sizeof(period),"%u 分钟",unsigned(m.preferences.weatherMinutes));
  settingsRow(u,m,3,207,"天气更新周期",period,1);
  label(u,12,267,m.settingsMessage);
}
