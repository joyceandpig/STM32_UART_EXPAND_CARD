#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
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
#define UART1_TX_DMA              1
#define UART1_RX_DMA             1
#define UART2_TX_DMA               1
#define UART2_RX_DMA              1
#define UART3_TX_DMA               1
#define UART3_RX_DMA                1
#define UART4_TX_DMA              1
#define UART4_RX_DMA               1
#define SPI1_TX_DMA	                 1
#define SPI1_RX_DMA	                 1
#define SPI2_TX_DMA			                1
#define SPI2_RX_DMA		                 1

#define SPI1_TX_DMA_CHANNEL 					DMA1_Channel3
#define SPI1_RX_DMA_CHANNEL 					DMA1_Channel2

#define SPI2_TX_DMA_CHANNEL 					DMA1_Channel5
#define SPI2_RX_DMA_CHANNEL 					DMA1_Channel4

#define UART1_TX_DMA_CHANNEL 					DMA1_Channel4
#define UART1_RX_DMA_CHANNEL 					DMA1_Channel5

#define UART2_TX_DMA_CHANNEL 					DMA1_Channel7
#define UART2_RX_DMA_CHANNEL 					DMA1_Channel6

#define UART3_TX_DMA_CHANNEL 					DMA1_Channel2
#define UART3_RX_DMA_CHANNEL 					DMA1_Channel3

#define UART4_TX_DMA_CHANNEL 					DMA2_Channel5
#define UART4_RX_DMA_CHANNEL 					DMA2_Channel3




u8 DMA_Init_Config( uint8_t type );

		   
#endif




