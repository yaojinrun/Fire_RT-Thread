# 光环境传感器

## 支持平台

| 验证平台                 | 硬件连接     | 环境搭建    |
| ------------------------ | ---- | ---- |
| 野火 M3 M4 M7 开发板     | [硬件连接]()     |      |
| 正点原子 M3 M4 M7 开发板 | [硬件连接]()      |      |
| QEMU 模拟器              |      | [环境搭建]()      |
| Keil MDK 模拟器          |      | [环境搭建]()     |

## 硬件要求

要求硬件上：

* 至少有一路带ADC输入功能的GPIO， 能够用来连接 光环境传感器
* 有一路串口用来做 msh shell 终端

## 软件要求

BSP 中已经实现如下驱动：

* 串口驱动
* ADC 驱动

## 准备工作

## 步骤一 编写例程

### 创建例程文件

创建 rgb_breathe.c 文件，并在文件中加入引用头文件

```{.c}
#include <rtthread.h>
#include <rtdevice.h>
```

### 编写示例函数

编写线程入口函数 lsens_get_val()，首先定义每次采样的次数，通道数以及周期：

```{.c}
#define LSENS_READ_TIMES	10		/* 定义光传感器的采样次数，然后取平均值 */
#define LSENS_CHANNEL	    4		/* 实际采样所使用的通道 */
#define LSENS_GET_PERIOD	500		/* 定义采样周期 */
```

通过多次采样光传感器上的电压值来减小误差，然后将采样的数据转换成0-100的光强数值，0代表最暗，100代表最强。如果数据有变化，就把光强信息从串口打印出来。

```{.c}
void lsens_get_val(void* parameter)
{
	rt_uint32_t temp = 0;
	rt_uint32_t Lsens_val = 0;
	
	while(1)
	{
		rt_uint8_t i;
		rt_uint16_t val=0;
		
		for(i = 0;i < LSENS_READ_TIMES;i++)
		{
            temp = rt_adc_read(LSENS_CHANNEL);
			val += temp;
			rt_thread_delay(1);		/* #define RT_TICK_PER_SECOND 1000 */
		}
		temp = (rt_uint32_t)(val/LSENS_READ_TIMES);	/* 取平均值 */
		
		if(temp>4000) temp=4000;
		temp = temp/40;
		if(temp != Lsens_val)
		{
			Lsens_val = temp;
			rt_kprintf("Lsens Vaule is:%d \n", Lsens_val);  
		}
		rt_thread_delay(LSENS_GET_PERIOD-LSENS_READ_TIMES);
	}
	
}
```

编写用户线程创建函数，并指定函数 lsens_get_val()为用户线程入口函数：

```{.c}
int lsens_test(void)
{
    rt_thread_t tid;
			
	tid = rt_thread_create("Lsens",
		lsens_get_val, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	
	return 0;
}
```

### 将示例函数导出到 msh 命令列表

通过如下的方式可以将示例函数 lsens_test 导出到 msh 命令列表中：

```{.c}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(lsens_test, get the light sens vaule.);
```

## 步骤二 运行例程

### 在 main 函数中运行

一种方式是在 main 程序中调用 lsens_test() 函数

```{.c}
int main(int argc, char **argv)
{
    lsens_test();
}
```

### 在 msh shell 中运行

另一种方式是通过 [msh shell](shell.md) 运行，在步骤一中已经将 lsens_testlsens_test 命令导出到了 msh 命令列表中，因此系统运行起来后，在 msh 命令行下输入 lsens_test 命令即可让例程运行。

```{.c}
msh> lsens_test
```

## 预期结果

当环境光强发生变化时，通过串口打印出光强信息。

```{.c}
msh />lsens_test

......
```

## 引用参考

* 源码 [test_adc.c]()
* [如何导出 msh 命令]()
* [系统时钟 OS Tick]()
* [如何进行 menuconfig 系统配置]()
