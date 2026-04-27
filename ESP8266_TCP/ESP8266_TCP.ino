#include <ESP8266WiFi.h>

// 设定你的机械臂专属 Wi-Fi 网络
const char *ssid = "LeapMotion_Arm"; 
const char *password = "12345678";   // 密码至少需要8位

// 设定 TCP 服务器端口号
const int port = 8080;
WiFiServer server(port);

void setup() {
  // 初始化硬件串口，波特率设置为 115200 
  Serial.begin(115200);
  delay(100);

  // 将 ESP8266 设置为 AP 热点模式
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  // 启动 TCP 服务器，此时这块板子的默认 IP 就是 192.168.4.1
  server.begin();
}

void loop() {
  // 检查是否有客户端连接进来
  WiFiClient client = server.available();

  if (client) {
    while (client.connected()) {
      // 如果接收到 Python 通过 Wi-Fi 发来的数据
      if (client.available()) {
        char command = client.read();
        
        // 原封不动地通过 TX 引脚发给 STM32
        Serial.print(command); 
      }
    }
    // 客户端断开连接
    client.stop(); 
  }
}
