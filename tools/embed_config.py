#!/usr/bin/env python3
"""Embed the project's default configuration without printing credentials."""
import json,sys
from pathlib import Path
source,target=map(Path,sys.argv[1:])
value=json.loads(source.read_text(encoding='utf-8-sig'))
# Fail a build with invalid defaults rather than silently embedding unusable values.
import math
try:
    if not isinstance(value,dict): raise ValueError()
    for section in ('device','wifi','weather'):
        if section in value and not isinstance(value[section],dict): raise ValueError()
    for section,key,limit,empty in [('device','name',48,False),('wifi','ssid',32,True),('wifi','password',63,True),('weather','city',48,False)]:
        if key not in value.get(section,{}): continue
        text=value[section][key]
        if not isinstance(text,str) or len(text.encode())>limit or (not empty and not text) or any(ord(c)<32 for c in text): raise ValueError()
    password=value.get('wifi',{}).get('password','')
    if password and len(password.encode())<8: raise ValueError()
    for key,limit in [('latitude',90),('longitude',180)]:
        if key not in value.get('weather',{}): continue
        coordinate=value['weather'][key]
        if isinstance(coordinate,bool) or not isinstance(coordinate,(int,float)) or not math.isfinite(coordinate) or abs(coordinate)>limit: raise ValueError()
except (ValueError,TypeError):
    raise SystemExit('Invalid default configuration fields (values omitted)') from None
# The serial port is a host-only setting.
value.pop('port',None)
raw=json.dumps(value,ensure_ascii=True,separators=(',',':')).encode()
if len(raw)>8192:raise SystemExit('Default configuration is too large')
target.write_text('#pragma once\nstatic const char DEFAULT_CONFIG_JSON[]={'+','.join(str(b if b<128 else b-256) for b in raw)+',0};\n')
