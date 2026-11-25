from pathlib import Path
import re,base64,io,json,math,sys
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parent.parent; html=Path(sys.argv[1]).read_text()
(root/'design/rlcd_clock_v032_fire_edge.html').write_text(html)
images=[Image.open(io.BytesIO(base64.b64decode(s))).convert('RGBA') for s in re.findall(r"'(iVBOR[^']+)'",html[html.index('const fireFrames='):html.index('let campLoaded')])]
bg=Image.open(io.BytesIO(base64.b64decode(re.search(r"campArt.src='data:image/png;base64,([^']+)",html)[1]))).convert('RGBA')
assert bg.size==(202,187) and len(images)==10
# V0.32 frame 5 has a three-pixel hole at the lower-left inner contour.
# Its neighboring frame 2 shares this exact lower contour. Repair at source
# resolution before generating the three weather strengths; keep all other art.
for x,y,rgba in [(14,31,(0,0,0,255)),(14,32,(0,0,0,255)),(13,32,(255,255,255,255))]:
 assert images[4].getpixel((x,y))[3]==0
 assert images[1].getpixel((x,y))==rgba
 images[4].putpixel((x,y),rgba)
out=['#pragma once\n#include <stdint.h>\n// V0.32 background and approved lower-contour fire sprites.']
def packed(im,pred):
 return [sum((1<<b) if x+b<im.width and pred(im.getpixel((x+b,y))) else 0 for b in range(8)) for y in range(im.height) for x in range(0,im.width,8)]
def arr(name,values):out.append('static const uint8_t '+name+'[]={'+','.join(map(str,values))+'};')
# Remove the reference art's unintended right border, source x=200.
ImageDraw.Draw(bg).line((200,0,200,186),fill='white')
arr('CAMP_BACKGROUND',packed(bg,lambda p:p[0]<150))
for k,scale in enumerate([1,.70,.43]):
 w,h=math.floor(29*scale+.5),math.floor(31*scale+.5)
 for f,im in enumerate(images):
  im=im.resize((w,h),Image.Resampling.NEAREST)
  # Draw each strength at its final pixel size before outlining, so scaling
  # cannot erase the one-pixel boundary. Padding preserves the flame anchor.
  outlined=Image.new('RGBA',(w+2,h+2));outlined.alpha_composite(im,(1,1))
  for y in range(math.floor(17*h/31+.5),h):
   for x in range(w):
    if im.getpixel((x,y))!=(255,255,255,255):continue
    for dx,dy in [(-1,0),(1,0),(0,-1),(0,1)]:
     nx,ny=x+dx,y+dy
     if not (0<=nx<w and 0<=ny<h) or im.getpixel((nx,ny))[3]==0:
      outlined.putpixel((nx+1,ny+1),(0,0,0,255))
  arr(f'FIRE_MASK_{k}_{f}',packed(outlined,lambda p:p[3]>=128));arr(f'FIRE_INK_{k}_{f}',packed(outlined,lambda p:p[3]>=128 and p[0]<150))
 out.append(f'static const uint8_t *const FIRE_MASK_{k}[]={{'+','.join(f'FIRE_MASK_{k}_{f}' for f in range(10))+'};')
 out.append(f'static const uint8_t *const FIRE_INK_{k}[]={{'+','.join(f'FIRE_INK_{k}_{f}' for f in range(10))+'};')
# Cloud outline sampled from the exact cubic control points in V0.32.
curves=[(-34,2,-35,-6,-27,-10),(-25,-17,-17,-19,-10,-14),(-4,-24,10,-23,15,-12),(25,-16,35,-8,30,0),(38,8,25,12,18,8),(10,11,-1,10,-8,8),(-16,10,-28,9,-27,3)]
def cubic(start,cs):
 pts=[start]
 for a,b,c,d,e,f in cs:
  x,y=pts[-1]
  for i in range(1,33):
   t=i/32;q=1-t;pts.append((q**3*x+3*q*q*t*a+3*q*t*t*c+t**3*e,q**3*y+3*q*q*t*b+3*q*t*t*d+t**3*f))
 return pts
for k,s in enumerate([.65,.50,.73]):
 im=Image.new('RGBA',(56,32));d=ImageDraw.Draw(im)
 cv=lambda ps:[(round(x*s+28),round(y*s+20)) for x,y in ps]
 ps=cv(cubic((-27,3),curves));d.polygon(ps,fill='white');d.line(ps+[ps[0]],fill='black',width=1)
 for start,curve in [((-21,-8),(-15,-11,-10,-8)),((7,-14),(15,-16,20,-10))]:
  x,y=start;a,b,c,e=curve;points=[((1-t)**2*x+2*(1-t)*t*a+t*t*c,(1-t)**2*y+2*(1-t)*t*b+t*t*e) for t in [i/32 for i in range(33)]];d.line(cv(points),fill='black')
 arr(f'CLOUD_MASK_{k}',packed(im,lambda p:p[3]>0));arr(f'CLOUD_INK_{k}',packed(im,lambda p:p[3]>0 and p[0]<150))
(root/'main/CampV032Assets.h').write_text('\n'.join(out)+'\n')
weather=json.loads(re.search(r'const weather=(\[.*?\]);',html)[1])


print('Extracted exact background, 10 RGBA flame frames, three strength levels and comic clouds')
