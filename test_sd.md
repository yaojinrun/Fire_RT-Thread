# SD卡读写

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求

要求硬件上：

* 有SD卡接口， 并插入SD卡
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* SD卡驱动

## 步骤一 编写例程

### 创建例程文件

创建 test_sd.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "finsh.h"
```

### 编写示例函数

编写缓冲区填充数据函数 fill_buffer()，将发送缓冲区buffer_multiblock_tx[MULTI_BUFFER_SIZE]中的数据初始化，为 sd 卡的数据写入做准备。

```{.c}
void fill_buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
    uint16_t index = 0;

    for (index = 0; index < BufferLength; index++)
    {
        pBuffer[index] = index + Offset;
    }
}
```

编写 sd 卡测试函数 sd_test()，先通过名称 "sd0" 查找到 sd 卡设备，打开 sd 卡设备后，将填充了数据的发送缓冲区数据写入sd中，然后再从sd卡中将数据读出，比较发送缓冲区和接收缓冲区的数据是否一致，将结果信息从串口打印出来。

```{.c}
void sd_test(int argc, char **argv)
{  
	rt_device_t sd_device = RT_NULL;
	uint8_t size;
    
    /* 查找SD设备 */
    sd_device = rt_device_find("sd0");
    if(sd_device != NULL)
    {
        /* 打开设备 */
        rt_device_open(sd_device, RT_DEVICE_OFLAG_RDWR);
        
        /* 填充发送缓冲区，为写操作做准备 */
        fill_buffer(buffer_multiblock_tx, MULTI_BUFFER_SIZE, 0x320F);
        
        /* 把发送缓冲区的数据写入SD卡中 */
        size = rt_device_write(sd_device, 0, buffer_multiblock_tx, NUMBER_OF_BLOCKS);
        if (size != NUMBER_OF_BLOCKS) return;
        
        /* 从SD卡中读出数据，并保存在接收缓冲区中 */
        size = rt_device_read(sd_device, 0, buffer_multiblock_rx, NUMBER_OF_BLOCKS);
        if (size != NUMBER_OF_BLOCKS) return;
        
        /* 比较发送缓冲区和接收缓冲区的内容是否完全一致 */
        if (rt_memcmp(buffer_multiblock_tx, buffer_multiblock_rx, MULTI_BUFFER_SIZE) == 0)
        {
            rt_kprintf("Block test OK!\n");
        }
        else
        {
            rt_kprintf("Block test Fail!\n");
        }
    }
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 sd_test 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(sd_test, sd card test sample);
```

## 步骤二 运行例程

### 在 main 函数中运行

一种方式是在 main 程序中调用 sd_test() 函数

```{.c}
int main(int argc, char **argv)
{
    sd_test();
}
```

### 在 msh shell 中运行

另一种方式是通过 [msh shell](shell.md) 运行，在步骤一中已经将 led_blink 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 sd_test 命令即可让例程运行。

```{.c}
msh />sd_test
```

## 预期结果

串口打印出 SD 卡测试结果信息。

```{.c}
msh />sd_test
Block test OK!
```

## 注意事项

* 野火开发板上的 SD 卡和 WiFi 模块共用 SDIO 接口，使用 SD 卡前应禁用 WIFI 模块。
* 本文中的sd卡测试程序会对SD卡进行 非文件系统 方式读写，会破坏SD卡的文件系统，实验前务必备份SD卡内的原文件！

## 引用参考

* 源码 [test_sd.c ]()
* [如何导出 msh 命令]()
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
