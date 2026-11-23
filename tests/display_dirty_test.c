#include "st7305_dirty.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 uint8_t old[304]={0},now[304]={0};int first=0,last=299;
 assert(!st7305_dirty_span(now,old,true,&first,&last));
 first=0;last=299;assert(st7305_dirty_span(now,old,false,&first,&last));assert(first==0&&last==299);
 for(int column=0;column<300;column++){
  memset(now,0,sizeof(now));now[column]=1;first=0;last=299;
  assert(st7305_dirty_span(now,old,true,&first,&last));assert(first==column&&last==column);
  int start=first/12*12,end=last/12*12+11;
  assert(start<=column&&end>=column&&end<300);
  for(int i=start;i<=end;i++)old[i]=now[i];
  first=0;last=299;assert(!st7305_dirty_span(now,old,true,&first,&last));
  memset(old,0,sizeof(old));
 }
 memset(now,0,sizeof(now));now[0]=0xff;now[299]=0xff;first=0;last=299;
 assert(st7305_dirty_span(now,old,true,&first,&last));assert(first==0&&last==299);
 puts("PASS initial full frame, identical-frame skip, all 300 one-column changes, 12-column panel alignment, cache convergence, edge columns.");
 return 0;
}
