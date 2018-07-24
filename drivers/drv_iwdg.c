#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"


IWDG_HandleTypeDef IWDG_Handler; //�������Ź����

/*
 * ���� IWDG �ĳ�ʱʱ��
 * Tout = prv/40 * rlv (s)
 *      prv������[4,8,16,32,64,128,256]
 * prv:Ԥ��Ƶ��ֵ��ȡֵ���£�
 *     @arg IWDG_Prescaler_4: IWDG prescaler set to 4
 *     @arg IWDG_Prescaler_8: IWDG prescaler set to 8
 *     @arg IWDG_Prescaler_16: IWDG prescaler set to 16
 *     @arg IWDG_Prescaler_32: IWDG prescaler set to 32
 *     @arg IWDG_Prescaler_64: IWDG prescaler set to 64
 *     @arg IWDG_Prescaler_128: IWDG prescaler set to 128
 *     @arg IWDG_Prescaler_256: IWDG prescaler set to 256
 *
 *		�������Ź�ʹ��LSI��Ϊʱ�ӡ�
 *		LSI ��Ƶ��һ���� 30~60KHZ ֮�䣬�����¶Ⱥ͹������ϻ���һ����Ư�ƣ���
 *		��һ��ȡ 40KHZ�����Զ������Ź��Ķ�ʱʱ�䲢һ���ǳ���ȷ��ֻ�����ڶ�ʱ�侫��
 *		Ҫ��Ƚϵ͵ĳ��ϡ�
 *
 * rlv:Ԥ��Ƶ��ֵ��ȡֵ��ΧΪ��0-0XFFF
 * �������þ�����
 * IWDG_Config(IWDG_Prescaler_64 ,625);  // IWDG 1s ��ʱ��� 
 *						��64/40��*625 = 1s
 */


rt_err_t iwdg_init(rt_watchdog_t *wdt)
{ 
    IWDG_HandleTypeDef *_IWDG_Handler = wdt->parent.user_data;  /* �������Ź���� */
    
    HAL_IWDG_Init(_IWDG_Handler);		/* ��ʼ��IWDG */
	
    return RT_EOK;
}



rt_err_t wdg_control(rt_watchdog_t *wdt, int cmd, void *arg)
{
    rt_err_t result = RT_EOK;
    IWDG_HandleTypeDef *_IWDG_Handler = wdt->parent.user_data; //�������Ź����
    rt_uint32_t *timeout = arg;
    
    switch (cmd)
    {
        case RT_DEVICE_CTRL_WDT_START:
            __HAL_IWDG_START(_IWDG_Handler);		/* �����������Ź� */
        break;
        
        case RT_DEVICE_CTRL_WDT_STOP:               /* һ���������Ź����������͹ز�����ֻ�и�λ���ܹص� */
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
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_4; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/4;		/* ��װ��ֵ */
            }
            else if((*timeout) < 819)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_8; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/8;		/* ��װ��ֵ */
            }
            else if((*timeout) < 1638)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_16; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/16;		/* ��װ��ֵ */
            }
            else if((*timeout) < 3276)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_32; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/32;		/* ��װ��ֵ */
            }
            else if((*timeout) < 65520)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_64; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/64;		/* ��װ��ֵ */
            }
            else if((*timeout) < 13104)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_128; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/128;		/* ��װ��ֵ */
            }
            else if((*timeout) < 26208)
            {
                _IWDG_Handler->Init.Prescaler = IWDG_PRESCALER_256; 	    /* ����IWDG��Ƶϵ�� */
                _IWDG_Handler->Init.Reload = 40*(*timeout)/256;		/* ��װ��ֵ */
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
