#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"


IWDG_HandleTypeDef IWDG_Handler; //独立看门狗句柄

/*
 * 设置 IWDG 的超时时间
 * Tout = prv/40 * rlv (s)
 *      prv可以是[4,8,16,32,64,128,256]
 * prv:预分频器值，取值如下：
 *     @arg IWDG_Prescaler_4: IWDG prescaler set to 4
 *     @arg IWDG_Prescaler_8: IWDG prescaler set to 8
 *     @arg IWDG_Prescaler_16: IWDG prescaler set to 16
 *     @arg IWDG_Prescaler_32: IWDG prescaler set to 32
 *     @arg IWDG_Prescaler_64: IWDG prescaler set to 64
 *     @arg IWDG_Prescaler_128: IWDG prescaler set to 128
 *     @arg IWDG_Prescaler_256: IWDG prescaler set to 256
 *
 *		独立看门狗使用LSI作为时钟。
 *		LSI 的频率一般在 30~60KHZ 之间，根据温度和工作场合会有一定的漂移，我
 *		们一般取 40KHZ，所以独立看门狗的定时时间并一定非常精确，只适用于对时间精度
 *		要求比较低的场合。
 *
 * rlv:预分频器值，取值范围为：0-0XFFF
 * 函数调用举例：
 * IWDG_Config(IWDG_Prescaler_64 ,625);  // IWDG 1s 超时溢出 
 *						（64/40）*625 = 1s
 */


rt_err_t iwdg_init(rt_watchdog_t *wdt)
{ 
    IWDG_HandleTypeDef *_IWDG_Handler = wdt->parent.user_data;  /* 独立看门狗句柄 */
    
    HAL_IWDG_Init(_IWDG_Handler);		/* 初始化IWDG */
	
    return RT_EOK;
}



rt_err_t wdg_control(rt_watchdog_t *wdt, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;
    IWDG_HandleTypeDef *_IWDG_Handler = wdt->parent.user_data; //独立看门狗句柄
    rt_uint32_t *timeout = arg;
    
    switch (cmd)
    {
        case RT_DEVICE_CTRL_WDT_START:
            __HAL_IWDG_START(_IWDG_Handler);		/* 开启独立看门狗 */
        break;
        
        case RT_DEVICE_CTRL_WDT_STOP:               /* 一旦独立看门狗启动，它就关不掉，只有复位才能关掉 */
            result = -RT_ERROR; 
        break;
        
        case RT_DEVICE_CTRL_WDT_KEEPALIVE:
            HAL_IWDG_Refresh(_IWDG_Handler);
        break;
        
        case RT_DEVICE_CTRL_WDT_GET_TIMELEFT:
            *timeout = _IWDG_Handler->Init.Prescaler/40*(_IWDG_Handler->Init.Reload - _IWDG_Handler->Instance->KR); 
        break;
        
        case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
            if((*timeout)<409)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_4; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/4;		/* 重装载值 */
            }
            else if((*timeout) < 819)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_8; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/8;		/* 重装载值 */
            }
            else if((*timeout) < 1638)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_16; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/16;		/* 重装载值 */
            }
            else if((*timeout) < 3276)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_32; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/32;		/* 重装载值 */
            }
            else if((*timeout) < 65520)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_64; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/64;		/* 重装载值 */
            }
            else if((*timeout) < 13104)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_128; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/128;		/* 重装载值 */
            }
            else if((*timeout) < 26208)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_256; 	    /* 设置IWDG分频系数 */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/256;		/* 重装载值 */
            }
            else result = -RT_ERROR;
            HAL_IWDG_Init(_IWDG_Handler);
        break;
        
        case RT_DEVICE_CTRL_WDT_GET_TIMEOUT:
            *timeout = _IWDG_Handler->Init.Prescaler/40*_IWDG_Handler->Init.Reload; 
        break;
        
        default:
        break;
    }
    return result;
}

static const struct rt_watchdog_ops wdg_ops =
{
	NULL,
    wdg_control
};

rt_watchdog_t wdg;

int rt_hw_iwdg_init(void)
{
    int ret = RT_EOK;
    
    IWDG_Handler.Instance = IWDG;
    
    wdg.ops = &wdg_ops;
    ret = rt_hw_watchdog_register(&wdg, "iwdg", NULL, &IWDG_Handler);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_iwdg_init);
