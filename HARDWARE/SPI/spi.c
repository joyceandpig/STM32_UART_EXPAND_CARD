#include "spi.h"
#include "dma.h"
#include "common.h"
#include "pbuf.h"
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
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//���ݲ����ڵڶ���ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
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
//SPI2�� �ӻ� ��ʼ������
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
    //GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15����
    /*����SPI_NRF_SPI��ƬѡCS����NSS GPIOB^12*/
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//ͨ���������
    //GPIO_Init(GPIOB, &GPIO_InitStructure); 
		SPI_I2S_DeInit(SPI2);  //������disable�����ܸı�MODE
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //˫���������ȫ˫��ģʽ
    SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;        //����ΪSPI������ģʽ
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;        //SPI���ݴ�С������8λ֡���ݽṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;        //�豸����״̬ʱͬ��ʱ��SCK��״̬��High��ʾ�ߵ�ƽ��Low��ʾ�͵�ƽ
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //ʱ����λ��1��ʾ��ͬ��ʱ��SCK�������ر߲�����2��ʾż���ر߲���
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //NSS������ؼ�Ƭѡ
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//ʱ�ӵ�Ԥ��Ƶֵ
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
		
		SPI_I2S_ITConfig(SPI2,SPI_I2S_IT_RXNE,ENABLE);
		
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

void SPI1_SetSpeed(u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1,ENABLE);
}  
//SPI2�ٶ����ú���
void SPI2_SetSpeed(u8 SpeedSet)
{
	SPI_InitStructure.SPI_BaudRatePrescaler = SpeedSet ;
  SPI_Init(SPI2, &SPI_InitStructure);
	SPI_Cmd(SPI2,ENABLE);
} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����					    
}

u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}

void SPI_SendRec_Data(SPI_TypeDef *spi ,u8 data_num)
{
//	spi->DR;
//	while((spi->CR2 & SPI_I2S_FLAG_TXE) == 0);
//	if( spi == SPI1 ){
//		DMA_SetCurrDataCounter(SPI1_TX_DMA_CHANNEL,data_num);
//		DMA_SetCurrDataCounter(SPI1_RX_DMA_CHANNEL,data_num);
//		DMA_Cmd(SPI1_TX_DMA_CHANNEL,ENABLE);
//		DMA_Cmd(SPI1_RX_DMA_CHANNEL,ENABLE);
//		DMA_ITConfig(SPI1_TX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
//		DMA_ITConfig(SPI1_RX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
//	}else {
//		DMA_SetCurrDataCounter(SPI2_TX_DMA_CHANNEL,data_num);
//		DMA_SetCurrDataCounter(SPI2_RX_DMA_CHANNEL,data_num);
//		DMA_Cmd(SPI2_TX_DMA_CHANNEL,ENABLE);
//		DMA_Cmd(SPI2_RX_DMA_CHANNEL,ENABLE);
//		DMA_ITConfig(SPI2_TX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
//		DMA_ITConfig(SPI2_RX_DMA_CHANNEL,DMA_IT_TC,ENABLE);
//	}
	uint16_t len = data_num;
	uint8_t  dat;
	spi->DR;
	while((spi->CR2 & SPI_I2S_FLAG_TXE) == 0);
	if( spi == SPI1 ){
		while(len--){
			dat = DeQueue(cir_buf[spi1_tx]);
			SPI1_ReadWriteByte(dat);
		}
	}else {
		while(len--){
			dat = DeQueue(cir_buf[spi2_tx]);
			SPI2_ReadWriteByte(dat);
		}
	}
}

void SPI1_IRQHandler(void)
{         
	static u8 dat;
	static uint16_t dat_rec_count = 0;
	if(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==SET){         
		dat = SPI_I2S_ReceiveData(SPI1);
		EnQueue(cir_buf[spi1_rx],dat);
#if FRAME_START_SIGN_EN
		if(*(cir_buf[spi2_rx]->front-1) == FRAME_START_SIGN2 && *(cir_buf[spi2_rx]->front-2) == FRAME_START_SIGN1) 
		{
			DeQueue(cir_buf[spi1_rx]);
#endif
			dat_rec_count++;
			if(*cir_buf[spi1_rx]->rear == 0x0D && *(cir_buf[spi1_rx]->rear-1) == 0x0A){
			received_buffer_flag |= 0x20;
			DeQueue(cir_buf[spi1_rx]);
			DeQueue(cir_buf[spi1_rx]);
			InsertLink(QLinkList[spi1_rx],dat_rec_count-2,cir_buf[spi1_rx]->front);
			dat_rec_count = 0;
		}
#if FRAME_START_SIGN_EN
		}
#endif
	}    
}
void SPI2_IRQHandler(void)
{         
	static u8 dat;
	static uint16_t dat_send_count = 0;
	if(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==SET){         
		dat = SPI_I2S_ReceiveData(SPI2);

		EnQueue(cir_buf[spi2_rx],dat);
#if FRAME_START_SIGN_EN
		if(*(cir_buf[spi2_rx]->front-1) == FRAME_START_SIGN2 && *(cir_buf[spi2_rx]->front-2) == FRAME_START_SIGN1) 
		{
			DeQueue(cir_buf[spi2_rx]);
#endif
			dat_send_count++;
			if(*(cir_buf[spi2_rx]->front-1) == 0x0D && *(cir_buf[spi2_rx]->front-2) == 0x0A){
				received_buffer_flag |= 0x10;
				DeQueue(cir_buf[spi2_rx]);
				DeQueue(cir_buf[spi2_rx]);
				InsertLink(QLinkList[spi2_rx],dat_send_count-2,cir_buf[spi2_rx]->front);
				dat_send_count = 0;
			}
#if FRAME_START_SIGN_EN
		}
#endif
	}    
}





























