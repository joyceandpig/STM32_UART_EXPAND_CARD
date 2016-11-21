#include "sys.h"
#include "usart.h"	  
#include "common.h"
#include "dma.h"
#include "pbuf.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
UartPara uartpara[4];
 
USART_TypeDef* COM_USART[]       = {USART1,          USART2,          USART3,          UART4}; 

GPIO_TypeDef* COM_TX_PORT[]      = {USART1_TX_PORT,  USART2_TX_PORT,  USART3_TX_PORT,  USART4_TX_PORT};
 
GPIO_TypeDef* COM_RX_PORT[]      = {USART1_TX_PORT,  USART2_TX_PORT,  USART3_TX_PORT,  USART4_TX_PORT};

const uint32_t COM_USART_CLK[]   = {USART1_CLK,      USART2_CLK,      USART3_CLK,      USART4_CLK};

const uint32_t COM_TX_PORT_CLK[] = {USART1_GPIO_CLK, USART2_GPIO_CLK, USART3_GPIO_CLK, USART4_GPIO_CLK};
 
const uint32_t COM_RX_PORT_CLK[] = {USART1_GPIO_CLK, USART2_GPIO_CLK, USART3_GPIO_CLK, USART4_GPIO_CLK};

const uint16_t COM_TX_PIN[]      = {USART1_TX_PIN,   USART2_TX_PIN,   USART3_TX_PIN,   USART4_TX_PIN};

const uint16_t COM_RX_PIN[]      = {USART1_RX_PIN,   USART2_RX_PIN,   USART3_RX_PIN,   USART4_RX_PIN};

const uint16_t COM_IRQn[]        = {USART1_IRQn,     USART2_IRQn,     USART3_IRQn,     UART4_IRQn};

const uint16_t COM_PRIORITY[][2] = {{USART1_PRE_PRIORITY, USART1_SUB_PRIORITY},
																		{USART2_PRE_PRIORITY, USART2_SUB_PRIORITY},
																		{USART3_PRE_PRIORITY, USART3_SUB_PRIORITY},
																		{USART4_PRE_PRIORITY, USART4_SUB_PRIORITY}}; 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

void usart_send(uint8_t com,u8 data)
{
    USART_SendData(COM_USART[com],data);
    while(USART_GetFlagStatus(COM_USART[com], USART_FLAG_TXE) == RESET);//等待发送完成   
}
void usart_sendstring(uint8_t com,u8 *dat, u8 sendlen)
{
	while(sendlen--)
	{
		usart_send(com,*dat++);
	}
}
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

static void uart_gpio_init(__IO UartCom COM)
{
	uint8_t res;
  /* Enable GPIO clock */	   
  RCC_APB2PeriphClockCmd( COM_TX_PORT_CLK[COM] | \
													COM_RX_PORT_CLK[COM] | \
													RCC_APB2Periph_AFIO, ENABLE);
	res = (COM < 0x01)?1:0;
  if(res){/* Enable UART clock */
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); 
  }else{
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	}
	
	USART_DeInit(COM_USART[COM]);

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;    //复用功能的推挽输出_AF_PP ——片内外设功能
  GPIO_InitStructure.GPIO_Pin   = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin  = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
}
static void uart_nvic_config(UartCom com,u8 preemptionpriority,u8 subpriority)
{
	  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = COM_IRQn[com];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=preemptionpriority ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subpriority;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}
static void uart_paramer_config(UartCom com, u32 boundrate)
{
	   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = boundrate;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	
//	 USART_Init(COM_USART[com1], &USART_InitStructure);
}
static void uart_dma_config(__IO UartCom com)
{
	switch(com)
	{
		case com1:
			DMA_Init_Config(uart1_rx);
			DMA_Init_Config(uart1_tx);
		break;
		case com2:
			DMA_Init_Config(uart2_rx);
			DMA_Init_Config(uart2_tx);
		break;
		case com3:
			DMA_Init_Config(uart3_rx);
			DMA_Init_Config(uart1_tx);
		break;
		case com4:
			DMA_Init_Config(uart4_rx);
			DMA_Init_Config(uart4_tx);
		break;
	}
}
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
//串口1挂在APB2上面，usart2，usart3，uart4,uart5挂在APB1总线上面
//Usart1初始化
void uart_init(__IO UartCom com, u32 bound,u8 preemptionpriority,u8 subpriority)
{
	uart_gpio_init(com);
	uart_nvic_config(com,preemptionpriority,subpriority);
	
	if(bound == USART_BAUDRATE_9600){
		bound = 9600;
	}else if(bound == USART_BAUDRATE_14400){
		bound = 14400;
	}else if(bound == USART_BAUDRATE_19200){
		bound = 19200;
	}else if(bound == USART_BAUDRATE_38400){
		bound = 38400;
	}else if(bound == USART_BAUDRATE_56000){
		bound = 56000;
	}else if(bound == USART_BAUDRATE_57600){
		bound = 576000;
	}else if(bound == USART_BAUDRATE_115200){
		bound = 115200;
	}else if(bound == USART_BAUDRATE_194000){
		bound = 194000;
	}
	
	uart_paramer_config(com,bound);
	uart_dma_config(com);

	USART_Init(COM_USART[com], &USART_InitStructure);
	USART_ITConfig(COM_USART[com], USART_IT_IDLE, ENABLE);//使能接收中断
	USART_Cmd(COM_USART[com], ENABLE);
	
	while (USART_GetFlagStatus(COM_USART[com], USART_FLAG_TC) == RESET);/*等待 发送区清空*/
}
void uart_config(void)
{
	uint8_t i;
	uart_init(com1,USART_BAUDRATE_115200,COM_PRIORITY[com1][0],COM_PRIORITY[com1][1]);
	uart_init(com2,USART_BAUDRATE_115200,COM_PRIORITY[com2][0],COM_PRIORITY[com2][1]);
	uart_init(com3,USART_BAUDRATE_115200,COM_PRIORITY[com3][0],COM_PRIORITY[com3][1]);
	uart_init(com4,USART_BAUDRATE_115200,COM_PRIORITY[com4][0],COM_PRIORITY[com4][1]);
	
	for(i = 0; i< 4; i++)
	{
		uartpara[i].buadrate = USART_BAUDRATE_115200;
		uartpara[i].preemptionpriority = COM_PRIORITY[i][0];
		uartpara[i].subpriority = COM_PRIORITY[i][1];
	}
}

