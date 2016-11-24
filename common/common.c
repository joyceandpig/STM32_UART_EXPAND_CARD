#include "common.h"
#include "memp.h"
#include "malloc.h"
#include "pbuf.h"
#include "usart.h"
#include "spi.h"
#include "dma.h"

uint8_t received_buffer_flag = 0;
uint8_t sending_buffer_flag = 0;
uint8_t StartSPI2RecData_flag = 0;

u8 spi1_getpack = 0,spi2_getpack = 0;
extern u32 memp_get_memorysize(void);	//��memp.c���涨��
extern u8 *memp_memory;				//��memp.c���涨��.
extern USART_TypeDef* COM_USART[];
extern USART_InitTypeDef USART_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;

SqQueue *cir_buf[MAX_BUF_NUM];
QueueLinkList QLinkList[MAX_BUF_NUM] = {0};

static uint8_t protocal_version = 0;
static uint8_t expand = 0;

struct pack_head *take_head = {0};

static uint8_t query_baud = 0,query_priro = 0, query_subpriro = 0;
uint8_t spi1_feedback_len = 0;

//input��none
//output��none
//descript���ڴ��ͷ�
void comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
}


//input��none
//output��none
//�ڴ�����
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	
	mempsize=memp_get_memorysize();			//�õ�memp_memory�����С
	memp_memory=mymalloc(SRAMIN,mempsize);	//Ϊmemp_memory�����ڴ�
//	printf("memp_memory�ڴ��СΪ:%d\r\n",mempsize);
	if(!memp_memory){	//������ʧ�ܵ�
		comm_mem_free();
		return 1;
	}
	return 0;	
}
//input��buffer��������
//output��none
//descript����ʼ�����������ͷ���
void qlink_init(QueueLinkList *L)
{
	(*L) = (QueueLink *)mymalloc(SRAMIN,sizeof(QueueLink));
	(*L)->next = NULL;
	(*L)->size = 0;
}
//input�����λ���buffer
//output��none
//descript�����λ���buffer�ĳ�ʼ��
void sq_init(Queue *L)
{
	(*L) = (SqQueue *)mymalloc(SRAMIN,sizeof(SqQueue));
}
SqQueue *QueueNew(void)
{
	SqQueue *p = NULL;
	p = (SqQueue *)mymalloc(SRAMIN,sizeof(SqQueue));
	p->size = 0;
	p->front = NULL;
	p->rear = NULL;
	p->queue_buf = NULL;
	return p;
}
void QueueAlloc(SqQueue *Q, uint8_t num)
{
	uint32_t *p = NULL;
	p =(uint32_t *)drive_queue_alloc(MEMP_uart1_pool+num/2);
	Q->queue_buf = p;
	Q->front = p;
	Q->rear = p;
}
void SqQueueInit(void)
{
	uint8_t i = 0;
	for(; i < MAX_BUF_NUM; i++){
		cir_buf[i] = 	QueueNew();
		QueueAlloc(cir_buf[i],i);
	}
}
struct queuelink *LinkListNew(void)
{
	struct queuelink *p;
	p = (QueueLink *)mymalloc(SRAMIN,sizeof(QueueLink));
	p->next = NULL;
	p->size = 0;
	p->start_postion = 0;
	return p;
}
void LinkListInit(void)
{
	uint8_t i = 0;
	for(; i < MAX_BUF_NUM; i++){
		QLinkList[i] = 	LinkListNew();
	}
}
//input��none
//output��none
//descript�����λ���buffer��buffer���������ʼ��
/**********************************���λ�����г�ʼ�������������ʼ��************************/
void CirQueue_LinkList_init(void)
{
	SqQueueInit();
	LinkListInit();
	
//	u8 i = 0;
//	for(; i < MAX_BUF_NUM;i++){
//		sq_init(&cir_buf[i]);
//		qlink_init(&QLinkList[i]);
//		
//		cir_buf[i]->size = 0;
//		cir_buf[i]->queue_buf = (uint32_t *)drive_queue_alloc(MEMP_uart1_pool+i/2);
//		(cir_buf[i]->front) = cir_buf[i]->queue_buf ;
//		(cir_buf[i]->rear) = cir_buf[i]->queue_buf ;
//		
//		QLinkList[i]->next = NULL;
//		QLinkList[i]->size = 0;
//		QLinkList[i]->start_postion = 0;
//	}
}
//input�����������������β������ݵĳ��ȣ���������buffer�е���ʼλ��
//output��success or faild
//descript������buffer��¼����������
/***********************************���λ�����м�¼����������***********************/
ErrorStatus InsertLink(QueueLinkList linklist,uint16_t len, uint32_t start_pos)
{
	uint16_t i;
	QueueLinkList p = linklist,q;
	if(len == 0)return Failture;
	
	while(p->next)
	{
		p = p->next;
	}
//if(!p)return ERROR;
	
	q = (QueueLinkList)malloc(sizeof(QueueLink));
	q->size = len;
	q->start_postion = start_pos;
	
	q->next = p->next;
	p->next = q;
	
	return SUCCESS;
}
//input��buffer��¼������
//output��success or faild
//descript���Ӽ�¼������ɾ��һ���ڵ���Ϣ
/*************************************���λ�����м�¼����ɾ������*********************/
ErrorStatus DeleteLink(QueueLinkList linklist)
{
	QueueLinkList p = linklist->next,q = linklist;
	
	if(!q->next)return ERROR;
	q->next = p->next;
	free(p);
	return SUCCESS;
}
//input����¼������
//output��������-�ڵ����
//descript���õ�������
/**************************************������¼�����õ�������********************/
uint16_t TravalLinkListGetLength(QueueLinkList linklist)
{
	QueueLinkList p = linklist;
	uint16_t len = 0;
	while(p->next)
	{
		p = p->next;
		len++;
	}
	return len;
}
//input�����ζ�����
//output��true�������Ѿ����ˣ�false������δ��
//descript���ж������Ƿ�Ϊ������
/******************************�ж������Ƿ�Ϊ������******************************/
static uint8_t IsFull(SqQueue *Q)
{
	return (Q->size >= MEMP_BUF_LAYER_TYPE_SIZE)?TRUE:FALSE;
}

