# 串口通讯

## 硬件连接

### 串口连接

野火F429开的底板的上已经集成了USB转串口电路，如图1：

![UART1电路图](.\figures\uart.png)

用跳帽连接：

J11: PA10（USART1_RX） → CH340G的TXD

J12：PA9（USART1_TX） → CH340G的RXD


### LED全彩灯连接

野火F429开的底板的上全彩LED灯电路如下：

![LED全彩灯](.\figures\led_rbg.png)

我们需要用跳帽把J35连接起来，将LED全彩灯的共阳极连接到3.3V电源上。

## 程序开发

### 初始化配置

我们需要通过串口 uart1 输入特定的字符来实现对全彩灯颜色的变化，但是由于bsp包中默认使用了uart1 来作为shell/msh的控制台，所以在我们使用之前需要在 rtconfig.h 中关闭控制台：

```{.c}
#define RT_CONSOLE_DEVICE_NAME 0
```

为了增加代码可读性，我们增加以下宏定义：

```{.c}
#define LED_R     87
#define LED_G     88
#define LED_B     89

//全彩灯共阳连接到电源，低电平为开，高电平为关
#define LED_ON(x)         rt_pin_write(x ,0)
#define LED_OFF(x)        rt_pin_write(x ,1)
```

我们对LED控制脚和串口1做初始化配置：

```{.c}
int rt_rgb_init()
{
    rt_device_t uart_device;

    //LED控制脚设置为输出模式
    rt_pin_mode(LED_R, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(LED_G, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(LED_B, PIN_MODE_OUTPUT_OD);
    //初始化默认关闭LED灯，全彩灯共阳连接，高电平为关
    rt_pin_write(LED_R ,1);
    rt_pin_write(LED_G ,1);
    rt_pin_write(LED_B ,1);

    uart_device = rt_device_find("uart1");  //通过名称uart1查找设备
    rt_device_set_rx_indicate(uart_device, uart_rx_ind);    //设置串口接收回调函数
    rt_device_open(uart_device, (RT_DEVICE_OFLAG_RDWR |  RT_DEVICE_FLAG_INT_RX));    //将串口设置为中断接收模式

    return 0;
}
INIT_APP_EXPORT(rt_rgb_init);   //使用组件初始化功能，自动加入初始化列表，不需要再手动调用
```

### 编写串口接收回调函数

由于串口接收回调函数运行于中断上下文中，所以该回调函数应该尽量简短，不要调用系统延时或让线程挂起的函数。
此次试验只需要通过判断串口命令来控制LED全彩灯的颜色变化，完全可以在中断中完成。

```{.c}
static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    static rt_uint8_t pos = 0;

    //中断接收模式下一次只能处理一个字符
    if(size == 1)
    {
        rt_device_read(dev, 0, &uart_rx_buffer[pos], size);
        //命令格式“LED R/B/G”
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
            }
            else if(uart_rx_buffer[pos] == 'G')
            {
                LED_OFF(LED_R);
                LED_ON(LED_G);
                LED_OFF(LED_B);
            }
            else if(uart_rx_buffer[pos] == 'B')
            {
                LED_OFF(LED_R);
                LED_OFF(LED_G);
                LED_ON(LED_B);
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

### 运行例程

编译后将程序烧录到野火F429开发板中运行，即可通过以下命名即可实现对LED全彩灯的颜色的切换控制：

| 命令  | 说明                  |
| ----- | --------------------- |
| LED R | LED全彩灯点亮为红色灯 |
| LED B | LED全彩灯点亮为蓝色灯 |
| LED G | LED全彩灯点亮为红色灯 |
