# 串口的使用

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求
要求硬件上：

* 至少有三路 GPIO， 能够用来接 LED 全彩灯
* 至少有1路串口，用来做作为串口命令输入端口控制全彩灯

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* PIN 驱动

## 准备工作

根据硬件连接情况配置与 LED 全彩灯连接的 PIN 号，有两种配置方式:

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

创建 uart.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

由于串口1分配给shell/msh控制台，所以选用串口2来输入控制命令。
编写 rt_uart_init() 函数，设置 PIN 脚模式和串口工作模式。

```{.c}
int uart_sample(void)
{
    rt_device_t uart_device;

    /*LED控制脚设置为输出模式*/
    rt_pin_mode(LED_R, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(LED_G, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(LED_B, PIN_MODE_OUTPUT_OD);
    /*初始化默认关闭LED灯，全彩灯共阳连接，高电平为关*/
    rt_pin_write(LED_R ,1);
    rt_pin_write(LED_G ,1);
    rt_pin_write(LED_B ,1);

    uart_device = rt_device_find("uart2");  /*通过名称uart2查找设备*/
    rt_device_set_rx_indicate(uart_device, uart_rx_ind);    /*设置串口接收回调函数*/
    rt_device_open(uart_device, (RT_DEVICE_OFLAG_RDWR |  RT_DEVICE_FLAG_INT_RX));    /*将串口设置为中断接收模式*/

    return 0;
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 uart_sample() 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(uart_sample, uart sample);
```

### 编写串口接收回调函数

串口工作模式设置好以后，就可以在串口接收回调函数中来编写命令控制LED全彩灯点亮不同的颜色，同时将灯的状态信息从串口打印出来。

```{.c}
static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    static rt_uint8_t pos = 0;

    /*中断接收模式下一次只能处理一个字符*/
    if(size == 1)
    {
        rt_device_read(dev, 0, &uart_rx_buffer[pos], size);
        /*命令格式“LED R/B/G”*/
        if((pos == 0)&&(uart_rx_buffer[pos] == 'L')) pos++;
        else if((pos == 1)&&(uart_rx_buffer[pos] == 'E')) pos++;
        else if((pos == 2)&&(uart_rx_buffer[pos] == 'D')) pos++;
        else if((pos == 3)&&(uart_rx_buffer[pos] == ' ')) pos++;
        else if (pos == 4)
        {
            if(uart_rx_buffer[pos] == 'R')
            {
                LED_ON(LED_R);
                LED_OFF(LED_G);
                LED_OFF(LED_B);
                rt_kprintf("Red led!\n");
            }
            else if(uart_rx_buffer[pos] == 'G')
            {
                LED_OFF(LED_R);
                LED_ON(LED_G);
                LED_OFF(LED_B);
                rt_kprintf("Green led!\n");
            }
            else if(uart_rx_buffer[pos] == 'B')
            {
                LED_OFF(LED_R);
                LED_OFF(LED_G);
                LED_ON(LED_B);
                rt_kprintf("Blue led!\n");
            }
            pos = 0;
        }
        else pos = 0;
    }
    else
        rt_kprintf("Please set the uart to interrupt RX mode!\n");

    return RT_EOK;
}
```


## 步骤二 运行例程

### 在 main 函数中运行

一种方式是在 main 程序中调用 uart_sample() 函数

```{.c}
int main(int argc, char **argv)
{
    uart_sample();
}
```

### 在 msh shell 中运行

另一种方式是通过 [msh shell](shell.md) 运行，在步骤一中已经将 uart_sample 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 uart_sample 命令即可让例程运行。

```{.c}
msh> uart_sample
```
当示例函数uart_sample运行后，就完成了串口和LED控制脚的初始化工作，然后我们就可通过串口2输入以下命令实现对LED全彩灯的颜色的切换控制：

| 命令  | 说明                  |
| ----- | --------------------- |
| LED R | LED全彩灯点亮为红色灯 |
| LED B | LED全彩灯点亮为蓝色灯 |
| LED G | LED全彩灯点亮为红色灯 |


## 预期结果

LED 全彩灯能够根据串口2输入的命令对全彩灯颜色切换，同时shell/msh打印出 LED 灯的状态信息。

## 注意事项

- PIN 号和 GPIO 号是不相同的，需要注意区分。
- 由于串口接收回调函数运行于中断上下文中，所以应该尽量简短，不要调用系统延时或让线程挂起的函数。
- 如果硬件电路只有一路串口时，我们可以关闭shell/msh控制台，让这路串口作为全彩灯命令输入端口。修改 rtconfig.h 中的配置：

```{.c}
#define RT_CONSOLE_DEVICE_NAME 	0
```


## 引用参考

* 源码 [uart.c]()
* [如何使用 pin 设备](../../../../topics/driver/pin/user-guide.md)
* [如何导出 msh 命令]()