//input�����ζ�����
//output��true�������Ѿ����ˣ�false������δ��
//descript���ж������Ƿ�Ϊ�ն���
/******************************�ж������Ƿ�Ϊ�ն���*****************************/
static uint8_t IsEmpty(SqQueue *Q)
{
	return (Q->size <= 0)?TRUE:FALSE;
}

//input��Q�����ζ�������data�����β��뻷�ζ��е�����
//output��none
//descript���ӻ���buffer�ｫ���ݿ�����pbuf�ṹ���������ݲ������
/*********************************���ζ��� ����в���****************************/
uint8_t EnQueue(SqQueue *Q, uint8_t data)
{
	if (IsFull(Q))
		return FALSE;
	*Q->front = data;
	Q->size++;
	Q->front = (u32 *)((u32)Q->queue_buf+(((u32)Q->front)+1- (u32)Q->queue_buf)%MEMP_BUF_LAYER_TYPE_SIZE);
	return TRUE;
}
//input�����ζ�����
//output��dat���˴δӻ��ζ�����ȡ��������
//descript���ӻ��ζ���ȡ��һ�����ݣ�����βָ���ƶ�һλ����buffer��С��1
/**********************************���ζ��� ���Ӳ���************************/
uint8_t DeQueue(SqQueue *Q)
{
	u8 dat;
	if (IsEmpty(Q))
		return FALSE;
	dat = *Q->rear;
	Q->rear = (u32 *)((u32)Q->queue_buf+(((u32)Q->rear)+1- (u32)Q->queue_buf)%MEMP_BUF_LAYER_TYPE_SIZE);
	Q->size -= 1;
	return dat;
}
//input�����ζ�����
//output��none
//descript����ָ������������գ�buffer��С��0
/*********************************��ն��в���***********************************/
void ClearOfQueue(SqQueue *Q)
{
	memset(Q->queue_buf,'\0',MEMP_BUF_LAYER_TYPE_SIZE);
	Q->front = Q->rear = Q->queue_buf;
	Q->size = 0;
}
//input�����ζ�����
//output��none
//descript���ӻ��ζ������Ƴ�һ��ָ�����͵����ݰ�
/*********************************�ӻ��λ���������Ƴ�һ�����ݰ�****************/
void RemovePackOfQueue(SqQueue *Q)
{
	Q->front = Q->rear;
	Q->size = 0;
}
//input�����ζ�����
//output��ָ�������е����ݳ���
//descript����ȡ�����е����ݳ���
/******************************��ȡ���ζ��������ݳ���***************************/
uint8_t GetLengthOfQueue(SqQueue *Q)
{
	return (*Q->front-*Q->rear + MEMP_BUF_LAYER_TYPE_SIZE)%MEMP_BUF_LAYER_TYPE_SIZE;
}

