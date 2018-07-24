#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"

#define ADC_CHANNEL_MAX		(16) /* 0-15 */


#define __STM32_ADC(index, gpio, gpio_index) {index, GPIO##gpio##_CLK_ENABLE, GPIO##gpio, GPIO_PIN_##gpio_index}

static void GPIOA_CLK_ENABLE(void)
{
#ifdef __HAL_RCC_GPIOA_CLK_ENABLE
    __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
}
static void GPIOB_CLK_ENABLE(void)
{
#ifdef __HAL_RCC_GPIOB_CLK_ENABLE
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
}
static void GPIOC_CLK_ENABLE(void)
{
#ifdef __HAL_RCC_GPIOC_CLK_ENABLE
    __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
}

struct adc_channel_index
{
    int index;
    void (*rcc)(void);
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

static const struct adc_channel_index adc1_channels[] =
{
	__STM32_ADC(0, A, 0),
	__STM32_ADC(1, A, 1),
	__STM32_ADC(2, A, 2),
	__STM32_ADC(3, A, 3),
	__STM32_ADC(4, A, 4),
	__STM32_ADC(5, A, 5),
	__STM32_ADC(6, A, 6),
	__STM32_ADC(7, A, 7),
	__STM32_ADC(8, B, 0),
	__STM32_ADC(9, B, 1),
	__STM32_ADC(10, C, 0),
	__STM32_ADC(11, C, 1),
	__STM32_ADC(12, C, 2),
	__STM32_ADC(13, C, 3),
	__STM32_ADC(14, C, 4),
	__STM32_ADC(15, C, 5),
};


ADC_HandleTypeDef ADC1_Handler;	//ADC句柄

//初始化ADC
//ch: ADC_channels 
//通道值 0~16取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
void ADC_Init(ADC_HandleTypeDef ADCx_Handler)
{ 
    ADCx_Handler.Instance=ADCx_Handler.Instance;
    ADCx_Handler.Init.ClockPrescaler=ADC_CLOCK_SYNC_PCLK_DIV4;   //4分频，ADCCLK=PCLK2/4=90/4=22.5MHZ
    ADCx_Handler.Init.Resolution=ADC_RESOLUTION_12B;              //8位模式
    ADCx_Handler.Init.DataAlign=ADC_DATAALIGN_RIGHT;             //右对齐
    ADCx_Handler.Init.ScanConvMode=DISABLE;                      //非扫描模式
    ADCx_Handler.Init.EOCSelection=DISABLE;                      //关闭EOC中断
    ADCx_Handler.Init.ContinuousConvMode=DISABLE;                //关闭连续转换
    ADCx_Handler.Init.NbrOfConversion=1;                         //1个转换在规则序列中 也就是只转换规则序列1 
    ADCx_Handler.Init.DiscontinuousConvMode=DISABLE;             //禁止不连续采样模式
    ADCx_Handler.Init.NbrOfDiscConversion=0;                     //不连续采样通道数为0
    ADCx_Handler.Init.ExternalTrigConv=ADC_SOFTWARE_START;       //软件触发
    ADCx_Handler.Init.ExternalTrigConvEdge=ADC_EXTERNALTRIGCONVEDGE_NONE;	//使用软件触发
    ADCx_Handler.Init.DMAContinuousRequests=DISABLE;             //关闭DMA请求
    HAL_ADC_Init(&ADCx_Handler);                       //初始化 
}

//ADC底层驱动，引脚配置，时钟使能
//此函数会被HAL_ADC_Init()调用
//hadc:ADC句柄
void _ADC_Init(rt_uint8_t ch)
{
    GPIO_InitTypeDef GPIO_Initure;
    const struct adc_channel_index *index;
	
	
    __HAL_RCC_ADC1_CLK_ENABLE();            //使能ADC1时钟
    adc1_channels[ch].rcc();			//开启GPIOX时钟
	
    GPIO_Initure.Pin = adc1_channels[ch].pin;            //PA4
    GPIO_Initure.Mode = GPIO_MODE_ANALOG;     //模拟
    GPIO_Initure.Pull = GPIO_NOPULL;          //不带上下拉
    HAL_GPIO_Init(adc1_channels[ch].gpio,&GPIO_Initure);
}

rt_uint16_t Get_Adc(rt_uint8_t ch)   
{
    ADC_ChannelConfTypeDef ADC1_ChanConf;
    
    ADC1_ChanConf.Channel=ch;                                   //通道
    ADC1_ChanConf.Rank=1;                                       //第1个序列，序列1
    ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES;        //采样时间
    ADC1_ChanConf.Offset=0;                 
    HAL_ADC_ConfigChannel(&ADC1_Handler,&ADC1_ChanConf);        //通道配置
	
    HAL_ADC_Start(&ADC1_Handler);                               //开启ADC
	
    HAL_ADC_PollForConversion(&ADC1_Handler,10);                //轮询转换
 
	return (rt_uint16_t)HAL_ADC_GetValue(&ADC1_Handler);	    //返回最近一次ADC1规则组的转换结果
}

rt_err_t convert(struct rt_device *device, int channel, rt_uint16_t *value)
{
	static int old_channel = 0xFF;
	rt_err_t result = RT_EOK;
//	rt_uint16_t temp;

	if(channel > (ADC_CHANNEL_MAX - 1))
	{
		result = -RT_EIO;
		goto _exit;
	}
	if(old_channel != channel)
	{
		_ADC_Init(channel);
		old_channel = channel;
	}
//	temp = Get_Adc(channel);
	*value = Get_Adc(channel); 

_exit:
	return result;
}

static const struct rt_adc_ops adc_ops =
{
	convert,
};


int rt_hw_adc_init(void)
{
    int ret = RT_EOK;
	ADC1_Handler.Instance = ADC1;
	
	ADC_Init(ADC1_Handler);
	
	
	/* add ADC initial. */ 

    ret = rt_device_adc_create("adc1", &adc_ops, RT_NULL);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_adc_init);
