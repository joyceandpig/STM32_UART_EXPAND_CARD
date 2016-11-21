#include "usmart.h"
#include "stm32f10x.h"
#include "usmart.h"
#include "stdio.h"
#include "string.h"
#include "debug.h"  
////////////////////////////�û�������///////////////////////////////////////////////
//������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
#ifdef	DEBUG

#endif
extern void test_send_to_uart(void);
extern void reset_sys(void);
//�������б��ʼ��(�û��Լ����)
//�û�ֱ������������Ҫִ�еĺ�����������Ҵ�
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//���ʹ���˶�д����
	(void*)read_addr,"u32 read_addr(u32 addr)",
	(void*)write_addr,"void write_addr(u32 addr,u32 val)",		
#endif		
	(void*)reset_sys,"void reset_sys(void)",	
	(void*)test_send_to_uart,"void test_send_to_uart(void)",
#ifdef	DEBUG

	
#endif	
};	

#include "usart.h"
void USMART_Send_Data(u8 data)
{//��ֲʹ��
	usart_send(1,data);
}

const u16 USMART_RECV_LEN_MAX = 1024;
u8 USMART_RX_BUF[USMART_RECV_LEN_MAX];     //���ջ���,���USART_REC_LEN���ֽ�.


///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�������ƹ�������ʼ��
//�õ������ܿغ���������
//�õ�����������
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//��������
	0,	  	//��������
	0,	 	//����ID
	1,		//������ʾ����,0,10����;1,16����
	0,		//��������.bitx:,0,����;1,�ַ���	    
	0,	  	//ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		//�����Ĳ���,��ҪPARM_LEN��0��ʼ��
};   

