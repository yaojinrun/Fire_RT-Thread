# RTC闹钟

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求

要求硬件上：

* 至少有一路 GPIO 能够用来输出控制 蜂鸣器
* 至少有一路 GPIO 作为按键输入检测端口
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* PIN 驱动
* RTC驱动

## 准备工作

### 配置 LED 灯对应的 PIN 号

由于需要使用RT-Thread 的RTC和ALRAM框架，需要打开rtc和alarm的宏定义。宏定义的配置有两种方式:

* 手动方式

直接在 rtconfig.h 文件中加入以下宏定义进行配置：

```{.c}
#define RT_USING_RTC
#define RT_USING_ALARM
```

* menuconfig 配置方式

通过使用 env 工具的 menuconfig 配置方式，会自动在 rtconfig.h 配置文件中生成如下的宏定义：

```{.c}
#define RT_USING_RTC
#define RT_USING_ALARM
```

## 步骤一 编写例程

### 创建例程文件

创建 rtc_alarm.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

编写初始化函数 alarm_init() ，首先设置 按键输入和蜂鸣器控制脚工作模式，然后又创建按键检测线程。

```{.c}
int alarm_init(void)
{
    rt_thread_t tid;
    
    /* 初始化按键输入 */
    rt_pin_mode(KEY1, PIN_MODE_INPUT); 
    
    /* 将蜂鸣器控制脚设置为输出模式 */
    rt_pin_mode(BEEP, PIN_MODE_OUTPUT); 
    /* 蜂鸣器默认不响 */
    BEEP_OFF(BEEP);
    
    rt_memset(&ALARM, NULL, sizeof(ALARM));
    
    
	tid = rt_thread_create("alarm_test",
		alarm_test_entry, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
    
    /* 初始化闹钟服务 */
    rt_alarm_system_init();
    
	return 0;
}
INIT_APP_EXPORT(alarm_init);
```

编写按键检测线程入口函数alarm_test_entry()，循环检测按键动作，当按键按下时关闭蜂鸣器。

```{.c}
void alarm_test_entry(void* parameter)
{

	while(1)
	{
        if(rt_pin_read(KEY1)==1)
        {
            BEEP_OFF(BEEP);
        }
        rt_thread_delay(20);
	}
	
}
```
编写闹钟超时回调函数alarm_callback_entry()，在回调函数中打开蜂鸣器，同时将闹钟的信息从串口打印出来。

```{.c}
void alarm_callback_entry(rt_alarm_t alarm, time_t timestamp)
{
    /* 打开蜂鸣器 */
    BEEP_ON(BEEP);
	rt_kprintf("Time out!\n");
}
```
编写示例函数 alarm_add()，设定闹钟时间，并指定闹钟超时回调函数为alarm_callback_entry()。

```{.c}
void alarm_add(int argc, char **argv)
{
    struct rt_alarm_setup _alarm;
	
	if(argc > 1)
	{
        _alarm.flag = RT_ALARM_ONESHOT;    /* 单次定时 */
        rt_memset(&_alarm.wktime, RT_ALARM_TM_NOW, sizeof(struct tm));
        _alarm.wktime.tm_hour  = atol(argv[1]);
        if(argc > 2)
        {
            _alarm.wktime.tm_min = atol(argv[2]);
            _alarm.wktime.tm_sec = 0;
        }
        if(ALARM[0] == NULL)
        {
            ALARM[0] = rt_alarm_create(alarm_callback_entry, &_alarm);
        }
        else 
        {
            rt_alarm_control(ALARM[0], RT_ALARM_CTRL_MODIFY, &_alarm);
        }
		rt_alarm_start(ALARM[0]);
        
	}
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 alarm_add 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(alarm_add, e.g: alarm_add 12 15);	/* 12时15分 */
```

## 步骤二 运行例程

### 在 main 函数中运行

### 在 msh shell 中运行

在步骤一中已经将 alarm_add 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 alarm_add 命令 + 时间参数 即可让例程运行。

```{.c}
msh> alarm_add 12 15
```

## 预期结果

定时时间到后蜂鸣器响，并打印闹钟超时信息。

```{.c}
msh> alarm_add 12 15
msh> ALARM A!
Time out!
```

## 注意事项

* PIN 号和 GPIO 号是不相同的，需要注意区分。
* QEMU 和 Keil MDK 模拟器平台因为没有实际的 蜂鸣器 硬件，因此只能通过串口日志信息来判断例程的运行情况。

## 引用参考

* 源码 [rtc_alarm.c]()
* [如何使用 pin 设备](../../../../topics/driver/pin/user-guide.md)
* [如何导出 msh 命令]()
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
