#!/bin/bash

while getopts ":b:t:i:s:" opt; do
  case $opt in
    b)
      echo "bin的参数为: $OPTARG"
      export m_bin=$OPTARG
      ;;
    t)
      echo "电机类型的参数为: $OPTARG"
      export m_type=$OPTARG
      ;;
    i)
      echo "电机id的参数为: $OPTARG"
      export m_id=$OPTARG
      ;;
    s)
      echo "can bus的参数为: $OPTARG"
      export m_side=$OPTARG
      ;;
    \?)
      echo "无效的选项: -$OPTARG" >&2
      ;;
  esac
done

rm /usr/motor_upgrade/OTA.bin 2> /dev/null 
rm /usr/motor_upgrade/ota_file_data.c 2> /dev/null
cp $m_bin /usr/motor_upgrade/OTA.bin
cd /usr/motor_upgrade
python upgrade_case.py > upgrade_motor.log
make clean > upgrade_motor.log
make > upgrade_motor.log
./can-app -MotorUpgrade $m_type $m_id $m_side
