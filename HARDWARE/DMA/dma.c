#include "dma.h"
#include "common.h"
#include "usart.h"
#include "pbuf.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK miniSTM32开发板
//DMA 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
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
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//使能DMA传输
	}else{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	}

	DMA_DeInit(dma_channel_addr[type]);
		
	if((type!= spi1_rx) && (type != spi1_tx) && (type != spi2_rx) && (type !=spi2_tx))
	{
			per_addr = type >5? (uint32_t)&(UART4->DR):type > 3?(uint32_t)&(USART3->DR):type > 1?(uint32_t)&(USART2->DR):(uint32_t)&(USART1->DR);
			DMA_InitStructure.DMA_PeripheralBaseAddr = per_addr;  //DMA外设ADC基地址
			DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(cir_buf[type]->queue_buf);  //DMA内存基地址
	}else {
			per_addr = type > 9?SPI2_DR_Addr:SPI1_DR_Addr;
			DMA_InitStructure.DMA_PeripheralBaseAddr = per_addr;  //DMA外设ADC基地址
			DMA_InitStructure.DMA_MemoryBaseAddr = *cir_buf[type]->queue_buf;  //DMA内存基地址
	}
	if(type > 5){
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	}else{
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设到内存
	}
	
	DMA_InitStructure.DMA_BufferSize = MEMP_BUF_LAYER_TYPE_SIZE;  //DMA通道的DMA缓存的大小
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //DMA通道 x拥有中优先级 
//	if(type > 7){
//		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //DMA通道 x拥有中优先级 
//	}
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(dma_channel_addr[type], &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
	
	USART_DMACmd(COM_USART[type], USART_DMAReq_Rx, ENABLE);
//	DMA_ITConfig(UART1_TX_DMA_CHANNEL,DMA_IT_TC,DISABLE);
	
	DMA_Cmd(dma_channel_addr[type], ENABLE);	
//	if(type > 7)DMA_Cmd(dma_channel_addr[type], DISABLE);	
}
//开启一次DMA传输
void MY_SPI_DMA_SEND_Enable( device_type type, u8 sendnum)
{ 
	DMA_Cmd(dma_channel_addr[type], DISABLE );  //关闭SPI 所指示的通道      
 	DMA_SetCurrDataCounter(dma_channel_addr[type],sendnum);//DMA通道的DMA缓存的大小,决定此次发送的数据个数
 	DMA_Cmd(dma_channel_addr[type], ENABLE);  //使能SPI 所指示的通道 
}	  

void MY_UART_DMA_SEND_ENABLE(device_type type, u8 sendnum)
{
	DMA_Cmd(dma_channel_addr[type], DISABLE );  //关闭SPI 所指示的通道      
	
 	DMA_SetCurrDataCounter(dma_channel_addr[type],sendnum);//DMA通道的DMA缓存的大小,决定此次发送的数据个数
 	DMA_Cmd(dma_channel_addr[type], ENABLE);  //使能SPI 所指示的通道 
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
















