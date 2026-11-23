#!/usr/bin/env python3
"""Flash and provision from project-local configuration without macOS network/Keychain access."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time


def load_config(path):
    try:
        config=json.loads(path.read_text(encoding='utf-8'))
        wifi=config['wifi']; weather=config['weather']
        ssid=wifi['ssid']; password=wifi['password']
        if not isinstance(ssid,str) or not 1<=len(ssid.encode())<=32:raise ValueError()
        if not isinstance(password,str) or (password and not 8<=len(password.encode())<=63):raise ValueError()
        city=weather['city'];lat=weather['latitude'];lon=weather['longitude']
        if not isinstance(city,str) or not 1<=len(city.encode())<=24:raise ValueError()
        if isinstance(lat,bool) or isinstance(lon,bool) or not isinstance(lat,(int,float)) or not isinstance(lon,(int,float)) or not -90<=lat<=90 or not -180<=lon<=180:raise ValueError()
        port=config.get('port','/dev/cu.usbmodem1101')
        if not isinstance(port,str) or not port.startswith('/dev/cu.'):raise ValueError()
        return ssid,password,{'city':city,'latitude':lat,'longitude':lon},port
    except (KeyError,TypeError,ValueError):
        raise RuntimeError('配置格式错误：请检查 wifi、weather 和 port 字段；不会输出配置中的密码。') from None


def wifi_failure(reason):
    if reason in (201,210,211,212):return '未找到匹配接入点或安全设置不匹配；检查 2.4 GHz、网络名称和信号。'
    if reason in (15,202,204):return '认证或握手失败；检查密码、路由器安全设置和信号。'
    if reason==200:return '接入点信号超时。'
    return '连接断开，设备会自动重试。'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parent.parent
    parser.add_argument('--config',type=Path,default=None)
    parser.add_argument('--port',default=None)
    parser.add_argument('--flash',action='store_true')
    args=parser.parse_args()
    config_path=args.config or (root/'halo.config.local.json')
    if args.config is None and not config_path.exists():config_path=root/'halo.config.json'
    args.ssid,password,weather,configured_port=load_config(config_path)
    args.port=args.port or configured_port
    args.bssid=''
    print('读取本地配置：'+config_path.name)
    print('目标 Wi-Fi：'+args.ssid+'；天气城市：'+weather['city'])
    import serial
    if args.flash:
        for line in (root/'firmware/SHA256SUMS').read_text().splitlines():
            checksum, name = line.split('  ')
            if hashlib.sha256((root/'firmware'/name).read_bytes()).hexdigest() != checksum:
                raise RuntimeError('Firmware checksum mismatch: '+name)
        subprocess.run([sys.executable,'-m','esptool','--chip','esp32s3','--port',args.port,'--baud','460800',
                        '--before','default_reset','--after','hard_reset','write_flash',
                        '--flash_mode','dio','--flash_size','16MB','--flash_freq','80m',
                        '0x0',str(root/'firmware/bootloader.bin'),
                        '0x8000',str(root/'firmware/partition-table.bin'),
                        '0x10000',str(root/'firmware/halo_home.bin')],check=True)
        time.sleep(2)
    port = serial.Serial()
    port.port=args.port;port.baudrate=115200;port.timeout=.3;port.write_timeout=3
    port.dtr=False;port.rts=False
    flags={'saved':False,'wifi':False,'time':False,'weather':False}
    with port:
        # Discard earlier boot logs; acknowledgements below correspond to this request.
        port.reset_input_buffer()
        # Settings are acknowledged before sending Wi-Fi credentials.
        settings=('SETTINGSJSON '+json.dumps(weather,ensure_ascii=False,separators=(',',':'))+'\n').encode()
        port.write(settings);port.flush()
        settings_deadline=time.monotonic()+10
        acknowledged=False
        while time.monotonic()<settings_deadline:
            response=port.readline()
            if b'SETTINGS_SAVED' in response:acknowledged=True;break
            if b'SETTINGS_REJECTED' in response:raise RuntimeError('开发板拒绝了城市配置')
        if not acknowledged:raise RuntimeError('城市配置未确认，请检查固件版本和串口')
        print('默认天气城市已保存。')
        payload={'ssid':args.ssid,'password':password,'bssid':args.bssid}
        line=('WIFIJSON '+json.dumps(payload,ensure_ascii=False,separators=(',',':'))+'\n').encode()
        if len(line)>512:raise RuntimeError('Provisioning command exceeds device limit')
        for offset in range(0,len(line),64):
            port.write(line[offset:offset+64]);port.flush();time.sleep(.03)
        print('已发送配网信息，等待连接、校时和天气更新…')
        deadline=time.monotonic()+120
        pending=b''
        while time.monotonic()<deadline:
            pending+=port.read(1024)
            while b'\n' in pending:
                raw,pending=pending.split(b'\n',1)
                text=raw.decode(errors='replace').strip()
                if password:text=text.replace(password,'[REDACTED]')
                if 'WIFI_CONFIG_SAVED' in text:flags['saved']=True;print('Wi-Fi 配置已保存到开发板。')
                elif 'WIFI_CONNECTED' in text:flags['wifi']=True;print('Wi-Fi 已连接。')
                elif 'SNTP_SYNC_OK' in text:flags['time']=True;print('北京时间已同步。')
                elif 'HALO_WEATHER' in text and 'parsed=1' in text:flags['weather']=True;print('真实天气与最低/最高气温已更新。')
                elif 'WIFI_DISCONNECTED' in text:
                    reason=re.search(r'reason=(\d+)',text)
                    if reason:print('Wi-Fi 失败原因 '+reason.group(1)+'：'+wifi_failure(int(reason.group(1))))
                elif 'HALO_WEATHER' in text or 'WIFI_CONFIG_REJECTED' in text or 'WIFI_SAVE_FAILED' in text:print(text)
            if all(flags.values()):print('完成。后续开机自动重连。');return
            if len(pending)>8192:pending=pending[-4096:]
        print('等待超时，已确认：'+json.dumps(flags,ensure_ascii=False))
        print('如 Wi-Fi 未连接，检查本地配置中的 2.4 GHz 网络名称和密码；如时间成功但天气失败，请查看 HTTP 日志。')
        raise SystemExit(2)

if __name__=='__main__':
    try:main()
    except KeyboardInterrupt:raise SystemExit(130)
    except Exception as exc:
        print('操作未完成：'+str(exc),file=sys.stderr)
        raise SystemExit(1)
