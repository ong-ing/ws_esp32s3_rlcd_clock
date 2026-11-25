from PIL import Image,ImageDraw
from pathlib import Path
import json,os
os.chdir(Path(__file__).resolve().parent.parent)
W,H=224,208
bayer=[[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]]
patterns={}
for density in range(17):
 p=Image.new('L',(W,H));p.putdata([0 if bayer[y%4][x%4]<density else 255 for y in range(H) for x in range(W)]);patterns[density]=p
from PIL import ImageChops

def mask(paths):
 m=Image.new('L',(W,H));d=ImageDraw.Draw(m)
 for path in paths:
  if len(path)>2:d.polygon([tuple(p) for p in path],fill=255)
 return m

def raster(ops):
 im=Image.new('L',(W,H),255)
 for op in ops:
  m=Image.new('L',(W,H));d=ImageDraw.Draw(m)
  for p in op['paths']:
   p=[tuple(v) for v in p]
   if op['kind']=='fill' and len(p)>2:d.polygon(p,fill=255)
   elif len(p)>1:d.line(p,fill=255,width=max(1,round(op['width'])))
  for c in op['clips']:m=ImageChops.multiply(m,mask(c))
  color=patterns[op['style']['density']] if isinstance(op['style'],dict) else (0 if op['style']=='#000' else 255)
  im.paste(color,(0,0),m)
 return im.resize((202,187),Image.Resampling.NEAREST).point(lambda p:0 if p<150 else 255,'1')

def xbm(im):
 a=[]
 for y in range(im.height):
  for x in range(0,im.width,8):
   v=0
   for b in range(8):
    if x+b<im.width and im.getpixel((x+b,y))==0:v|=1<<b
   a.append(v)
 return bytes(a)
assets=[]
for w in range(20):
 for f in range(8):
  im=raster(json.loads(Path(f'.asset-build/{w}-{f}.json').read_text()));assets.append(xbm(im))
  if f==0:im.save(f'.asset-build/weather-{w}.png')
Path('.asset-build/frames.bin').write_bytes(b''.join(assets))
out=Path('main/CampFrames.h')
with out.open('w') as f:
 f.write('#pragma once\n#include <stdint.h>\n// Generated from RLCD_CLOCK V0.9: 20 weather scenes, eight frames each, LSB-first XBM.\nstatic constexpr unsigned CAMP_WIDTH=202, CAMP_HEIGHT=187, CAMP_BYTES=4862;\nstatic const uint8_t CAMP_FRAMES[20][8][CAMP_BYTES] = {\n')
 for w in range(20):
  f.write('{\n')
  for frame in assets[w*8:w*8+8]:
   f.write('{'+','.join(str(v) for v in frame)+'},\n')
  f.write('},\n')
 f.write('};\n')
print('Generated',len(assets),'1-bit frames;',len(b''.join(assets)),'bytes in flash')
