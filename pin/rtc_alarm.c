#include <rtthread.h>
#include <rtdevice.h>


#define BEEP        13      //����������
#define KEY1        40		//���尴���������ź�


#define BEEP_ON(x)         rt_pin_write(x ,1)
#define BEEP_OFF(x)        rt_pin_write(x ,0)

#define ALARM_NUM_MAX	10

struct rt_alarm *ALARM[ALARM_NUM_MAX];

void alarm_callback_entry(rt_alarm_t alarm, time_t timestamp)
{
    /* �򿪷����� */
    BEEP_ON(BEEP);
	rt_kprintf("Time out��\n");
}

void alarm_test_entry(void* parameter)
{

	while(1)
	{
        if(rt_pin_read(KEY1)==1)
        {
            BEEP_OFF(BEEP);
        }
        rt_thread_delay(20);
	}
	
}

int alarm_init(void)
{
    rt_thread_t tid;
    
    /* ��ʼ���������� */
    rt_pin_mode(KEY1, PIN_MODE_INPUT); 
    
    /* �����������ƽ�����Ϊ���ģʽ */
    rt_pin_mode(BEEP, PIN_MODE_OUTPUT); 
    /* ������Ĭ�ϲ��� */
    BEEP_OFF(BEEP);
    
    rt_memset(&ALARM, NULL, sizeof(ALARM));
    
    
	tid = rt_thread_create("alarm_test",
		alarm_test_entry, RT_NULL,
		512, RT_THREAD_PRIORITY_MAX/3, 20);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
    
    /* ��ʼ�����ӷ��� */
    rt_alarm_system_init();
    
	return 0;
}
INIT_APP_EXPORT(alarm_init);


void alarm_add(int argc, char **argv)
{
    struct rt_alarm_setup _alarm;
	
    
	if(argc > 1)
	{
        _alarm.flag = RT_ALARM_ONESHOT;    /* ���ζ�ʱ */
        rt_memset(&_alarm.wktime, RT_ALARM_TM_NOW, sizeof(struct tm));
        _alarm.wktime.tm_hour  = atol(argv[1]);
        if(argc > 2)
        {
            _alarm.wktime.tm_min = atol(argv[2]);
            _alarm.wktime.tm_sec = 0;
        }
        if(ALARM[0] == NULL)
        {
            ALARM[0] = rt_alarm_create(alarm_callback_entry, &_alarm);
        }
        else 
        {
            rt_alarm_control(ALARM[0], RT_ALARM_CTRL_MODIFY, &_alarm);
        }
		rt_alarm_start(ALARM[0]);
        
	}
}
MSH_CMD_EXPORT(alarm_add, e.g: alarm_add 12 15);    /* ��ʱʱ��Ϊ12:15 */
