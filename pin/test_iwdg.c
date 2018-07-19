#include <rtthread.h>
#include <rtdevice.h>


#define KEY1	40		//定义按键输入引脚号

#define LED_R     87
#define LED_G     88     

#define LED_ON(x)         rt_pin_write(x ,0)
#define LED_OFF(x)        rt_pin_write(x ,1)

struct rt_device *wdg_dev = NULL;

void iwdg_test_entry(void* parameter)
{
    /* 初始化按键输入 */
    rt_pin_mode(KEY1, PIN_MODE_INPUT);   
    /* 将LED控制脚设置为输出模式 */
    rt_pin_mode(LED_R, PIN_MODE_OUTPUT_OD); 
    rt_pin_mode(LED_G, PIN_MODE_OUTPUT_OD); 
    /* 初始化默认关闭LED灯 */
	LED_ON(LED_R);
	LED_OFF(LED_G);
	while(1)
	{
        if(rt_pin_read(KEY1)==1)
        {
            rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
            LED_OFF(LED_R);
            LED_ON(LED_G);
        }
	}
	
}

int iwdg_test(void)
{
    rt_thread_t tid;
    rt_err_t result = RT_EOK;
    rt_uint32_t timeout = 1000;     //看门狗一出时间为1s
    
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
    
	tid = rt_thread_create("iwdg_test",
		iwdg_test_entry, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
    
    /* 启动看门狗 */
	rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, NULL);
    
	return 0;
}
INIT_APP_EXPORT(iwdg_test);

