// Included beside the shared drawing helpers.
void clippedText(u8g2_t *u,int x,int y,int width,const char *text){
 u8g2_SetClipWindow(u,x,y-16,x+width,y+3);label(u,x,y,text);u8g2_SetMaxClipWindow(u);
}
void filesUi(u8g2_t *u,const Model &m){
 const auto &s=m.files;bool reading=m.page==Page::Text;
 label(u,12,47,reading?"文本阅读 / UTF-8":"文件管理 / TF",u8g2_font_wqy16_t_gb2312);
 clippedText(u,12,64,376,reading?s.file:s.directory);
 if(reading){
  for(int i=0;i<10;i++)clippedText(u,12,85+i*16,376,s.lines[i]);
  char page[64];snprintf(page,sizeof(page),"第 %u 页%s",s.page+1,s.next?"":" / 文件结束");label(u,12,265,page);
 }else{
  for(unsigned i=0;i<s.count;i++){
   int y=74+i*29;if(m.selected==i){u8g2_DrawBox(u,8,y,384,27);u8g2_SetDrawColor(u,0);}
   label(u,16,y+19,s.entries[i].directory?"DIR":"FILE");clippedText(u,48,y+19,334,s.entries[i].name);u8g2_SetDrawColor(u,1);
  }
  for(int i=0;i<2;i++){int x=8+i*194;if(m.selected==s.count+i){u8g2_DrawBox(u,x,232,188,25);u8g2_SetDrawColor(u,0);}else u8g2_DrawFrame(u,x,232,188,25);label(u,x+12,250,i?"下一页":"上一页");u8g2_SetDrawColor(u,1);}
 }
 if(s.message[0]){u8g2_SetDrawColor(u,0);u8g2_DrawBox(u,8,261,390,18);u8g2_SetDrawColor(u,1);clippedText(u,12,275,378,s.message);}
}
