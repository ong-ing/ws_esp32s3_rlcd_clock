#include "HaloCamp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static u8x8_display_info_t info;
uint8_t display(u8x8_t *u,uint8_t msg,uint8_t,void*){if(msg==U8X8_MSG_DISPLAY_SETUP_MEMORY)u8x8_d_helper_display_setup_memory(u,&info);return 1;}
int main(){
 info.tile_width=50;info.tile_height=38;info.pixel_width=400;info.pixel_height=300;
 u8g2_t u;uint8_t pixels[50*38*8]={},expected[sizeof(pixels)]={};
 u8x8_Setup(u8g2_GetU8x8(&u),display,u8x8_dummy_cb,u8x8_dummy_cb,u8x8_dummy_cb);
 u8g2_SetupBuffer(&u,pixels,38,u8g2_ll_hvline_vertical_top_lsb,U8G2_R0);
 u8g2_SetBitmapMode(&u,1);
 unsigned count=0;
 for(unsigned weather=0;weather<20;weather++)for(unsigned frame=0;frame<3360;frame+=37){
  u8g2_ClearBuffer(&u);halo::camp::draw(&u,weather,frame);
  for(int y=0;y<300;y++)for(int x=0;x<400;x++)
   if(x<196||x>=396||y<49||y>=236)assert(!(pixels[y/8*400+x]&(1<<(y%8))));
  memcpy(expected,pixels,sizeof(pixels));
  halo::camp::draw(&u,(weather+3)%20,(frame+11)%3360);
  halo::camp::draw(&u,weather,frame);
  assert(memcmp(expected,pixels,sizeof(pixels))==0);count++;
 }
 u8g2_DrawPixel(&u,399,299);assert(pixels[(299/8)*400+399]&(1<<(299%8)));
 printf("PASS %u scene samples: all 20 weather types clipped to x<396; frame restoration and UI clip reset\n",count);
}
