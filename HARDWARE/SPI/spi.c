#include "spi.h"
#include "dma.h"
#include "common.h"
#include "pbuf.h"
#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//SPI ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/6/13 
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
 

//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������SD Card/W25X16/24L01/JF24C							  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��
extern u8 spi1_getpack,spi2_getpack;
uint8_t spi1_getdata_count = 0,spi2_getdata_count = 0;
uint32_t tim3_count = 0;
SPI_InitTypeDef  SPI_InitStructure;
//SPI1 �� ���� ��ʼ������
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
  
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
	
	SPI_I2S_DeInit(SPI1); 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//���ݲ����ڵڶ���ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	
	/* Configure the Priority Group to 1 bit */                  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
    
  /* Configure the SPI interrupt priority */  
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
  NVIC_Init(&NVIC_InitStructure); 
	
	SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_RXNE,ENABLE);

	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����	 
	
//	DMA_Init_Config(spi1_rx);
//	DMA_Init_Config(spi1_tx);
// 	SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);       //??DMA??
} 
//SPI2�� ���� ��ʼ������
void SPI2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE );//SPI2ʱ��ʹ�� 
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //���츴��
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//ʵʼ���������
    GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15����

		SPI_I2S_DeInit(SPI2);  //������disable�����ܸı�MODE
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //˫���������ȫ˫��ģʽ
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //����ΪSPI������ģʽ
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;        //SPI���ݴ�С������8λ֡���ݽṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;        //�豸����״̬ʱͬ��ʱ��SCK��״̬��High��ʾ�ߵ�ƽ��Low��ʾ�͵�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //ʱ����λ��1��ʾ��ͬ��ʱ��SCK�������ر߲�����2��ʾż���ر߲���
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //NSS������ؼ�Ƭѡ
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_256;//ʱ�ӵ�Ԥ��Ƶֵ
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    //MSB��λ��ǰ
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC����͵Ķ���ʽ
		
		/* Configure the Priority Group to 1 bit */                  
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

		/* Configure the SPI interrupt priority */  
		NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;  
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
		NVIC_Init(&NVIC_InitStructure); 
		
//		SPI_I2S_ITConfig(SPI2,SPI_I2S_IT_RXNE,ENABLE);
		
    SPI_Init(SPI2, &SPI_InitStructure);  //��ʼ��SPI2��������
		SPI_Cmd(SPI2, ENABLE); //ʹ��SPI2   
		

//		DMA_Init_Config(spi2_tx);//��ʼ��SPI����DMA����
//		DMA_Init_Config(spi2_rx);//��ʼ��SPI����DMA����
//		
//		SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx,ENABLE);       //SPI DMA����    ���պͷ���
}   
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   (SPI 36M@sys 72M)
//SPI_BaudRatePrescaler_8   8��Ƶ   (SPI 9M@sys 72M)
//SPI_BaudRatePrescaler_16  16��Ƶ  (SPI 4.5M@sys 72M)
//SPI_BaudRatePrescaler_256 256��Ƶ (SPI 281.25K@sys 72M)

void SPI1_SetSpeed(SPI_TypeDef *spi,u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  SPI_Init(spi, &SPI_InitStructure);
	SPI_Cmd(spi,ENABLE);
}  

u8 SPI_ReadByte(SPI_TypeDef *spi)
{
	u8 retry=0;
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
	retry++;
	if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(spi); //����ͨ��SPIx������յ�����		
}
uint8_t SPI_WriteByte(SPI_TypeDef *spi,u8 data)
{
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(spi, data); //ͨ������SPIx����һ������
}
//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI_ReadWriteByte(SPI_TypeDef *spi, u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(spi, TxData); //ͨ������SPIx����һ������
	retry=0;
	while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(spi); //����ͨ��SPIx������յ�����					    
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
//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
//	static u16 i = 0;
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
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
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
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