//input��pbuf�ṹ���ݰ���ַ
//output��none
//descript����pbuf�ṹ���ݴ�spi2�ӿڸ��Ƶ�spi1���ͻ�����
/*********************************SPI2�ӿ������͵�SPI1������****************************/
void spi2_to_spi1(struct pbuf *judge_p)
{
	uint16_t i,len = judge_p->head.framelength;
	uint8_t *p = (uint8_t *)judge_p->payload;
	
	InsertLink(QLinkList[spi1_tx],len,*cir_buf[spi1_tx]->front);
	for(i = 0;i < len;i++){
		EnQueue(cir_buf[spi1_tx],*p++);
	}
	pbuf_free(judge_p);
}
//input��dev��spi�˿ںţ�spi1����spi2
//       sendlen�����η��͵����ݳ���
//output��none
//descript����ָ����spi����ָ����������
/***********************************SPI��������**************************/
#include "delay.h"
void spi_send(device_type dev, uint16_t sendlen)
{
	if(dev == spi1_tx){
		GPIO_SetBits(GPIOA,GPIO_Pin_12);//���������ж��ߣ��������������ݻش���ʹ�����ṩʱ��
		spi1_feedback_len = sendlen;
		SPI2_WriteByte(SPI1,spi1_feedback_len);
//		SPI2_WriteByte(SPI1,sendlen);
//		SPI_SendRec_Data(SPI1,sendlen);
		GPIO_ResetBits(GPIOA,GPIO_Pin_12);//���������ж��ߣ������������ݴ�����ɣ�ʹ�����Ͽ�ʱ��
	}else if(dev == spi2_tx){
		SPI_SendRec_Data(SPI2,sendlen);
	}
}
//input��pbuf�ṹ���ݵ�ַ
//output��none
//descript����pbuf�ṹ���ݴ�spi1���Ƶ�spi2�Ļ�����
/***********************************SPI1�����͵�SPI2������**************************/
void spi1_to_spi2(struct pbuf *judge_p)
{	
	uint16_t i,len = judge_p->head.framelength;
	uint8_t *p = (uint8_t *)((u32)judge_p+4);

	for(i = 0;i < 8;i++){
		EnQueue(cir_buf[spi2_tx],*p++);
	}
		p = p+4;
	for(i = 0; i < len - 8;i++){
		EnQueue(cir_buf[spi2_tx],*p++);
	}
	InsertLink(QLinkList[spi2_tx],len,(u32)cir_buf[spi2_tx]->rear);
	
	pbuf_free(judge_p);
}
//input��dev���������ݵ����ö���buffer�ţ�ȡֵΪ6-9��
//       judge_p��pbuf�ṹ���ݵĵ�ַ
//output��none
//descript����spi���ݷ�����ָ�����ڵĻ�����
/**************************************SPI��������������***********************/
void spi_to_uart(device_type dev,struct pbuf *judge_p)
{
	uint16_t i,len = judge_p->head.framelength-8;
	uint8_t *p = (uint8_t *)judge_p->payload;
	
	InsertLink(QLinkList[dev],len,(u32)cir_buf[dev]->front);
	
	for(i = 0;i < len;i++){
		EnQueue(cir_buf[dev],*p++);
	}

	pbuf_free(judge_p);
}
//input��head�����ݰ�����й̶�ͷ�ṹ
//output��none
//descript����ͷ�ṹ��Ա����
void reset_head(struct pack_head *head)
{
	struct pack_head *p = head;
	p->framelength       = 0;
	p->direct            = 0;
	p->DevicePort 			 = 0;
	p->Config 					 = 0;
	p->DestinationCount  = 0;
	p->Expand 					 = expand;
	p->ProtocalVersion   = protocal_version;
}
//input��dev���������ݽ�����Դ�˿ڣ�buf��pbuf�ṹ��ַ������Ҫ��buf��Ա���б仯���ʴ���˫��ָ��
//output���������ݰ���Ҫ���õĶ˿ں�
//descript�����ݽ��գ���buffer������pbuf�ṹ��������pbuf������䡣
/****************************************���ݽ����ж��봦��*********************/
device_type DataRec(device_type dev, struct pbuf **buf)
{
	u32 *p = NULL;
	device_type dev_port_detination;
	
	(*buf) = (struct pbuf *)pbuf_alloc(512);//����һ���ڴ�ռ�
	(*buf)->next = NULL;
	*((*buf)->payload) = ((u32)(*buf)+SIZEOF_STRUCT_PACKHEAD+4);
	p = (u32 *)*buf;
	if(dev < spi1_rx){
		take_head = (struct pack_head*)mymalloc(SRAMIN,SIZEOF_STRUCT_PACKHEAD);
		reset_head(take_head);
		take_head->DevicePort = dev;
		take_head->direct = 1;
		take_head->framelength = QLinkList[dev]->next->size + SIZEOF_STRUCT_PACKHEAD;
		pbuf_encode((*buf),(u32*)QLinkList[dev]->next->start_postion,QLinkList[dev]->next->size,take_head);
		myfree(SRAMIN ,take_head);
		dev_port_detination = spi1_tx;
	}else{
		memcpy((u32*)&(*buf)->head,(u32*)QLinkList[dev]->next->start_postion,SIZEOF_STRUCT_PACKHEAD);//QLinkList[dev]->next->size);//�����ջ������ݿ������½��ڴ�ռ�
		memcpy((*buf)->payload,(u32*)(QLinkList[dev]->next->start_postion+SIZEOF_STRUCT_PACKHEAD),QLinkList[dev]->next->size-SIZEOF_STRUCT_PACKHEAD);
		dev_port_detination = (*buf)->head.DevicePort+uart1_tx-1;
		(*buf)->head.framelength = QLinkList[dev]->next->size;
	}
	ClearOfQueue(cir_buf[dev]);//��������н��յ�����
	DeleteLink(QLinkList[dev]);
	
	if(QLinkList[dev]->next == NULL){
		received_buffer_flag &= ~0x20;//���spi1��rx���ձ�־λ
	}
	return dev_port_detination;
}
//input��dev��pbuf�ṹ��ָ�������ݶ˿�
//       judge_p��pbuf�ṹ���ݰ���ַ
//output���ж����
//descript���Խ��յ������ݻ������ж���ȷ��������Ŀ�꼰�����ж�
/***********************************���ݰ����ж�**************************/
u8 PackJudge(device_type *dev,struct pbuf *judge_p)
{
	if(!judge_p->head.direct){
		if(judge_p->head.DestinationCount != 0){
			return 0;
		}else{
			if(judge_p->head.Config){
				return 1;
			}else if(judge_p->head.Query){
				return 2;
			}else{
				if(*dev >= spi1_rx)
					return 3;
			}
		}
	}else{
		*dev = spi1_tx;
		return 4;
	}
}
//input��dev��pbuf��ָ�������ݶ˿ڡ� presendbuf��pbuf�ṹ���ݵ�ַ
//output��none
//descript��pbuf�ṹ��������spi�ӿڻ�����
/**************************************pbuf�ṹ��������spi�ӿڻ�����***********************/
void pbuf_to_spi(device_type dev,struct pbuf *presendbuf)
{
	uint8_t i;
	uint8_t *p = (uint8_t *)presendbuf;
	uint16_t remin_len = presendbuf->head.framelength-8;
	uint16_t count = 0;
	
	p = p + 4;
	for(i = 0; i< 8; i++){
		EnQueue(cir_buf[dev],*p++);
	}
	p =p + 4;
	while(count++ < remin_len)
	{
		EnQueue(cir_buf[dev],*p++);
	}
	InsertLink(QLinkList[dev], remin_len+8 ,(u32)cir_buf[dev]->rear);
	pbuf_free(presendbuf);
}	
//input��dev����ѯ�����ݶ˿ڣ�assgin_p��pbuf�ṹ
//output��none
//descript���Բ�ѯָ��������ݰ������
void query_uart_data_take(device_type dev, struct pbuf *Assgin_p)
{
	Assgin_p->payload[0] = uartpara[dev].buadrate;
	*(u32*)((u32)(Assgin_p->payload)+1) = uartpara[dev].preemptionpriority;
	*(u32*)((u32)(Assgin_p->payload)+2) = uartpara[dev].subpriority;
	Assgin_p->head.framelength += 3;
	Assgin_p->head.Query = 0;
}
//input��order�����ݹ������ж��ó��ķ���ֵ��dev��ָ�����ݶ˿ڡ�assgin_p��pbuf�ṹ���ݵ�ַ
//output��none
//descript������ָ�����Ӧ��������
/************************************����ָ�����Ӧ��������*************************/
void AssignOpration(u8 order,device_type dev,struct pbuf *Assgin_p)
{
	struct pbuf *feedbackpack;
	switch(order)
	{
		case 0://ת������һ���忨
			Assgin_p->head.DestinationCount -= 1;
			spi1_to_spi2(Assgin_p);		
			break;
		case 1://���ò���
			uartpara[dev-6].buadrate = *Assgin_p->payload;
			uartpara[dev-6].preemptionpriority = *((u32*)((u32)Assgin_p->payload+1));
			uartpara[dev-6].subpriority = *((u32*)(((u32)Assgin_p->payload+2)));
			uart_init(COM_USART[dev-6],uartpara[dev-6].buadrate,uartpara[dev-6].preemptionpriority,uartpara[dev-6].subpriority);
			printf("uart config baud = 9600 success!\r\n");
		break;
		case 2://��ѯ����
			query_uart_data_take(dev-6,Assgin_p);
			pbuf_to_spi(spi1_tx,Assgin_p);
			break;
		case 3://ת��������
			spi_to_uart(dev,Assgin_p);
			break;
		case 4://�ش���spi1���ϴ��ϼ��忨
			pbuf_to_spi(dev,Assgin_p);
			break;
		default:break;
	}
}
//input��output_dev_port:�����ָ���˿�
//output��none
//descript�����ݷ�����ָ�����ݶ˿�
/*******************************************���ݷ������豸�ӿ�******************/
void QueueDataSend(device_type output_dev_port)
{
	switch(output_dev_port)
	{
		case uart1_tx://uart1
		case uart2_tx://uart2
		case uart3_tx://uart3
		case uart4_tx://uart4
			usart_sendstring((uint8_t)output_dev_port-6,cir_buf[output_dev_port]->rear, QLinkList[output_dev_port]->next->size);
			DeleteLink(QLinkList[output_dev_port]);
			break;
		case spi1_tx://spi1
		case spi2_tx:
			spi_send(output_dev_port,QLinkList[output_dev_port]->next->size);
			DeleteLink(QLinkList[output_dev_port]);
			break;
	}
}
//input��dev����������ݶ˿ڡ�buf��pbuf�ṹ���ݵ�ַ
//output��none
//descript���ײ��������ݽӿ�
/**************************************�ײ��������ݽӿ�***********************/
device_type low_level_input(device_type dev, struct pbuf **buf)
{
	device_type dev_port_destination;
	dev_port_destination = DataRec(dev,buf);//�����ݴ�SPI������ȡ������pbuf�ṹ
	return dev_port_destination;
}
//input��dev����������ݶ˿ںš�buf��pbuf�ṹ���ݵ�ַ
//output��none
//descript��Ӧ�ò�Խ��յ�pbuf�ṹ�е����ݽ��д���
void apply_layer_ctl(device_type dev, struct pbuf *buf)
{
	u8 res;

	res=PackJudge(&dev,buf);//�ж��ð����ݵ����ö����Ƿ��Ѿ�ΪĿ��忨
	AssignOpration(res,dev,buf);//�������ö�������ͬ����
	pbuf_free(buf);//�ͷ������ڴ�
}
//input��output_dev_port�����ָ���Ķ˿ں�
//output��none
//descript���ײ�������ݽӿ�
/****************************************�ײ�������ݽӿ�********************/
void low_level_output(device_type output_dev_port)
{
		QueueDataSend(output_dev_port);
}
//input��data_input_dev_port:�������ݵĶ˿ںš�buf��pbuf�ṹ��ַ���������ݵĽ���
//output�����ظ����ݰ����������ö˿�
//descript���ӻ���buffer�н�����ȡ��������pbuf�ṹ��
device_type apply_layer_getdata(device_type data_input_dev_port, struct pbuf **buf)
{
	device_type dev_port_destination;
	dev_port_destination = low_level_input(data_input_dev_port,buf);
	return dev_port_destination;
}
//input��data_pack_src_dev�����ݰ�����ʱ�Ķ˿ں�
//output��none
//descript��Ӧ�ò��������ݽӿ�
/****************************************Ӧ�ò��������ݽӿ�********************/
void apply_layer_input(device_type data_pack_src_dev)
{
	struct pbuf *pre_pbuf = NULL;
	device_type destination_dev_port;
	uint8_t dat;
	if(QLinkList[data_pack_src_dev]->next->size >= 8){
		destination_dev_port = apply_layer_getdata(data_pack_src_dev,&pre_pbuf);
		apply_layer_ctl(destination_dev_port,pre_pbuf);
	}else{
		if(*(u8*)QLinkList[data_pack_src_dev]->next->start_postion == 0xFF){
			*(u8*)QLinkList[data_pack_src_dev]->next->start_postion = 0;
				
				dat = DeQueue(cir_buf[spi1_tx]);
				SPI2_WriteByte(SPI1,dat);
			
		}
		DeleteLink(QLinkList[spi1_rx]);
	}
}
//input��output_dev_port��������ݶ˿ں�
//output��none
//descript��Ӧ�ò�������ݽӿ�
/****************************************Ӧ�ò�������ݽӿ�********************/
void apply_layer_output(device_type output_dev_port)
{
	low_level_output(output_dev_port);
}