#!/bin/zsh
project_dir=${0:A:h}
python_path=/opt/anaconda3/envs/esp-idf-5.5/bin/python
if [[ ! -x "$python_path" ]]; then
  print '未找到已配置的 esp-idf-5.5 Python 环境。'
  read '?按回车关闭…'
  exit 1
fi
"$python_path" "$project_dir/tools/update_device.py" --flash
result=$?
print ''
read '?按回车关闭…'
exit "$result"
