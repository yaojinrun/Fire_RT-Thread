#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "stm32f4xx.h"

#define PWM_CHANNEL_MAX     (4) /* 1-4 */

struct rt_stm32_pwm
{
    struct rt_device_pwm parent;

    rt_uint32_t period[PWM_CHANNEL_MAX];
    rt_uint32_t pulse[PWM_CHANNEL_MAX];
};
static struct rt_stm32_pwm _stm32_pwm_device;


static rt_err_t set(struct rt_device_pwm *device, struct rt_pwm_configuration *configuration)
{
    TIM_TypeDef *TIMx;         //定时器x PWM句柄
    rt_err_t result = RT_EOK;
    struct rt_stm32_pwm *stm32_pwm_device = (struct rt_stm32_pwm *)device;
    
    TIMx = (TIM_TypeDef *)device->parent.user_data;

    if ((configuration->channel > PWM_CHANNEL_MAX)||(configuration->channel < 1))
    {
        result = -RT_EIO;
        goto _exit;
    }

    rt_kprintf("drv_pwm.c set channel: %d, period: %d, pulse: %d\n", configuration->channel, configuration->period, configuration->pulse);

    stm32_pwm_device->period[configuration->channel] = configuration->period;
    stm32_pwm_device->pulse[configuration->channel] = configuration->pulse;
    TIMx->ARR = configuration->period;
    switch (configuration->channel)
    {
        case 1:
            TIMx->CCR1 = configuration->pulse; 
        break;
        
        case 2:
            TIMx->CCR2 = configuration->pulse; 
        break;
        
        case 3:
            TIMx->CCR3 = configuration->pulse; 
        break;
        
        case 4:
            TIMx->CCR4 = configuration->pulse; 
        break;
        
        default:
        break;
    }
    
_exit:
    return result;
}

static rt_err_t get(struct rt_device_pwm *device, struct rt_pwm_configuration *configuration)
{
    rt_err_t result = RT_EOK;
    struct rt_stm32_pwm *stm32_pwm_device = (struct rt_stm32_pwm *)device;
	
    if ((configuration->channel > PWM_CHANNEL_MAX)||(configuration->channel < 1))
    {
        result = -RT_EIO;
        goto _exit;
    }
	
	configuration->period = stm32_pwm_device->period[configuration->channel];
	configuration->pulse = stm32_pwm_device->pulse[configuration->channel];
	
_exit:
    return result;
}

static rt_err_t control(struct rt_device_pwm *device, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;

    rt_kprintf("drv_pwm.c control cmd: %d. \n", cmd);

    if (cmd == PWM_CMD_ENABLE)
    {
        struct rt_pwm_configuration *cfg;
        uint32_t TIM_CHANNEL_X = 0;
        TIM_TypeDef *TIMx;         //定时器x PWM句柄
        TIM_HandleTypeDef TIMx_Handler; 
        
        TIMx = (TIM_TypeDef *)device->parent.user_data;
        TIMx_Handler.Instance = TIMx;
        
        cfg = (struct rt_pwm_configuration *)arg;
        
        if ((cfg->channel > PWM_CHANNEL_MAX)||(cfg->channel < 1))
        {
            result = -RT_EIO;
            return result;
        }
        TIM_CHANNEL_X = (cfg->channel-1)*4;
        HAL_TIM_PWM_Start(&TIMx_Handler, TIM_CHANNEL_X);//开启PWM通道1
        
        rt_kprintf("PWM_CMD_ENABLE\n");
    }
    else if (cmd == PWM_CMD_SET)
    {
        rt_kprintf("PWM_CMD_SET\n");
        result = set(device, (struct rt_pwm_configuration *)arg);
    }
    else if (cmd == PWM_CMD_GET)
    {
        rt_kprintf("PWM_CMD_GET\n");
        result = get(device, (struct rt_pwm_configuration *)arg);
    }

    return result;
}

static const struct rt_pwm_ops pwm_ops =
{
    control,
};

int rt_hw_pwm_init(void)
{
    int ret = RT_EOK;
    TIM_TypeDef *TIMx;         //定时器x PWM句柄 
    GPIO_InitTypeDef  GPIO_InitStruct;
    TIM_HandleTypeDef TIMx_Handler;         //定时器3PWM句柄 
    TIM_OC_InitTypeDef TIMx_CHxHandler;	    //定时器3通道4句柄
    
    TIMx = TIM5;

    /* add pwm initial. */
    
    __HAL_RCC_TIM5_CLK_ENABLE();			//使能定时器3
    __HAL_RCC_GPIOH_CLK_ENABLE();			//开启GPIOB时钟
    
    /* TIM5 channel1 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    /* TIM5 channel2 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    /* TIM5 channel3 GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    TIMx_Handler.Instance = TIMx;
    TIMx_Handler.Init.Prescaler = 12-1;    //SystemCoreClock/180 = 1MHz
    TIMx_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;    //向上计数模式
    TIMx_Handler.Init.Period = 256-1;          //自动重装载值
    TIMx_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIMx_Handler);       //初始化PWM
    
    TIMx_CHxHandler.OCMode = TIM_OCMODE_PWM1; //模式选择PWM1
    TIMx_CHxHandler.Pulse = 20;            //设置比较值,此值用来确定占空比，默认比较值为自动重装载值的一半,即占空比为50%
    TIMx_CHxHandler.OCPolarity = TIM_OCPOLARITY_LOW; //输出比较极性为低 
    
    HAL_TIM_PWM_ConfigChannel(&TIMx_Handler, &TIMx_CHxHandler, TIM_CHANNEL_1);    //配置TIMx通道1
    HAL_TIM_PWM_ConfigChannel(&TIMx_Handler, &TIMx_CHxHandler, TIM_CHANNEL_2);    //配置TIMx通道2
    HAL_TIM_PWM_ConfigChannel(&TIMx_Handler, &TIMx_CHxHandler, TIM_CHANNEL_3);    //配置TIMx通道3

    ret = rt_device_pwm_register(&_stm32_pwm_device.parent, "pwm", &pwm_ops, TIMx);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);
