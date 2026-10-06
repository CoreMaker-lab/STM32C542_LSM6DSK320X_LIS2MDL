# Overview
- **Name**: LSM6DSK320X-V1.0
- **MCU**: STM32C542CCT6
- **IDE**: STM32CUBEMX2+STM32CUBEIDE


# Buy Link
[https://shop192352884.taobao.com/](https://shop192352884.taobao.com/)





# Contact Information

- **Name**: Billy
- **交流群**: 925643491
- **Email**: a845656974@outlook.com
- **Phone**: +86 15622736378
- **CSDN Blog**: [Blog](https://blog.csdn.net/qq_24312945)
- **Video**: [Video](https://space.bilibili.com/26152390)



# Project Introduction
- **STM32C5_LSM6DSK320X_Project1**:STM32C5开发LSM6DSK320X(1)----轮询获取陀螺仪数据
- **CSDN Blog**:

LSM6DSK320X 是一款紧凑型、低功耗、高性能的惯性测量单元 (IMU)，集成了三轴数字低重力加速度计（满量程可选，最高可达 ±16 g）、三轴数字高重力加速度计（最高可达 ±320 g）和三轴数字陀螺仪。其先进的架构通过四个独立通道处理加速度和角速率数据：用户界面 (UI)、光学防抖 (OIS)、电子防抖 (EIS) 和高重力加速度通道。
LSM6DSK320X 专为要求苛刻的运动传感应用而设计，可实现高强度运动跟踪、冲击和脑震荡监测以及其他高动态应用场景。
该器件采用全新的陀螺仪架构，配备全差分读出链和增强的机械对称性，从而具备出色的抗振性和抗外部干扰能力。它还支持边缘人工智能功能，通过有限状态机 (FSM) 实现可配置的运动检测，并通过机器学习核心 (MLC) 实现情境感知，为个人电子产品、物联网、运动和可穿戴设备以及无人机应用提供智能功能。
为了优化功耗，LSM6DSK320X 集成了自适应自配置 (ASC) 功能，可根据检测到的运动模式或 MLC 事件实时自动调整传感器设置，无需主机干预。此外，该数字子系统还集成了 ST 的低功耗传感器融合 (SFLP) 技术，用于空间定位。
此外，专用的高g加速度传感器具有独立的通道和专用的滤波功能，可确保可靠的冲击检测，使该设备非常适合运动、脑震荡检测、冲击检测、紧急呼叫和无人机应用。
The **LSM6DSK320X** is a compact, low-power, high-performance inertial measurement unit (IMU) integrating a 3-axis digital low-g accelerometer with selectable full-scale ranges up to ±16 g, a 3-axis digital high-g accelerometer with a full-scale range up to ±320 g, and a 3-axis digital gyroscope. Its advanced architecture processes acceleration and angular-rate data through four independent channels: the User Interface (UI), Optical Image Stabilization (OIS), Electronic Image Stabilization (EIS), and the high-g acceleration channel.

Designed for demanding motion-sensing applications, the LSM6DSK320X enables high-intensity motion tracking, impact and concussion monitoring, as well as other highly dynamic use cases.

The device features a new gyroscope architecture with a fully differential readout chain and enhanced mechanical symmetry, providing excellent immunity to vibration and external disturbances. It also supports edge AI capabilities, including configurable motion detection through a Finite State Machine (FSM) and context awareness through a Machine Learning Core (MLC), enabling intelligent functions for personal electronics, IoT devices, sports and wearable applications, and drones.

To optimize power consumption, the LSM6DSK320X integrates an Adaptive Self-Configuration (ASC) function that can automatically adjust sensor settings in real time according to detected motion patterns or MLC events, without host intervention. In addition, the digital subsystem incorporates ST’s Sensor Fusion Low Power (SFLP) technology for spatial orientation and positioning.

Furthermore, the dedicated high-g accelerometer features an independent signal path and dedicated filtering functions to ensure reliable impact detection. These capabilities make the LSM6DSK320X particularly suitable for sports applications, concussion detection, impact detection, emergency-call systems, and drones.


- **STM32C5_LSM6DSK320X_Project2**:STM32C5开发LSM6DSK320X(2)----中断获取陀螺仪数据
- **CSDN Blog**:

本章节介绍基于 STM32C542 的 LSM6DSK320X 陀螺仪中断采集功能。通过配置 INT1 数据就绪中断，当陀螺仪产生新数据时触发 MCU 中断，读取三轴角速度数据并进行单位转换，实现高实时性的运动数据采集。 
This section introduces the **LSM6DSK320X gyroscope interrupt-based data acquisition** using the **STM32C542**. By configuring the **INT1 data-ready interrupt**, an MCU interrupt is triggered whenever new gyroscope data is available. The three-axis angular velocity data is then read and converted into physical units, enabling real-time motion data acquisition with low latency.


- **STM32C5_LSM6DSK320X_Project3**:STM32C5开发LSM6DSK320X(3)----配置单击与双击检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现单击（Single Tap）和双击（Double Tap）检测。LSM6DSK320X 内置硬件 Tap 检测功能，可通过低 G 加速度计对敲击动作进行识别，并支持将单击、双击事件映射到 INT1/INT2 中断引脚。 

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **Single Tap** and **Double Tap** detection. The LSM6DSK320X features built-in hardware tap detection, which uses the low-g accelerometer to recognize tapping events and supports routing single-tap and double-tap events to the **INT1/INT2 interrupt pins**.

- **STM32C5_LSM6DSK320X_Project4**:STM32C5开发LSM6DSK320X(4)----高级计步器、步数检测与步数计数 
- **CSDN Blog**:
本章将在前面外部中断配置的基础上，使能计步器并设置 Debounce 参数，将 Step Detector 事件映射至 INT1；当检测到有效步数后，由 STM32C542 响应外部中断并读取芯片内部 Step Counter，实现步数检测与实时计数。  
Based on the external interrupt configuration introduced earlier, this chapter enables the **pedometer** function and configures the **debounce parameters**, then routes the **Step Detector** event to **INT1**. When a valid step is detected, the **STM32C542** responds to the external interrupt and reads the internal **Step Counter** of the LSM6DSK320X, enabling real-time step detection and counting.

- **STM32C5_LSM6DSK320X_Project5**:STM32C5开发LSM6DSK320X(5)----显著运动检测   
- **CSDN Blog**:
本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现 Wake-up 运动检测。LSM6DSK320X 内置可配置的 Wake-up 标准中断，可利用 Low-G 加速度计对设备的动态运动进行检测，并在运动幅度超过设定阈值后通过 INT1/INT2 输出中断信号。  

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **Wake-up motion detection**. The LSM6DSK320X integrates a configurable **Wake-up interrupt** function that uses the **low-g accelerometer** to detect device motion. When the detected acceleration exceeds the configured threshold, an interrupt signal can be generated through the **INT1/INT2 pins**.


- **STM32C5_LSM6DSK320X_Project6**:STM32C5开发LSM6DSK320X(6)----倾斜度检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现 Tilt 倾斜检测。程序使能并初始化芯片内部的倾斜检测算法，配置嵌入式功能中断锁存，并将 Tilt 事件映射到 INT1。当设备姿态变化满足内部算法判定条件时，MCU 读取事件状态并通过串口输出检测结果。该功能用于检测倾斜变化事件，不直接输出具体的倾斜角度。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **Tilt detection**. The program enables and initializes the embedded tilt algorithm, configures latched embedded-function interrupts, and routes Tilt events to **INT1**. When an orientation change meets the algorithm’s detection criteria, the MCU reads the event status and reports the result through UART. This function detects tilt events without directly providing numerical tilt angles.


- **STM32C5_LSM6DSK320X_Project7**:STM32C5开发LSM6DSK320X(7)----自由落体检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现自由落体检测。程序配置加速度阈值和持续时间，当三轴加速度的绝对值同时低于设定阈值并满足时间条件时，芯片产生自由落体事件，并通过 INT1 通知 MCU。主循环读取事件状态，通过串口输出检测结果。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **Free-fall detection**. The program configures an acceleration threshold and a duration requirement. When the absolute acceleration on all three axes remains below the threshold for the required duration, the sensor generates a free-fall event and notifies the MCU through **INT1**. The main loop reads the event status and reports the result through UART.


- **STM32C5_LSM6DSK320X_Project8**:STM32C5开发LSM6DSK320X(8)----6D方向检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现 6D 方向检测。程序关闭 4D 模式，设置方向识别阈值，并将 6D 事件映射到 INT1。当设备姿态满足方向判定条件时，MCU 读取方向状态，通过串口输出 X+、X−、Y+、Y−、Z+ 或 Z−，实现六种基本方向的识别。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **6D orientation detection**. The program disables 4D mode, configures the orientation threshold, and routes 6D events to **INT1**. When the device orientation meets the detection criteria, the MCU reads the direction flags and reports **X+, X−, Y+, Y−, Z+, or Z−** through UART.


- **STM32C5_LSM6DSK320X_Project9**:STM32C5开发LSM6DSK320X(9)----活动与静止检测
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现活动与静止检测。程序配置运动阈值和静止等待时间，根据经过滤波的加速度数据判断设备状态，并将活动与静止状态变化事件映射到 INT1。MCU 读取状态后，通过串口输出 Activity 或 Inactivity 提示。本例在静止状态下保持加速度计和陀螺仪的工作配置不变。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **Activity and Inactivity detection**. The program configures motion thresholds and an inactivity time window, determines the device state from filtered acceleration data, and routes state-change events to **INT1**. The MCU reads the status and reports **Activity** or **Inactivity** through UART. In this example, the accelerometer and gyroscope configurations remain unchanged during inactivity.


- **STM32C5_LSM6DSK320X_Project10**:STM32C5开发LSM6DSK320X(10)----FIFO水位中断与六轴数据读取
- **CSDN Blog**:

本章介绍如何使用 STM32C542CCT6 驱动 LSM6DSK320X 实现 FIFO 缓存与水位中断读取。程序将 Low-G 加速度计和陀螺仪数据分别以 60 Hz 的速率写入 FIFO，配置连续模式，并设置 128 条记录的水位阈值。达到水位后，INT1 通知 MCU 批量读取数据，根据 TAG 区分加速度和角速度，完成单位换算，并通过串口输出本批次的样本数量及三轴平均值。

This chapter introduces how to use the **STM32C542CCT6** to drive the **LSM6DSK320X** for **FIFO buffering and watermark interrupt-based acquisition**. Low-g accelerometer and gyroscope data are each written to the FIFO at **60 Hz**. The FIFO operates in continuous mode with a watermark of **128 records**. When the watermark is reached, **INT1** notifies the MCU to read the buffered data in a batch. The program identifies acceleration and angular velocity records by their **TAG**, converts the measurements into physical units, and outputs the sample counts and three-axis averages through UART.
