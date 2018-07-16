/*
 * File      : uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2012 - 2015, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <rtthread.h>
#include <rtdevice.h>

#define BUFF_SIZE 	64

#define LED_R 	87
#define LED_G 	88	
#define LED_B 	89	

#define LED_ON(x) 		rt_pin_write(x ,0)
#define LED_OFF(x) 		rt_pin_write(x ,1)

static char uart_rx_buffer[BUFF_SIZE];

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
	static rt_uint8_t pos = 0;
	
	if(size == 1)
	{
		rt_device_read(dev, 0, &uart_rx_buffer[pos], size);
		if((pos == 0)&&(uart_rx_buffer[pos] == 'L')) pos++;
		else if((pos == 1)&&(uart_rx_buffer[pos] == 'E')) pos++;
		else if((pos == 2)&&(uart_rx_buffer[pos] == 'D')) pos++;
		else if((pos == 3)&&(uart_rx_buffer[pos] == ' ')) pos++;
		else if (pos == 4)
		{
			if(uart_rx_buffer[pos] == 'R')
			{
				LED_ON(LED_R);
				LED_OFF(LED_G);
				LED_OFF(LED_B);
			}
			else if(uart_rx_buffer[pos] == 'G')
			{
				LED_OFF(LED_R);
				LED_ON(LED_G);
				LED_OFF(LED_B);
			}
			else if(uart_rx_buffer[pos] == 'B')
			{
				LED_OFF(LED_R);
				LED_OFF(LED_G);
				LED_ON(LED_B);
			}
			pos = 0;
		}
		else pos = 0;
	}
	else
		rt_kprintf("Please set the uart to interrupt RX mode!\n");

    return RT_EOK;
}

int uart_sample(void)
{
    rt_thread_t tid;
	rt_device_t uart_device;

	uart_device = rt_device_find("uart1");  
	rt_device_set_rx_indicate(uart_device, uart_rx_ind);
	rt_device_open(uart_device, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX));

    return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(uart_sample, uart sample);
