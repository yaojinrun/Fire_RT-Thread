#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <rtthread.h>
#include <rtdevice.h>


#define LSENS_READ_TIMES	10		//定义光敏传感器读取次数,读这么多次,然后取平均值
#define LSENS_CHANNEL	    4
#define LSENS_GET_PERIOD	500


/* 读取Light Sens的值 */
/* 0~100:0,最暗;100,最亮 */
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
		temp = (rt_uint32_t)(val/LSENS_READ_TIMES);//得到平均值
		
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

int adc_test(void)
{
    rt_thread_t tid;
			
	tid = rt_thread_create("Lsens",
		lsens_get_val, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	
	return 0;
}
INIT_APP_EXPORT(adc_test);

