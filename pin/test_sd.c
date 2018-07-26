#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "finsh.h"

/* SD卡的块大小只能为512 */
#define BLOCK_SIZE            512 

/* 定义需要操作的数据块数量 */
#define NUMBER_OF_BLOCKS      10

/* 缓冲区的总字节数 */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)


uint8_t buffer_multiblock_tx[MULTI_BUFFER_SIZE];
uint8_t buffer_multiblock_rx[MULTI_BUFFER_SIZE];


void fill_buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
    uint16_t index = 0;

    for (index = 0; index < BufferLength; index++)
    {
        pBuffer[index] = index + Offset;
    }
}


void sd_test(int argc, char **argv)
{  
	rt_device_t sd_device = RT_NULL;
	uint8_t size;
    
    /* 查找SD设备 */
    sd_device = rt_device_find("sd0");
    if(sd_device != NULL)
    {
        /* 打开设备 */
        rt_device_open(sd_device, RT_DEVICE_OFLAG_RDWR);
        
        /* 填充发送缓冲区，为写操作做准备 */
        fill_buffer(buffer_multiblock_tx, MULTI_BUFFER_SIZE, 0x320F);
        
        /* 把发送缓冲区的数据写入SD卡中 */
        size = rt_device_write(sd_device, 0, buffer_multiblock_tx, NUMBER_OF_BLOCKS);
        if (size != NUMBER_OF_BLOCKS) return;
        
        /* 从SD卡中读出数据，并保存在接收缓冲区中 */
        size = rt_device_read(sd_device, 0, buffer_multiblock_rx, NUMBER_OF_BLOCKS);
        if (size != NUMBER_OF_BLOCKS) return;
        
        /* 比较发送缓冲区和接收缓冲区的内容是否完全一致 */
        if (rt_memcmp(buffer_multiblock_tx, buffer_multiblock_rx, MULTI_BUFFER_SIZE) == 0)
        {
            rt_kprintf("Block test OK!\n");
        }
        else
        {
            rt_kprintf("Block test Fail!\n");
        }
    }
}
MSH_CMD_EXPORT(sd_test, sd card test sample);

