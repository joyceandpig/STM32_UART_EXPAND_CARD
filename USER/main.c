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
extern spi1_getpack,spi2_getpack;
const u8 TEXT_TO_SEND[]={"板卡扩展"};
#define TEXT_LENTH  sizeof(TEXT_TO_SEND)-1			//TEXT_TO_SEND字符串长度(不包含结束符)
u8 SendBuff[(TEXT_LENTH+2)*100];

											 //len direct cnt port query config protocalVer expand
uint8_t frame_head[] =  {37,0,1,1,0,0,0,1};
uint8_t frame_head_to_spi2[] =  {11,0,1,1,0,0,0,1};
const u8 *test_to_uart="Test Data from spi1 to uart\r\n";
const u8 *test_to_spi2="Test Data from spi1 to spi2\r\n";
const u8 *uart_to_spi1="Test Data from uart to spi1\r\n";

uint8_t frame_head_config[] = {11,0,1,1,0,1,0,1};
uint8_t test_to_query[] =     {11,0,1,1,1,0,0,1};
//baud prority
uint8_t test_to_config[3] = {1,0,1};
//uint8_t test_to_spi2[] = {};
void test_send_to_spi2(void)
{
	uint8_t i,m = strlen((char*)test_to_spi2);
	uint8_t *p = (u8*)test_to_uart;
	for(i = 0;i<8;i++)
	{
		EnQueue(cir_buf[4],frame_head_to_spi2[i]);
	}
	for(i = 0;i<m;i++)
	{
		EnQueue(cir_buf[4],*p++);
	}
	InsertLink(QLinkList[4],m+8,(u32)cir_buf[4]->rear);
}
void test_uart_to_spi1(void)
{
	uint8_t i,m = strlen((char*)uart_to_spi1);
	uint8_t *p = (u8*)uart_to_spi1;

	for(i = 0;i<m;i++)
	{
		EnQueue(cir_buf[0],*p++);
	}
	InsertLink(QLinkList[0],m,(u32)cir_buf[0]->rear);
}
void test_query(void)
{
	uint8_t i;

	for(i = 0;i<8;i++)
	{
		EnQueue(cir_buf[4],test_to_query[i]);
	}
	InsertLink(QLinkList[4],8,(u32)cir_buf[4]->rear);
}
void test_config(uint8_t k)
{
	uint8_t i;
	for(i = 0;i<8;i++)
	{
		EnQueue(cir_buf[k+3],frame_head_config[i]);
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
	uint8_t *p = (u8*)test_to_uart;
	for(i = 0;i<8;i++)
	{
		EnQueue(cir_buf[4],frame_head[i]);
	}
	for(i = 0;i<m;i++)
	{
		EnQueue(cir_buf[4],*p++);
	}
	InsertLink(QLinkList[4],m+8,(u32)cir_buf[4]->rear);
}
void system_bsp_init(void)
{
	mallco_dev.init(SRAMIN);
	KEY_Init();
	EXTIX_Init();
	LED_Init();
	SPI1_Init();
	SPI2_Init();
	TIM3_Int_Init(72*100,0);
	comm_mem_malloc();
	memp_init();
	
	CirQueue_LinkList_init();
	
	uart_config();
	delay_init();
	
	usmart_dev.init(96);
}
int main(void)
{ 
	uint8_t i = 0;
	uint32_t cnt = 0;
	uint8_t key_val = 0;
	uint16_t spi2_rec_len = 0,j = 0;
	uint8_t spi2_rec_val;
	uint8_t spi2_rx_status =0;
	uint8_t spi2_rx_h,spi2_rx_l;
	system_bsp_init();
	while(1)
	{
		/*数据查询与处理*/
		for(i = 0; i < 6; i++){
		/*按键选择功能*/
		key_val = KEY_Scan(0);
		if(key_val == WKUP_PRES){
			test_send_to_uart();
//			test_send_to_spi2();
//			test_uart_to_spi1();
		}else if(key_val == KEY1_PRES){
			test_query();
		}else if(key_val == KEY0_PRES){
			test_config(1);
		}
		if(!StartSPI2RecData_flag){
			if(QLinkList[spi1_rx]->next != NULL){
				apply_layer_input(spi1_rx);
			}
			if(QLinkList[i]->next != NULL){
				apply_layer_input(i);
			}
			if(QLinkList[i+uart1_tx]->next != NULL){
				apply_layer_output(i+uart1_tx);
			}
		}
			//spi从机有数据发送回来
			if(StartSPI2RecData_flag){
				switch(spi2_rx_status){
				case 0://接收数据长度高位
					spi2_rec_len = SPI_ReadWriteByte(SPI2,0xaa);
					spi2_rx_status = 1;
					break;
				case 1://接收数据长度低位
					spi2_rec_len = (spi2_rec_len << 8)|SPI_ReadWriteByte(SPI2,0xaa);
					printf("spi2 rec len :%d \r\n",spi2_rec_len-8);
					spi2_rx_status = 2;
					if(spi2_rec_len > MEMP_BUF_LAYER_TYPE_SIZE)
						spi2_rx_status = 0;
					break;
				case 2://接收真正的数据
					j++;
					spi2_rec_val = SPI_ReadWriteByte(SPI2,0xFF);
					if(j == spi2_rec_len){
						spi2_rx_status = 3;
					}
				  if(j>8){
						usart_send(0,spi2_rec_val);
					}
					break;
				case 3://接收最后一个数据，并重置状态机及计数变量
					spi2_rec_val = SPI_ReadWriteByte(SPI2,0xFE);
					usart_send(0,spi2_rec_val);
					j = 0;
					printf("\r\nspi2 rec end");
					printf("\r\n");
					spi2_rx_status = 4;
					break;
				case 4:
					spi2_rx_status = 0;
					break;
				}
				StartSPI2RecData_flag = 0;
			}
			if(i == 5){
				i = 255;
			}
		/*指示灯闪烁提示*/
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




