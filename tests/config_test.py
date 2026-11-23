import importlib.util,json,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('update',root/'tools/update_device.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
base={'wifi':{'ssid':'测试网络','password':'test-pass-123'},'weather':{'city':'杭州','latitude':30.2741,'longitude':120.1551}}
with tempfile.TemporaryDirectory() as td:
 p=Path(td)/'config.json'
 def check(data,valid):
  p.write_text(json.dumps(data))
  try:result=mod.load_config(p)
  except RuntimeError as exc:
   assert not valid
   assert 'test-pass-123' not in str(exc)
  else:
   assert valid
   assert result[2]['city']=='杭州'
 check(base,True)
 for field,value in [('latitude',True),('latitude',91),('longitude',float('nan')),('city','城'*9)]:
  test=json.loads(json.dumps(base));test['weather'][field]=value;check(test,False)
 test=json.loads(json.dumps(base));test['wifi']['password']='bad';check(test,False)
 test=json.loads(json.dumps(base));test['wifi']['password']='';check(test,True)
source=(root/'tools/update_device.py').read_text()
assert 'networksetup' not in source and 'system_profiler' not in source and 'find-generic-password' not in source
print('PASS local config, UTF-8 fields, bounds/NaN rejection, redacted errors; no macOS Wi-Fi or Keychain queries')
