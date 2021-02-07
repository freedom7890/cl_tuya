# -
窗帘机器人-涂鸦版
This project is developed using Tuya SDK, which enables you to quickly develop branded apps connecting and controlling smart scenarios of many devices. For more information, please check Tuya Developer Website.

智能窗帘机器人-涂鸦版
==

功能：
----

1，红外遥控控制
2，手拉启动
3，遇阻停止
4，首次启动自动设置限位点
5，电量检测
6，涂鸦智能控制



硬件配置
---------

mcu stm32l010f4p6

wifi   涂鸦wbr3

红外接收 LF0038M

霍尔 ss460s 或 hs229 双极锁存低功耗霍尔传感器

电机驱动 DRV8837C



工作原理
-------

       涂鸦模组或红外接收 控制信号，mcu处理后控制电机转动。
       霍尔传感器检测位置及方向，实现行程限位，及手拉启动。
       mcu与涂鸦模块通过串口通讯，mcu使用低功耗模式，待机自动进入stop模式，
       可以由红外接收、涂鸦模块串口、霍尔模块等唤醒执行动作。
       配置adc检测电池电量。

安装使用：主机安装置于窗帘开启状态位置，开启电源，遥控或涂鸦app连接控制窗帘关闭，窗帘运行至关闭状态遇阻停止，程序自动设置关闭限位点，完成行程设置。后续就可以正常使用了。

红外遥控可实现，开启，关闭，停止 3种状态控制功能

涂鸦app实现：

开启，关闭，停止，继续 4种状态控制功能

开启关闭百分比显示及控制功能。

电量显示，手拉启动开关功能，定时功能，及与其他涂鸦设备联动。
