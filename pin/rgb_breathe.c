#include <rtthread.h>
#include <rtdevice.h>

#define     PERIOD    512

#define     CH_R    1
#define     CH_G    2
#define     CH_B    3

/* LED亮度等级 PWM表,指数曲线 */
/*此表使用工程目录下的python脚本index_wave.py生成*/
uint16_t indexWave[] = {
1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
	2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 
	6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 
	12, 13, 14, 15, 17, 18, 19, 20, 22, 
	23, 25, 27, 29, 31, 33, 36, 38, 41, 
	44, 47, 51, 54, 58, 63, 67, 72, 77, 
	83, 89, 95, 102, 110, 117, 126, 135,
	145, 156, 167, 179, 192, 206, 221, 237, 
	254, 272, 292, 313, 336, 361, 387, 415, 
	445, 477, 512, 512, 477, 445, 415, 387, 
	361, 336, 313, 292, 272, 254, 237, 221, 
	206, 192, 179, 167, 156, 145, 135, 126, 
	117, 110, 102, 95, 89, 83, 77, 72, 67, 63,
	58, 54, 51, 47, 44, 41, 38, 36, 33, 31, 29,
	27, 25, 23, 22, 20, 19, 18, 17, 15, 14, 13,
	12, 12, 11, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5,
	5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 1, 1, 1, 1, 1
};

/*计算PWM表有多少个元素*/
rt_uint16_t POINT_NUM = sizeof(indexWave)/sizeof(indexWave[0]); 

void rt_pwm_rgb_entry(void* parameter)
{
    rt_uint16_t i = 0;
    
    /*使能各通道*/
    rt_pwm_enable(CH_R);
    rt_pwm_enable(CH_G);
    rt_pwm_enable(CH_B);
        
    while(1)
    {
        rt_pwm_set(CH_R, PERIOD, indexWave[i]);
        rt_pwm_set(CH_G, PERIOD, indexWave[i]);
        rt_pwm_set(CH_B, PERIOD, indexWave[i]);
        
        if(i >= POINT_NUM-1) i = 0;
        else i++;
        
        /* 设置使得整个呼吸过程为3秒左右即可达到很好的效果 */
        rt_thread_delay(3000/POINT_NUM);
    }
}



int rt_breathe_rgb_init()
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
MSH_CMD_EXPORT(rt_breathe_rgb_init, rgb breathe by pwm.);
