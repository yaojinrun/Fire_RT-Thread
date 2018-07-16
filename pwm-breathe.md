# PWM控制全彩灯颜色

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求

要求硬件上：

* 至少有一路带PWM输出功能的GPIO， 能够用来连接 LED 灯
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* PIN 驱动
* PWM 驱动

## 准备工作

## 步骤一 编写例程

### 创建例程文件

创建 rgb_breathe.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

编写线程入口函数 rt_pwm_breathe_entry()，首先使能pwm连接到全彩灯引脚的三个通道：

```{.c}
    rt_pwm_enable(CH_R);
    rt_pwm_enable(CH_G);
    rt_pwm_enable(CH_B);
```

使能pwm通道后，就可以使用 rt_pwm_set() 接口来设置pwm波的频率和占空比来控制对应灯的亮度，对不同颜色的灯亮度的变化来混合成不同的颜色。通过调整延时的长度来控制pwm的参数的修改频率，同时将灯的状态信息从串口打印出来。

```{.c}
    while(1)
    {
        rt_pwm_set(CH_R, PERIOD, indexWave[i]);
        rt_pwm_set(CH_G, PERIOD, indexWave[i]);
        rt_pwm_set(CH_B, PERIOD, [i]);
        
        if(i >= POINT_NUM-1) i = 0;
        else i++;
        
        /* 设置整个呼吸过程为3秒左右即可达到很好的效果 */
        rt_thread_delay(3000/POINT_NUM);
    }
```

这里的 `PERIOD` 被设定成了常数512， indexWave[]数字为工程目录下的python脚本index_wave.py生产。

```{.c}
uint16_t indexWave[] = {
1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 
	6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 
	12, 13, 14, 15, 17, 18, 19, 20, 22, 
	23, 25, 27, 29, 31, 33, 36, 38, 41, 
	44, 47, 51, 54, 58, 63, 67, 72, 77, 
	83, 89, 95, 102, 110, 117, 126, 135,
	145, 156, 167, 179, 192, 206, 221, 237, 
	254, 272, 292, 313, 336, 361, 387, 415, 
	445, 477, 512, 512, 477, 445, 415, 387, 
	361, 336, 313, 292, 272, 254, 237, 221, 
	206, 192, 179, 167, 156, 145, 135, 126, 
	117, 110, 102, 95, 89, 83, 77, 72, 67, 63,
	58, 54, 51, 47, 44, 41, 38, 36, 33, 31, 29,
	27, 25, 23, 22, 20, 19, 18, 17, 15, 14, 13,
	12, 12, 11, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5,
	5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 1, 1, 1, 1, 1
};

```

编写用户线程创建函数，并指定函数 rt_breathe_init()为用户线程入口函数：

```{.c}
int rt_breathe_init()
{
    rt_device_t pwm_dev;
    rt_thread_t tid;
	
	pwm_dev = rt_device_find("pwm");
	if(NULL != pwm_dev)
	{
        tid = rt_thread_create("pwm_led",
            rt_pwm_breathe_entry, RT_NULL,
            512, RT_THREAD_PRIORITY_MAX/3, 20);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
	}

    return 0;
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 rt_breathe_init 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(rt_breathe_init, rgb breathe by pwm.);
```

## 步骤二 运行例程

### 在 main 函数中运行

一种方式是在 main 程序中调用 rt_pwm_rgb_init() 函数

```{.c}
int main(int argc, char **argv)
{
    rt_pwm_rgb_init();
}
```

### 在 msh shell 中运行

另一种方式是通过 [msh shell](shell.md) 运行，在步骤一中已经将 rt_breathe_init 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 rt_breathe_init 命令即可让例程运行。

```{.c}
msh> rt_breathe_init
```

## 预期结果

LED 全彩灯能够定时变换不同颜色，同时串口打印出 PWM 的参数信息。

```{.c}
msh />rt_pwm_rgb_init
drv_pwm.c control cmd: 128. 
PWM_CMD_ENABLE
drv_pwm.c control cmd: 128. 
PWM_CMD_ENABLE
drv_pwm.c control cmd: 128. 
PWM_CMD_ENABLE
drv_pwm.c control cmd: 130. 
PWM_CMD_SET
drv_pwm.c set channel: 1, period: 512, pulse: 1
drv_pwm.c control cmd: 130. 
PWM_CMD_SET
drv_pwm.c set channel: 2, period: 512, pulse: 1
drv_pwm.c control cmd: 130. 
PWM_CMD_SET
drv_pwm.c set channel: 3, period: 512, pulse: 1
......
```

## 注意事项

* PIN 号和 GPIO 号是不相同的，需要注意区分。
* QEMU 和 Keil MDK 模拟器平台因为没有实际的 LED 灯硬件，因此只能通过串口日志信息来判断例程的运行情况。

## 引用参考

* 源码 [rgb_breathe.c]()
* [如何使用 pin 设备](../../../../topics/driver/pin/user-guide.md)
* [如何导出 msh 命令]()
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
