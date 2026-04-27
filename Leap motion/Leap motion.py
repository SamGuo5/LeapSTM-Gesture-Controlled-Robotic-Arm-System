import websocket
import json
import time
import socket

# ================= TCP 客户端配置 =================
# 这是刚才代码里设定的 ESP8266 的固定 AP 地址和端口
ESP8266_IP = "192.168.4.1"
ESP8266_PORT = 8080
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
last_command_sent = None

def connect_to_esp8266():
    try:
        print(f"正在尝试连接到下位机 ({ESP8266_IP}:{ESP8266_PORT})...")
        client_socket.connect((ESP8266_IP, ESP8266_PORT))
        print(">>> 成功连接到 ESP8266 机械臂控制器！ <<<")
        return True
    except Exception as e:
        print(f"连接失败: {e}")
        print("【避坑提示】请确保你的电脑 Wi-Fi 已经连接到了 'LeapMotion_Arm'！")
        return False

# ================= Leap Motion 识别逻辑 =================
last_print_time = 0
cooldown = 0.5  # 防抖时间：0.5秒
last_command_sent = None  # 记住上一次发送的动作指令

#开机保护机制变量
script_start_time = time.time()  # 记录程序启动的初始时间
INIT_DELAY = 10.0                 # 设置保护期为 10 秒

def on_message(ws, message):
    global last_print_time, last_command_sent, script_start_time
    
    current_time = time.time()
    
    # ========================================================
    elapsed_time = current_time - script_start_time
    if elapsed_time < INIT_DELAY:
        # 为了防止控制台疯狂刷屏，每隔 1 秒打印一次倒计时提示
        if current_time - last_print_time > 1.0:
            print(f"[{time.strftime('%H:%M:%S')}]  系统初始化中，等待机械臂复位... 剩余 {int(INIT_DELAY - elapsed_time) + 1} 秒")
            last_print_time = current_time
        return  # 直接返回，不执行下面的任何手势识别代码
    # ========================================================
    
    # 10 秒过后，正常解析 Leap Motion 数据
    data = json.loads(message)
    
    if "hands" in data and len(data["hands"]) > 0:
        # 防抖限制
        if current_time - last_print_time < cooldown:
            return 

        if "pointables" in data:
            extended_fingers = 0
            for p in data["pointables"]:
                if p.get("extended", False):
                    extended_fingers += 1
            
            command = None
            if extended_fingers == 0:
                command = 'A'  # 握拳
            elif extended_fingers >= 4:
                command = 'B'  # 张开
                
            # 双重拦截：有指令，且指令与上次不同，才执行发送
            if command and (command != last_command_sent):
                try:
                    client_socket.sendall(command.encode('utf-8'))
                    
                    if command == 'A':
                        print(f"[{time.strftime('%H:%M:%S')}]  状态切换: 握拳 -> 发送 'A'")
                    else:
                        print(f"[{time.strftime('%H:%M:%S')}]  状态切换: 张开 -> 发送 'B'")
                    
                    last_command_sent = command  
                    last_print_time = current_time 
                    
                except Exception as e:
                    print(f"TCP 发送数据失败，连接可能已断开: {e}")
                    
def on_error(ws, error):
    print(f"报错了: {error}")

def on_close(ws, close_status_code, close_msg):
    print("Leap Motion 连接已断开")
    client_socket.close()

def on_open(ws):
    print("=========================================")
    print("Leap Motion 传感器启动正常！")
    print("请将手放在传感器上方测试【握拳】和【张开】...")
    print("=========================================")
    ws.send(json.dumps({"background": True}))

if __name__ == "__main__":
    # 强制要求先连上单片机，再开启手势识别
    if connect_to_esp8266():
        ws_url = "ws://127.0.0.1:6437/v6.json"
        ws = websocket.WebSocketApp(ws_url,
                                    on_open=on_open,
                                    on_message=on_message,
                                    on_error=on_error,
                                    on_close=on_close)
        ws.run_forever()
    else:
        print("程序退出，请检查网络连接后重新运行。")