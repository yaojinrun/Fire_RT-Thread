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

* 至少有三路带PWM输出功能的GPIO， 能够用来连接 LED 全彩灯
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* PIN 驱动
* PWM 驱动

## 准备工作

### 配置 LED 灯对应的 PIN 号

根据硬件连接情况配置与 LED 灯连接的 PIN 号，有两种配置方式:

* 手动方式

直接在新创建的例程文件中加入以下宏定义进行配置：

```{.c}
#define LED_PIN     xxx
```

* menuconfig 配置方式

通过使用 env 工具的 menuconfig 配置方式，会自动在 rtconfig.h 配置文件中生成如下的宏定义：

```{.c}
#define LED_PIN     xxx
```

## 步骤一 编写例程

### 创建例程文件

创建 rgb_pwm.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

编写线程入口函数 rt_pwm_rgb_entry()，首先使能pwm连接到全彩灯引脚的三个通道：

```{.c}
    rt_pwm_enable(CH_R);
    rt_pwm_enable(CH_G);
    rt_pwm_enable(CH_B);
```

P使能pwm通道后，就可以使用 rt_pwm_set() 接口来设置pwm波的频率和占空比来控制对应灯的亮度，对不同颜色的灯亮度的变化来混合成不同的颜色。通过调整延时的长度来控制pwm的参数的修改频率，同时将灯的状态信息从串口打印出来。

```{.c}
     while(1)
    {
        random = rand()%255;    /*产生一个伪随机数*/
        rt_pwm_set(CH_R, PERIOD, random);   /*用生成的伪随机数来设定PWM波的占空比*/
        random = rand()%255;
        rt_pwm_set(CH_G, PERIOD, random);
        random = rand()%255;
        rt_pwm_set(CH_B, PERIOD, random);
        
        rt_thread_delay(500);
    }
```

这里的 `PERIOD` 被设定成了常数255， 即将 pwm波的一个周期分为了256份，也就是将led灯的亮度分为了256级。

编写用户线程创建函数，并指定函数 rt_pwm_rgb_entry()为用户线程入口函数：

```{.c}
int rt_pwm_rgb_init()
{
    rt_device_t pwm_dev;
    rt_thread_t tid;
	
	pwm_dev = rt_device_find("pwm");	/*查找名为pwm的设备*/
	if(NULL != pwm_dev)
	{
        tid = rt_thread_create("pwm_led",
            rt_pwm_rgb_entry, RT_NULL,
            512, RT_THREAD_PRIORITY_MAX/3, 20);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
	}

    return 0;
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 rt_pwm_rgb_init 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(rt_pwm_rgb_init, rgb by pwm.);
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

另一种方式是通过 [msh shell](shell.md) 运行，在步骤一中已经将 rt_pwm_rgb_init 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 rt_pwm_rgb_init 命令即可让例程运行。

```{.c}
msh> rt_pwm_rgb_init
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
drv_pwm.c set channel: 1, period: 255, pulse: 238
drv_pwm.c control cmd: 130. 
PWM_CMD_SET
drv_pwm.c set channel: 2, period: 255, pulse: 222
drv_pwm.c control cmd: 130. 
PWM_CMD_SET
drv_pwm.c set channel: 3, period: 255, pulse: 184
......
```

## 注意事项

* 全彩LED灯的三个引脚的pwm频率相同，占空比不同，所以连接到同一定时器的不同通道即可。
* PIN 号和 GPIO 号是不相同的，需要注意区分。
* QEMU 和 Keil MDK 模拟器平台因为没有实际的 LED 灯硬件，因此只能通过串口日志信息来判断例程的运行情况。

## 引用参考

* 源码 [rgb_pwm.c]()
* [如何使用 pin 设备](../../../../topics/driver/pin/user-guide.md)
* [如何导出 msh 命令]()
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
