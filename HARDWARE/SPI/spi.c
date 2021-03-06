#include "spi.h"
#include "dma.h"
#include "common.h"
#include "pbuf.h"
#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//SPI 驱动函数	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/13 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
 

//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25X16/24L01/JF24C							  
//SPI口初始化
//这里针是对SPI1的初始化
extern u8 spi1_getpack,spi2_getpack;
uint8_t spi1_getdata_count = 0,spi2_getdata_count = 0;
uint32_t tim3_count = 0;
SPI_InitTypeDef  SPI_InitStructure;
//SPI1 做 主机 初始化配置
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
	
	SPI_I2S_DeInit(SPI1); 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//选择了串行时钟的稳态:时钟悬空高
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//数据捕获于第二个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	
	/* Configure the Priority Group to 1 bit */                  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
    
  /* Configure the SPI interrupt priority */  
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure); 
	
	SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_RXNE,ENABLE);

	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设	 
	
//	DMA_Init_Config(spi1_rx);
//	DMA_Init_Config(spi1_tx);
// 	SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);       //??DMA??
} 
//SPI2做 主机 初始化配置
void SPI2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //推挽复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//实始化相关引脚
    GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15上拉

		SPI_I2S_DeInit(SPI2);  //必须先disable，才能改变MODE
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //双线输入输出全双工模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //设置为SPI的主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;        //SPI数据大小：发送8位帧数据结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;        //设备空闲状态时同步时钟SCK的状态，High表示高电平，Low表示低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //时钟相位，1表示在同步时钟SCK的奇数沿边采样，2表示偶数沿边采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //NSS由软件控件片选
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_256;//时钟的预分频值
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    //MSB高位在前
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC较验和的多项式
		
		/* Configure the Priority Group to 1 bit */                  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

		/* Configure the SPI interrupt priority */  
		NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;  
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
		NVIC_Init(&NVIC_InitStructure); 
		
//		SPI_I2S_ITConfig(SPI2,SPI_I2S_IT_RXNE,ENABLE);
		
    SPI_Init(SPI2, &SPI_InitStructure);  //初始化SPI2的配置项
		SPI_Cmd(SPI2, ENABLE); //使能SPI2   
		

//		DMA_Init_Config(spi2_tx);//初始化SPI接收DMA参数
//		DMA_Init_Config(spi2_rx);//初始化SPI发送DMA参数
//		
//		SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx,ENABLE);       //SPI DMA传输    接收和发送
}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8分频   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16分频  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256分频 (SPI 281.25K@sys 72M)

void SPI1_SetSpeed(SPI_TypeDef *spi,u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  SPI_Init(spi, &SPI_InitStructure);
	SPI_Cmd(spi,ENABLE);
}  

u8 SPI_ReadByte(SPI_TypeDef *spi)
{
	u8 retry=0;
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
	retry++;
	if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(spi); //返回通过SPIx最近接收的数据		
}
uint8_t SPI_WriteByte(SPI_TypeDef *spi,u8 data)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(spi, data); //通过外设SPIx发送一个数据
}
//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI_ReadWriteByte(SPI_TypeDef *spi, u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(spi, TxData); //通过外设SPIx发送一个数据
	retry=0;
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(spi); //返回通过SPIx最近接收的数据					    
}

void SPI_SendRec_Data(SPI_TypeDef *spi ,u8 data_num)
{
	uint16_t len = data_num;
	uint8_t  dat;
	uint8_t device = 0;
	
	if(spi == SPI1){
		device = spi1_tx;
	}else if(spi == SPI2){
		device = spi2_tx;
	}
	while(len--){
		dat = DeQueue(cir_buf[device]);
		SPI_ReadWriteByte(spi,dat);
//		printf("spi send the %d byte %x of total %d data\r\n",data_num-len,dat,data_num);
	}

}
//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
//	static u16 i = 0;
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		if(tim3_count++> 50)
		{
//			printf("spi1 get pack, total num: %d byte\r\n",spi1_getdata_count);
//			for(;i< spi1_getdata_count;i++){
//				printf("the %d rec data is: %x\r\n",i,(u8)(*(u32 *)((u32)(cir_buf[spi1_rx]->rear)+i)));
//			}
//			i = 0;
			InsertLink(QLinkList[spi1_rx],spi1_getdata_count,cir_buf[spi1_rx]->rear);
			spi1_getdata_count = 0;
			TIM_Cmd(TIM3,DISABLE);
			tim3_count = 0;
		}
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}
#include "led.h"
void SPI1_IRQHandler(void)
{         
	static u8 dat;
	LED1 = 0;
	if(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==SET){
		
		TIM_Cmd(TIM3,DISABLE);
		TIM_SetCounter(TIM3,0);
		TIM_Cmd(TIM3,ENABLE);
		
//		SPI_ReadWriteByte(SPI1,0xFF);
		dat = SPI_ReadByte(SPI1);
//		printf("spi1 rec data is: %x\r\n",dat);
		EnQueue(cir_buf[spi1_rx],dat);
		spi1_getdata_count++;

	}   
	LED1 = 1;	
}
void SPI2_IRQHandler(void)
{         
	static u8 dat;
	static uint16_t dat_send_count = 0;
	if(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==SET){         
		dat = SPI_ReadByte(SPI2);

		EnQueue(cir_buf[spi2_rx],dat);
		InsertLink(QLinkList[spi2_rx],dat_send_count,cir_buf[spi2_rx]->front);
	}    
}




