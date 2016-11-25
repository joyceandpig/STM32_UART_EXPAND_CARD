#include "dma.h"
#include "common.h"
#include "usart.h"
#include "pbuf.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK miniSTM32������
//DMA ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/8
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

DMA_InitTypeDef DMA_InitStructure;



void* dma_channel_addr[12] = {

#if UART1_RX_DMA
UART1_RX_DMA_CHANNEL ,	
#endif
#if UART2_RX_DMA
UART2_RX_DMA_CHANNEL ,	
#endif
#if UART3_RX_DMA
UART3_RX_DMA_CHANNEL ,	
#endif
#if UART4_RX_DMA
UART4_RX_DMA_CHANNEL, 
#endif
#if SPI1_RX_DMA
SPI1_RX_DMA_CHANNEL ,	
#endif
#if SPI2_RX_DMA
SPI2_RX_DMA_CHANNEL,
#endif
	

	
#if UART1_TX_DMA
UART1_TX_DMA_CHANNEL ,	
#endif
#if UART2_TX_DMA
UART2_TX_DMA_CHANNEL ,	
#endif
#if UART3_TX_DMA
UART3_TX_DMA_CHANNEL ,	
#endif
#if UART4_TX_DMA
UART4_TX_DMA_CHANNEL ,	
#endif
#if SPI1_TX_DMA
SPI1_TX_DMA_CHANNEL ,
#endif
#if SPI2_TX_DMA
SPI2_TX_DMA_CHANNEL, 
#endif	
 };
#define SPI1_DR_Addr ( (u32)0x4001300C )  
#define SPI2_DR_Addr ( (u32)0x4000380C )  
 
u8 DMA_Init_Config(__IO uint8_t type )
{
	
	u32 per_addr;
	DMA_Cmd(dma_channel_addr[type], DISABLE);
	if(type == uart4_rx|| type == uart4_tx){
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//ʹ��DMA����
	}else{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	}

	DMA_DeInit(dma_channel_addr[type]);
		
	if((type!= spi1_rx) && (type != spi1_tx) && (type != spi2_rx) && (type !=spi2_tx))
	{
			per_addr = type >5? (uint32_t)&(UART4->DR):type > 3?(uint32_t)&(USART3->DR):type > 1?(uint32_t)&(USART2->DR):(uint32_t)&(USART1->DR);
			DMA_InitStructure.DMA_PeripheralBaseAddr = per_addr;  //DMA����ADC����ַ
			DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(cir_buf[type]->queue_buf);  //DMA�ڴ����ַ
	}else {
			per_addr = type > 9?SPI2_DR_Addr:SPI1_DR_Addr;
			DMA_InitStructure.DMA_PeripheralBaseAddr = per_addr;  //DMA����ADC����ַ
			DMA_InitStructure.DMA_MemoryBaseAddr = *cir_buf[type]->queue_buf;  //DMA�ڴ����ַ
	}
	if(type > 5){
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	}else{
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����赽�ڴ�
	}
	
	DMA_InitStructure.DMA_BufferSize = MEMP_BUF_LAYER_TYPE_SIZE;  //DMAͨ����DMA����Ĵ�С
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //DMAͨ�� xӵ�������ȼ� 
//	if(type > 7){
//		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //DMAͨ�� xӵ�������ȼ� 
//	}
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(dma_channel_addr[type], &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	
	USART_DMACmd(COM_USART[type], USART_DMAReq_Rx, ENABLE);
//	DMA_ITConfig(UART1_TX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
	
	DMA_Cmd(dma_channel_addr[type], ENABLE);	
//	if(type > 7)DMA_Cmd(dma_channel_addr[type], DISABLE);	
}
//����һ��DMA����
void MY_SPI_DMA_SEND_Enable( device_type type, u8 sendnum)
{ 
	DMA_Cmd(dma_channel_addr[type], DISABLE );  //�ر�SPI ��ָʾ��ͨ��      
 	DMA_SetCurrDataCounter(dma_channel_addr[type],sendnum);//DMAͨ����DMA����Ĵ�С,�����˴η��͵����ݸ���
 	DMA_Cmd(dma_channel_addr[type], ENABLE);  //ʹ��SPI ��ָʾ��ͨ�� 
}	  

void MY_UART_DMA_SEND_ENABLE(device_type type, u8 sendnum)
{
	DMA_Cmd(dma_channel_addr[type], DISABLE );  //�ر�SPI ��ָʾ��ͨ��      
	
 	DMA_SetCurrDataCounter(dma_channel_addr[type],sendnum);//DMAͨ����DMA����Ĵ�С,�����˴η��͵����ݸ���
 	DMA_Cmd(dma_channel_addr[type], ENABLE);  //ʹ��SPI ��ָʾ��ͨ�� 
}
void DMA1_Channel2_IRQHandler(void)
{
		DMA_ITConfig(SPI1_TX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		DMA_Cmd(SPI1_TX_DMA_CHANNEL,DISABLE);
}
void DMA1_Channel3_IRQHandler(void)
{
		DMA_ITConfig(SPI1_RX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		DMA_Cmd(SPI1_RX_DMA_CHANNEL,DISABLE);
}
void DMA1_Channel4_IRQHandler(void)
{
		DMA_ITConfig(SPI2_TX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		DMA_Cmd(SPI1_TX_DMA_CHANNEL,DISABLE);
}
void DMA1_Channel5_IRQHandler(void)
{
		DMA_ITConfig(SPI2_RX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
		DMA_Cmd(SPI2_RX_DMA_CHANNEL,DISABLE);
}
















