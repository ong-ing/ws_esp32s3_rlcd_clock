#pragma once
#include "CampV032Assets.h"
#include "u8g2.h"
#include <math.h>
#include <stdlib.h>
namespace halo {
namespace camp {
constexpr int SCENE_LEFT=196, SCENE_TOP=49, SCENE_WIDTH=200, SCENE_HEIGHT=187;
// All layers share the same right boundary (screen x < 396).
// Scene-local signed coordinates are clipped before reaching U8g2.
inline void pixel(u8g2_t *u,int x,int y,bool ink) {
  if(x<0||x>=SCENE_WIDTH||y<0||y>=SCENE_HEIGHT)return;
  u8g2_SetDrawColor(u,ink?1:0);u8g2_DrawPixel(u,SCENE_LEFT+x,SCENE_TOP+y);
}
inline int rnd(double n){return int(floor(n+.5));}
inline double random(int n){double v=sin(n*127.1+311.7)*43758.5453;return v-floor(v);}
inline void line(u8g2_t *u,double ax,double ay,double bx,double by,bool ink=true){
 int x=rnd(ax),y=rnd(ay),xx=rnd(bx),yy=rnd(by);
 int dx=abs(xx-x),sx=x<xx?1:-1,dy=-abs(yy-y),sy=y<yy?1:-1,err=dx+dy;
 for(;;){pixel(u,x,y,ink);if(x==xx&&y==yy)break;int e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}
}
inline void sprite(u8g2_t *u,int x,int y,int w,int h,const uint8_t *mask,const uint8_t *ink){
 for(int j=0;j<h;j++)for(int i=0;i<w;i++){int b=j*((w+7)/8)+i/8;uint8_t bit=1<<(i%8);if(mask[b]&bit)pixel(u,x+i,y+j,ink[b]&bit);}
}
inline void cloud(u8g2_t *u,double x,double y,int size,double phase){
 const uint8_t *m[]={CLOUD_MASK_0,CLOUD_MASK_1,CLOUD_MASK_2};
 const uint8_t *b[]={CLOUD_INK_0,CLOUD_INK_1,CLOUD_INK_2};
 sprite(u,rnd(x)-28,rnd(y+sin(phase)*.7)-20,56,32,m[size],b[size]);
}
inline void draw(u8g2_t *u,unsigned weather,unsigned frame){
 // Restore the entire scene in RAM. The existing ST7305 dirty-span driver sends
 // only changed bytes to the panel; no alternating or temporal grey patterns.
 u8g2_SetDrawColor(u,0);u8g2_DrawBox(u,196,49,202,187);
 u8g2_SetClipWindow(u,SCENE_LEFT,SCENE_TOP,SCENE_LEFT+SCENE_WIDTH,SCENE_TOP+SCENE_HEIGHT);
 u8g2_SetDrawColor(u,1);u8g2_DrawXBMP(u,196,49,202,187,CAMP_BACKGROUND);
 const unsigned w=weather%20;const double f=frame;
 const int rain[]={0,0,25,60,0,45,75,40,0,0,0,25,0,0,0,0,20,35,90,0};
 const int snow[]={0,0,0,0,0,0,0,0,24,45,70,25,0,0,0,0,0,0,0,85};
 int strength=(w==3||rain[w]>=70||snow[w]>=70)?2:(rain[w]||snow[w])?1:0;
 const int widths[]={31,22,14},heights[]={33,24,15};
 const double severity[]={1,.70,.43};
 const uint8_t *const *masks[]={FIRE_MASK_0,FIRE_MASK_1,FIRE_MASK_2};
 const uint8_t *const *inks[]={FIRE_INK_0,FIRE_INK_1,FIRE_INK_2};
 unsigned idx=(frame*2/3)%10;
 sprite(u,rnd(99-widths[strength]/2.0),141-heights[strength],widths[strength],heights[strength],masks[strength][idx],inks[strength][idx]);
 if(strength<2)for(int i=0;i<(strength==0?5:2);i++){
  double age=fmod(f*1.15+i*9.4,32);
  int x=rnd(99+(random(i+91)-.5)*16+sin(f*.23+i*1.7)*2);
  int y=rnd(139-31*severity[strength]-2-age*.8);
  if(y<61||y>140)continue;
  pixel(u,x,y,i%4!=0);if(i%3==1)pixel(u,x,y+1,i%4!=0);if(i%3==2)pixel(u,x+1,y,i%4!=0);
 }
 bool clouds=w==1||w==4||rain[w]||snow[w];
 if(clouds){cloud(u,22+fmod(f*.23,230),27,0,f*.028);cloud(u,157-fmod(f*.14,230),14,1,f*.017);if(w==3||w==4)cloud(u,106+sin(f*.013)*11,39,2,f*.013);}
 for(int i=0;i<rnd(rain[w]*.65);i++){
  double x=fmod(random(i+700)*202+f*1.5,202),y=35+fmod(random(i+820)*133+f*4.1,133);
  line(u,x,y,x-2,y+(rain[w]>50?6:4),i%4!=0);
 }
 for(int i=0;i<rnd(snow[w]*.7);i++){
  int x=int(fmod(random(i+800)*202+sin(f*.02+i)*3,202));
  int y=int(20+fmod(random(i+830)*148+f*(.55+i%3*.15),148));
  int size=i%3==0?2:1;for(int a=0;a<size;a++)for(int b=0;b<size;b++)pixel(u,x+a,y+b,i%5!=0);
 }
 if(w==3&&frame%62<4){
  const int xs[]={145,137,144,135,153,145},ys[]={32,46,46,62,41,41};
  for(int y=32;y<=62;y++)for(int x=135;x<=153;x++){
   bool inside=false;for(int i=0,j=5;i<6;j=i++)if((ys[i]>y)!=(ys[j]>y)&&x<(xs[j]-xs[i])*double(y-ys[i])/(ys[j]-ys[i])+xs[i])inside=!inside;
   if(inside)pixel(u,x,y,true);
  }
 }
 if(w==12||w==13||w==15)for(int j=0;j<2;j++)line(u,20+sin(f*.02+j)*7,84+j*19,92+sin(f*.02+j)*7,84+j*19,false);
 if(w==14||w==15)for(int i=0;i<8;i++){double x=fmod(random(i+140)*250+f*2,250)-25;line(u,x,33+i*16,x+14,32+i*16);}
 u8g2_SetMaxClipWindow(u);
 u8g2_SetDrawColor(u,1);
}
} // namespace camp
} // namespace halo
