#include <rtthread.h>
#include <rtdevice.h>

#define     PERIOD    255

#define     CH_R    1
#define     CH_G    2
#define     CH_B    3

void rt_pwm_rgb_entry(void* parameter)
{
    rt_uint8_t random;
    /*使能各通道*/
    rt_pwm_enable(CH_R);
    rt_pwm_enable(CH_G);
    rt_pwm_enable(CH_B);
        
    while(1)
    {
        random = rand()%255;    /*产生一个伪随机数*/
        rt_pwm_set(CH_R, PERIOD, random);   /*用产生的伪随机数决定pwm波的占空比*/
        random = rand()%255;
        rt_pwm_set(CH_G, PERIOD, random);
        random = rand()%255;
        rt_pwm_set(CH_B, PERIOD, random);
        
        rt_thread_delay(500);
    }
}



int rt_pwm_rgb_init()
{
    rt_device_t pwm_dev;
    rt_thread_t tid;
	
	pwm_dev = rt_device_find("pwm");
	if(NULL != pwm_dev)
	{
        tid = rt_thread_create("pwm_led",
            rt_pwm_rgb_entry, RT_NULL,
            512, RT_THREAD_PRIORITY_MAX/3, 20);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
	}

    return 0;
}
MSH_CMD_EXPORT(rt_pwm_rgb_init, rgb by pwm.);
