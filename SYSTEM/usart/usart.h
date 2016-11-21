#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	

extern USART_TypeDef* COM_USART[];
typedef enum
{
	com1 = 0x00,
	com2,
	com3,
	com4
}UartCom;

typedef struct 
{
	u8 buadrate;
	u8 preemptionpriority;
	u8 subpriority;
}UartPara;
extern UartPara uartpara[4];

#define USART1_TX_PORT    GPIOA
#define USART1_TX_PIN         GPIO_Pin_9
#define USART1_RX_PIN        GPIO_Pin_10
#define USART1_GPIO_CLK   RCC_APB2Periph_GPIOA
#define USART1_CLK              RCC_APB2Periph_USART1
#define USART1_PRE_PRIORITY      3
#define USART1_SUB_PRIORITY     3

#define USART2_TX_PORT    GPIOA
#define USART2_TX_PIN         GPIO_Pin_2
#define USART2_RX_PIN        GPIO_Pin_3
#define USART2_GPIO_CLK   RCC_APB2Periph_GPIOA
#define USART2_CLK              RCC_APB1Periph_USART2
#define USART2_PRE_PRIORITY      3
#define USART2_SUB_PRIORITY     3

#define USART3_TX_PORT    GPIOB
#define USART3_TX_PIN         GPIO_Pin_10
#define USART3_RX_PIN        GPIO_Pin_11
#define USART3_GPIO_CLK   RCC_APB2Periph_GPIOB
#define USART3_CLK              RCC_APB1Periph_USART3
#define USART3_PRE_PRIORITY      3
#define USART3_SUB_PRIORITY     3

#define USART4_TX_PORT    GPIOC
#define USART4_TX_PIN         GPIO_Pin_10
#define USART4_RX_PIN        GPIO_Pin_11
#define USART4_GPIO_CLK   RCC_APB2Periph_GPIOC
#define USART4_CLK              RCC_APB1Periph_UART4
#define USART4_PRE_PRIORITY      3
#define USART4_SUB_PRIORITY     3

//����봮���жϽ��գ��벻Ҫע�����º궨��

void usart_send(uint8_t com,u8 data);
#endif


