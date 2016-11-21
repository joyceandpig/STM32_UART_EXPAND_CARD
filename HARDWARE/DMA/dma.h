#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
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




