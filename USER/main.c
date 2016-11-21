#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"
#include "spi.h"
#include "usmart.h"
#include "dma.h"
#include "malloc.h"
#include "common.h"
#include "pbuf.h"
#include "exti.h"
//成都淞幸科技有限责任公司
//串口板卡扩展代码

const u8 TEXT_TO_SEND[]={"板卡扩展"};
#define TEXT_LENTH  sizeof(TEXT_TO_SEND)-1			//TEXT_TO_SEND字符串长度(不包含结束符)
u8 SendBuff[(TEXT_LENTH+2)*100];
//len direct  cnt port query  config protocalVer  expand
uint8_t frame_head[] =  {37,0,0,1,0,0,0,0};
const u8 *test_to_uart="ENC28J60 TCP Client send data\r\n";

uint8_t frame_head1[] = {11,0,0,1,0,1,0,0};
uint8_t test_to_query[] = {11,0,0,1,1,0,0,0};
//baud prority
uint8_t test_to_config[3] = {1,0,1};
//uint8_t test_to_spi2[] = {};

void test_query(void)
{
	uint8_t i,m = strlen((char*)test_to_uart);
	for(;i<8;i++)
	{
		EnQueue(cir_buf[4],test_to_query[i]);
	}
	InsertLink(QLinkList[4],8,(u32)cir_buf[4]->rear);
}
void test_config(uint8_t k)
{
	uint8_t i,m = strlen((char*)test_to_uart);
	for(;i<8;i++)
	{
		EnQueue(cir_buf[k+3],frame_head1[i]);
	}
	for(i = 0;i<3;i++)
	{
		EnQueue(cir_buf[k+3],test_to_config[i]);
	}
	InsertLink(QLinkList[k+3],11,(u32)cir_buf[k+3]->rear);
}
void test_send_to_uart(void)
{
	uint8_t i,m = strlen((char*)test_to_uart);
	for(;i<8;i++)
	{
		EnQueue(cir_buf[4],frame_head[i]);
	}
	for(i = 0;i<m;i++)
	{
		EnQueue(cir_buf[4],*test_to_uart++);
	}
	InsertLink(QLinkList[4],m+8,(u32)cir_buf[4]->rear);
}
void system_bsp_init(void)
{
	mallco_dev.init(SRAMIN);
	KEY_Init();
	LED_Init();
	SPI1_Init();
	SPI2_Init();
	comm_mem_malloc();
	memp_init();
	
	queue_init();
	
	uart_config();
	delay_init();
	
	usmart_dev.init(96);
}
int main(void)
{ 
	uint8_t i = 0;
	uint32_t cnt = 0;
	
	system_bsp_init();
//	test_config(1);
//	test_send_to_uart();
	test_query();
	while(1)
	{
		for(i = 0; i < 6; i++){
			if(QLinkList[i]->next != NULL){
				apply_layer_input(i);
			}
			if(QLinkList[i+6]->next != NULL){
				apply_layer_output(i+6);
			}
			if(StartSPI2RecData_flag){
				SPI2_ReadWriteByte(0xFF);
			}
			if(i == 5){
				i = 255;
			}
			
		cnt++;
		if(cnt <= 100000){
			LED0 = 0;
		}else if(cnt <= 400000){
			LED0 = 1;
		}else
		{
			cnt = 0;
		}
		}


	}
}