void uart_dma_send_data(UartCom com,u8 len)
{
	MY_UART_DMA_SEND_ENABLE( (device_type)com*2,len);
}

u16 pre_remain_count = MEMP_BUF_LAYER_TYPE_SIZE,cur_count = MEMP_BUF_LAYER_TYPE_SIZE;
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(UART1_RX_DMA_CHANNEL,DISABLE);
		USART1->SR;
		USART1->DR;

		cur_count = DMA_GetCurrDataCounter(UART1_RX_DMA_CHANNEL);
		
		cir_buf[uart1_rx]->front = (u32*)((MEMP_BUF_LAYER_TYPE_SIZE-cur_count)+(u32)(cir_buf[uart1_rx]->front));
		pre_remain_count = DMA_GetCurrDataCounter(UART1_RX_DMA_CHANNEL);
		
		cir_buf[uart1_rx]->size = ((u32)(cir_buf[uart1_rx]->front)- (u32)(cir_buf[uart1_rx]->rear) + MEMP_BUF_LAYER_TYPE_SIZE)%MEMP_BUF_LAYER_TYPE_SIZE;
		
		InsertLink(QLinkList[uart1_rx],cir_buf[uart1_rx]->size,(u32)cir_buf[uart1_rx]->rear);
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);//空闲
		received_buffer_flag |= 0x8;
		DMA_Cmd(UART1_RX_DMA_CHANNEL,ENABLE);
	}
} 


void USART2_IRQHandler(void) 
{
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(UART2_RX_DMA_CHANNEL,DISABLE);
		USART2->SR;
		USART2->DR;

		cur_count = DMA_GetCurrDataCounter(UART2_RX_DMA_CHANNEL);
		
		cir_buf[com2]->front = (uint32_t *)(( *cir_buf[com2]->front + 512 - cur_count)%(* cir_buf[com2]->queue_buf));
		pre_remain_count = DMA_GetCurrDataCounter(UART2_RX_DMA_CHANNEL);
		
		cir_buf[com2]->size = (*(cir_buf[com2]->front)- *(cir_buf[com2]->rear) + 512)%512;
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);//空闲
		received_buffer_flag |= 0x4;
		DMA_Cmd(UART2_RX_DMA_CHANNEL,ENABLE);
	}
}
void USART3_IRQHandler(void) 
{
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(UART3_RX_DMA_CHANNEL,DISABLE);
		USART3->SR;
		USART3->DR;

		cur_count = DMA_GetCurrDataCounter(UART3_RX_DMA_CHANNEL);
		
		cir_buf[com3]->front = (uint32_t *)(( *cir_buf[com3]->front + 512 - cur_count)%(* cir_buf[com3]->queue_buf));
		pre_remain_count = DMA_GetCurrDataCounter(UART3_RX_DMA_CHANNEL);
		
		cir_buf[com3]->size = (*(cir_buf[com3]->front)- *(cir_buf[com3]->rear) + 512)%512;
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);//空闲
		received_buffer_flag |= 0x2;
		DMA_Cmd(UART3_RX_DMA_CHANNEL,ENABLE);
	}
}
void USART4_IRQHandler(void) 
{
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(UART4_RX_DMA_CHANNEL,DISABLE);
		UART4->SR;
		UART4->DR;

		cur_count = DMA_GetCurrDataCounter(UART4_RX_DMA_CHANNEL);
		
		cir_buf[com4]->front = (uint32_t *)(( *cir_buf[com4]->front + 512 - cur_count)%(* cir_buf[com4]->queue_buf));
		pre_remain_count = DMA_GetCurrDataCounter(UART4_RX_DMA_CHANNEL);
		
		cir_buf[com4]->size = (*(cir_buf[com4]->front)- *(cir_buf[com4]->rear) + 512)%512;
		USART_ClearITPendingBit(UART4, USART_IT_IDLE);//空闲
		received_buffer_flag |= 0x1;
		DMA_Cmd(UART4_RX_DMA_CHANNEL,ENABLE);
	}
}

