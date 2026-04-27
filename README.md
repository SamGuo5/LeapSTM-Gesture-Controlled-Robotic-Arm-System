# LeapSTM 手势控制机械臂系统

## 项目简介
基于Leap Motion和STM32的手势控制机械臂系统，通过Leap Motion双目红外摄像头检测手势，在上位机端进行手势识别，通过ESP8266创建的局域网无线传输数据给STM32下位机，控制机械臂运动。

## 功能特点
- 集成Leap Motion手势检测
- 实现ESP8266无线数据传输
- 完成STM32机械臂控制
- 实现基本手势识别算法

## 硬件要求
- Leap Motion控制器
- ESP8266开发板
- STM32开发板
- 机械臂系统

## 软件依赖
- Python（上位机手势识别）
- Arduino IDE（ESP8266编程）
- STM32CubeIDE（STM32编程）

## 项目结构
- `ESP8266_TCP/` - ESP8266无线传输代码
- `Leap motion/` - 上位机手势识别代码
- `stm32/` - STM32机械臂控制代码
- `robot-arm-parts/` - 机械臂系统硬件组件代码
- `control-board/` - 控制板

## 使用方法
1. 连接Leap Motion控制器到电脑
2. 上传ESP8266代码
3. 上传STM32代码
4. 运行上位机Python脚本
5. 通过手势控制机械臂

## 版本信息
- v1.0.0 - 初始版本，完成核心功能

## 许可证
MIT License
