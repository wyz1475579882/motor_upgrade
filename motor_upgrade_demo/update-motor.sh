#!/bin/bash

# 定义最大尝试次数
max_attempts=10
# 计数器初始化为0
counter=0

systemctl stop tita-bringup.service

sleep 3

echo "正在执行所有电机通讯查询与版本号查询,请等待一分钟..."

# 定义一个函数来执行OTA命令并检查输出
ota_update1() {
    local output
    # 执行OTA命令并捕获输出
    output=$(otafifth_demo -f new_zephyr.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* || $output == *"failed"* ]]; then
        return 0  # 成功，返回0
    else
        return 1  # 失败，返回非0值
    fi
}

get_current_version() {
    echo "正在查询当前系统版本号..."
    
    # 设置超时时间（秒）
    local timeout=3
    local version_info=""
    local tmpfile=$(mktemp)
    
    # 先启动后台监听
    echo "启动后台 CAN 数据监听..."
    candump -ta -x can0,001:DFFFFFFF > "$tmpfile" 2>&1 &
    local candump_pid=$!
    
    # 等待监听器启动
    sleep 0.1
    
    # 发送版本查询指令
    echo "发送版本查询指令: can-app -Version"
    can-app -Version
    
    # 等待指定时间收集响应
    echo "等待响应（超时时间: ${timeout}秒）..."
    sleep $timeout
    
    # 停止后台监听
    kill $candump_pid 2>/dev/null
    wait $candump_pid 2>/dev/null
    
    # 读取收集到的数据
    version_info=$(cat "$tmpfile")
    rm -f "$tmpfile"
    
    if [ -n "$version_info" ]; then
        echo "接收到的版本号信息: $version_info"
    else
        echo "版本号查询失败或超时，未接收到数据"
        return 1
    fi
}

# 新增：完整的版本查询流程
query_version_with_can() {
    echo "=== 开始CAN总线版本号查询 ==="
    
    # 调用版本查询函数
    if get_current_version; then
        return 0
    else
        return 1
    fi
}

# 定义一个函数来执行OTA命令并检查输出
ota_update_version() {
    local output
    # 执行OTA命令并捕获输出
    output=$(otafifth_demo -f zephyr_version.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* || $output == *"failed"* ]]; then
        return 0  # 成功，返回0
    else
        return 1  # 失败，返回非0值
    fi
}

while [[ $counter -lt $max_attempts ]]; do
    # 调用OTA更新函数
    if ota_update_version; then
        echo "Success,please wait for the version number to be checked..."
        break  # 成功，跳出循环
    else
        echo "zephyr update failed, retrying... ($counter/$max_attempts)"
        sleep 1  # 暂停一秒再重试
        counter=$((counter+1))
    fi
done

if query_version_with_can; then
    echo "版本号查询成功"
    
    # 询问用户是否开始升级流程
    while true; do
        read -p "是否开始升级流程？[Y/n] " answer
        case $answer in
            [Yy]|"" )  # Y/y 或直接回车
                echo "开始升级流程..."
                break
                ;;
            [Nn] )  # N/n
                echo "用户取消升级流程，请等待一分钟版本回溯后自动退出..."
                sleep 2
                otafifth_demo -f new_zephyr.bin
                exit 0
                ;;
            * )
                echo "请输入 Y(是) 或 N(否)。"
                ;;
        esac
    done
else
    echo "版本号查询失败，退出升级流程，请等待..."
    exit 1
fi
echo "============================="

ota_update() {
    local output
    # 执行OTA命令并捕获输出
    output=$(otafifth_demo -f old_zephyr.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* || $output == *"failed"* ]]; then
        return 0  # 成功，返回0
    else
        return 1  # 失败，返回非0值
    fi
}

# 循环直到OTA成功或达到最大尝试次数
while [[ $counter -lt $max_attempts ]]; do
    # 调用OTA更新函数
    if ota_update; then
        echo "old zephyr update successful, continuing..."
        break  # 成功，跳出循环
    else
        echo "old zephyr update failed, retrying... ($counter/$max_attempts)"
        sleep 1  # 暂停一秒再重试
        counter=$((counter+1))
    fi
done

# 检查是否成功完成OTA更新
if [[ $counter -eq $max_attempts ]]; then
    echo "Max attempts reached without success, exiting..."
    exit 1
fi

# OTA更新成功，执行下一步操作
motor_update() {
    local output=""
    # 执行OTA命令并捕获输出
    output=$(/usr/motor_upgrade/run.sh -t 3 -i 4 -s 1 -b wheel.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 3 4 1 update successful, continuing..."
    else
        echo "motor 3 4 1 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 3 -i 4 -s 2 -b wheel.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 3 4 2 update successful, continuing..."
    else
        echo "motor 3 4 2 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 1 -s 1 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 1 1 update successful, continuing..."
    else
        echo "motor 2 1 1 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 1 -s 2 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 1 2 update successful, continuing..."
    else
        echo "motor 2 1 2 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 2 -s 1 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 2 1 update successful, continuing..."
    else
        echo "motor 2 2 1 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 2 -s 2 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 2 2 update successful, continuing..."
    else
        echo "motor 2 2 2 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 3 -s 1 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 3 1 update successful, continuing..."
    else
        echo "motor 2 3 1 update fail, continuing..."
    fi

    output=""
    output=$(/usr/motor_upgrade/run.sh -t 2 -i 3 -s 2 -b slow.bin 2>&1)
    # 检查输出是否包含"success"
    if [[ $output == *"success"* ]]; then
        echo "motor 2 3 2 update successful, continuing..."
    else
        echo "motor 2 3 2 update fail, continuing..."
    fi

    return 0
}

# 循环直到OTA成功或达到最大尝试次数
while [[ $counter -lt $max_attempts ]]; do
    # 调用OTA更新函数
    if motor_update; then
        echo "motor update successful, continuing..."
        break  # 成功，跳出循环
    else
        echo "motor update failed!!!"
        break
    fi
done

# 循环直到OTA成功或达到最大尝试次数
while [[ $counter -lt $max_attempts ]]; do
    # 调用OTA更新函数
    if ota_update1; then
        echo "new zephyr update successful, continuing..."
        break  # 成功，跳出循环
    else
        echo "new zephyr update failed, retrying... ($counter/$max_attempts)"
        sleep 1  # 暂停一秒再重试
        counter=$((counter+1))
    fi
done