# 点亮第一盏灯 LED

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求

要求硬件上：

* 至少有两路 GPIO， 能够用来接 LED 灯
* 至少有一路 GPIO 作为输入端接按键
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* PIN 驱动
* 看门狗驱动

## 准备工作

### 配置 LED 灯对应的 PIN 号

根据硬件连接情况配置与 按键和 LED 灯连接的 PIN 号。

在创建的例程文件中加入以下宏定义进行配置：

```{.c}
#define KEY1	 40

#define LED_R     87
#define LED_G     88
```

## 步骤一 编写例程

### 创建例程文件

创建 test_iwdg.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

编写用户线程入口函数 iwdg_test_entry()，首先设置 PIN 脚模式

```{.c}
    /* 初始化按键引脚为输入模式 */
    rt_pin_mode(KEY1, PIN_MODE_INPUT);   
    /* 将LED控制脚设置为输出模式 */
    rt_pin_mode(LED_R, PIN_MODE_OUTPUT_OD); 
    rt_pin_mode(LED_G, PIN_MODE_OUTPUT_OD);
```

PIN 脚模式设置好以后，默认点亮红灯，关闭绿灯。然后通过while(1)不断检测按键输入，当按键按下时点亮绿灯并喂狗。那么当没有在设定时间内按键喂狗，就会发生独立看门狗复位，能够看见红灯闪烁。

```{.c}
    /* 初始化默认关闭LED灯 */
	LED_ON(LED_R);
	LED_OFF(LED_G);
	while(1)
	{
        if(rt_pin_read(KEY1)==1)
        {
        	/* 喂狗并点亮绿灯关闭红灯 */
            rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
            LED_OFF(LED_R);
            LED_ON(LED_G);
        }
	}
```

编写看门狗初始化函数，设置看门狗溢出时间为1000ms，并创建用户线程。
```{.c}

int iwdg_test(void)
{
    rt_thread_t tid;
    rt_err_t result = RT_EOK;
    rt_uint32_t timeout = 1000;     /* 超时时间为1000ms*/
    
    wdg_dev = rt_device_find("iwdg");
    if (!wdg_dev)
    {
        return -RT_EIO;
    }
    wdg_dev->open(wdg_dev, NULL);
    /* 设定看门狗溢出时间 */
    result = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (result != RT_EOK)
    {
        return -RT_EIO;
    }
    /* 创建用户线程，并指定iwdg_test_entry */
	tid = rt_thread_create("iwdg_test",
		iwdg_test_entry, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
    
    /* 启动看门狗 */
	rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, NULL);
    
	return 0;
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 iwdg_test 导出到 组件初始化 列表中后自动运行：

```{.c}
/* 导出到 组件初始化 命令列表中 */
INIT_APP_EXPORT(iwdg_test);
```

## 预期结果

当没有及时按KEY1键喂狗时，系统被复位，LED 红灯闪烁；当在看门狗设定溢出时间前按KEY1键成功喂狗时，LED绿灯点亮。


## 注意事项

* PIN 号和 GPIO 号是不相同的，需要注意区分。
* QEMU 和 Keil MDK 模拟器平台因为没有实际的 LED 灯硬件，因此只能通过串口日志信息来判断例程的运行情况。

## 引用参考

* 源码 [test_iwdg.c]()
* [如何使用 pin 设备](../../../../topics/driver/pin/user-guide.md)
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
