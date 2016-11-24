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
//�ɶ����ҿƼ��������ι�˾
//���ڰ忨��չ����

const u8 TEXT_TO_SEND[]={"�忨��չ"};
#define TEXT_LENTH  sizeof(TEXT_TO_SEND)-1			//TEXT_TO_SEND�ַ�������(������������)
u8 SendBuff[(TEXT_LENTH+2)*100];

											 //len direct cnt port query config protocalVer expand
uint8_t frame_head[] =  {37,0,1,1,0,0,0,1};
uint8_t frame_head_to_spi2[] =  {11,0,1,1,0,0,0,1};
const u8 *test_to_uart="Test Data from spi2 to uart\r\n";
const u8 *test_to_spi2="Test Data from spi1 to spi2\r\n";

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
	uint8_t i = 0,j = 0;
	uint32_t cnt = 0;
	uint8_t key_val = 0;
	uint8_t spi2_rec_len = 0;
	uint8_t spi2_rec_val;
	system_bsp_init();
	while(1)
	{

		/*���ݲ�ѯ�봦��*/
		for(i = 0; i < 6; i++){
		/*����ѡ����*/
		key_val = KEY_Scan(0);
		if(key_val == WKUP_PRES){
//			test_send_to_uart();
			test_send_to_spi2();
		}else if(key_val == KEY1_PRES){
			test_query();
		}else if(key_val == KEY0_PRES){
			test_config(1);
		}
		
			if(QLinkList[i]->next != NULL){
				apply_layer_input(i);
			}
			if(QLinkList[i+6]->next != NULL){
				apply_layer_output(i+6);
			}
//			if(StartSPI2RecData_flag){
//				spi2_rec_len = SPI_ReadWriteByte(SPI2,0xFF);
//				printf("spi2 rec len :%d\r\n",spi2_rec_len);
//				delay_ms(10);
//				for(j = 0; j < spi2_rec_len;j++){
//					spi2_rec_val = SPI_ReadWriteByte(SPI2,0xFF);
//					printf("spi2 received data is: %x \r\n",spi2_rec_val);
//					delay_ms(10);
////					EnQueue(cir_buf[spi2_rx],spi2_rec_val);
//				}
////				InsertLink(QLinkList[spi2_rx],spi2_rec_val,(u32)(cir_buf[spi2_rx]->rear));
//				StartSPI2RecData_flag = 0;
//			}
			if(i == 5){
				i = 255;
			}
		/*ָʾ����˸��ʾ*/
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




