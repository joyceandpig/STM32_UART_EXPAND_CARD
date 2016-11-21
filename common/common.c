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
extern u32 memp_get_memorysize(void);	//在memp.c里面定义
extern u8 *memp_memory;				//在memp.c里面定义.
extern USART_TypeDef* COM_USART[];
extern USART_InitTypeDef USART_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;

SqQueue *cir_buf[MAX_BUF_NUM];
QueueLinkList QLinkList[MAX_BUF_NUM] = {0};

static uint8_t protocal_version = 0;
static uint8_t expand = 0;

struct pack_head *take_head = {0};

static uint8_t query_baud = 0,query_priro = 0, query_subpriro = 0;

//内存释放
void comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
}

//内存申请
//返回值:0,成功;
//    其他,失败
u8 comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	
	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
	memp_memory=mymalloc(SRAMIN,mempsize);	//为memp_memory申请内存
//	printf("memp_memory内存大小为:%d\r\n",mempsize);
	if(!memp_memory){	//有申请失败的
		comm_mem_free();
		return 1;
	}
	return 0;	
}
void qlink_init(QueueLinkList *L)
{
	(*L) = (QueueLink *)mymalloc(SRAMIN,sizeof(QueueLink));
	(*L)->next = NULL;
	(*L)->size = 0;
}
void sq_init(Queue *L)
{
	(*L) = (SqQueue *)mymalloc(SRAMIN,sizeof(SqQueue));
}
/**********************************环形缓冲q队列初始化************************/
void queue_init(void)
{
	u8 i = 0;
	for(; i < MAX_BUF_NUM;i++){
		sq_init(&cir_buf[i]);
		qlink_init(&QLinkList[i]);
		
		cir_buf[i]->size = 0;
		cir_buf[i]->queue_buf = (uint32_t *)drive_queue_alloc(MEMP_uart1_pool+i/2);
		(cir_buf[i]->front) = cir_buf[i]->queue_buf ;
		(cir_buf[i]->rear) = cir_buf[i]->queue_buf ;
		
		QLinkList[i]->next = NULL;
		QLinkList[i]->size = 0;
		QLinkList[i]->start_postion = 0;
	}
}
/***********************************环形缓冲队列记录链表插入操作***********************/
ErrorStatus InsertLink(QueueLinkList linklist,uint16_t len, uint32_t start_pos)
{
	uint16_t i;
	QueueLinkList p = linklist,q;
	
	
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
/*************************************环形缓冲队列记录链表删除操作*********************/
ErrorStatus DeleteLink(QueueLinkList linklist)
{
	QueueLinkList p = linklist->next,q = linklist;
	
	if(!q->next)return ERROR;
	q->next = p->next;
	free(p);
	return SUCCESS;
}
/**************************************遍历记录链表并得到链表长度********************/
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
/******************************判定队列是否为满队列******************************/
static uint8_t IsFull(SqQueue *Q)
{
	return (Q->size >= MEMP_BUF_LAYER_TYPE_SIZE)?TRUE:FALSE;
}


/******************************判定队列是否为空队列*****************************/
static uint8_t IsEmpty(SqQueue *Q)
{
	return (Q->size <= 0)?TRUE:FALSE;
}

/*********************************环形队列 入队列操作****************************/
uint8_t EnQueue(SqQueue *Q, uint8_t data)
{
	if (IsFull(Q))
		return FALSE;
	*Q->front = data;
	Q->size++;
	Q->front = (u32 *)((u32)Q->queue_buf+(((u32)Q->front)+1- (u32)Q->queue_buf)%MEMP_BUF_LAYER_TYPE_SIZE);
	return TRUE;
}
/**********************************环形队列 出队操作************************/
uint8_t DeQueue(SqQueue *Q)
{
	u8 dat;
	if (IsEmpty(Q))
		return FALSE;
	dat = *Q->rear;
	Q->rear += 1;
	Q->size -= 1;
	return dat;
}
/*********************************清空队列操作***********************************/
uint8_t ClearOfQueue(SqQueue *Q)
{
	memset(Q->queue_buf,'\0',MEMP_BUF_LAYER_TYPE_SIZE);
	Q->front = Q->rear = Q->queue_buf;
	Q->size = 0;
}
/*********************************从环形缓冲队列中移出一个数据包****************/
uint8_t RemovePackOfQueue(SqQueue *Q)
{
	Q->front = Q->rear;
	Q->size = 0;
}
/******************************获取环形队列中数据长度***************************/
uint8_t GetLengthOfQueue(SqQueue *Q)
{
	return (*Q->front-*Q->rear + MEMP_BUF_LAYER_TYPE_SIZE)%MEMP_BUF_LAYER_TYPE_SIZE;
}



/*********************************SPI2接口数据送到SPI1缓冲区****************************/
void spi2_to_spi1(struct pbuf *judge_p)
{
//	memcpy((u32 *)*cir_buf[spi1_tx]->front,(u32 *)*cir_buf[spi2_rx]->front,*((u32 *)(*cir_buf[spi2_rx]->front)));
//	*(u32 *)((*cir_buf[spi1_tx]->front + 2)) += 1;
	
	uint16_t i,len = judge_p->head.framelength;
	uint8_t *p = (uint8_t *)judge_p->payload;
	
	InsertLink(QLinkList[spi1_tx],len,*cir_buf[spi1_tx]->front);
	for(i = 0;i < len;i++){
		EnQueue(cir_buf[spi1_tx],*p++);
	}
	pbuf_free(judge_p);
}
/***********************************SPI发送数据**************************/
void spi_send(device_type dev, uint16_t sendlen)
{
	if(dev == spi1_tx){
		GPIO_SetBits(GPIOA,GPIO_Pin_8);//拉高主机中断线，告诉主机有数据回传，使主机提供时钟
		SPI_SendRec_Data(SPI1,sendlen);
		GPIO_ResetBits(GPIOA,GPIO_Pin_8);//拉低主机中断线，告诉主机数据传输完成，使主机断开时钟
	}else if(dev == spi2_tx){
		SPI_SendRec_Data(SPI2,sendlen);
	}
}
/***********************************SPI1数据送到SPI2缓冲区**************************/
void spi1_to_spi2(struct pbuf *judge_p)
{
//	memcpy(cir_buf[spi2_tx]->front,judge_p,judge_p->head.framelength);
//	SPI_SendRec_Data(SPI2,judge_p->head.framelength);
	
	uint16_t i,len = judge_p->head.framelength;
	uint8_t *p = (uint8_t *)judge_p->payload;
	
	InsertLink(QLinkList[spi2_tx],len,*cir_buf[spi2_tx]->front);
#if FRAME_START_SIGN_EN
	EnQueue(cir_buf[dev],FRAME_START_SIGN1);
	EnQueue(cir_buf[dev],FRAME_START_SIGN2);
#endif
	for(i = 0;i < len;i++){
		EnQueue(cir_buf[spi2_tx],*p++);
	}
#if FRAME_START_SIGN_EN	
	EnQueue(cir_buf[dev],FRAME_END_SIGN1);
	EnQueue(cir_buf[dev],FRAME_END_SIGN2);
#endif
	pbuf_free(judge_p);
}
/**************************************SPI数据送至串口区***********************/
void spi_to_uart(device_type dev,struct pbuf *judge_p)
{
//	memcpy(cir_buf[dev]->front,judge_p->payload,judge_p->head.framelength-8);
//	usart_sendstring(COM_USART[dev-1],judge_p->payload,judge_p->head.framelength);
	
	uint16_t i,len = judge_p->head.framelength-8;
	uint8_t *p = (uint8_t *)judge_p->payload;
	
	InsertLink(QLinkList[dev],len,(u32)cir_buf[dev]->front);
	
	for(i = 0;i < len;i++){
		EnQueue(cir_buf[dev],*p++);
	}

	pbuf_free(judge_p);
}
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
/****************************************数据接收判定与处理*********************/
struct pbuf *DataRec(device_type dev, struct pbuf **buf)
{
//	struct pbuf *new_p;
	u32 *p = NULL;
	
	(*buf) = (struct pbuf *)pbuf_alloc(512);//申请一个内存空间
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
	}else{
		memcpy((u32*)&(*buf)->head,(u32*)QLinkList[dev]->next->start_postion,SIZEOF_STRUCT_PACKHEAD);//QLinkList[dev]->next->size);//将接收缓存数据拷备至新建内存空间
		memcpy((*buf)->payload,(u32*)(QLinkList[dev]->next->start_postion+SIZEOF_STRUCT_PACKHEAD),QLinkList[dev]->next->size-SIZEOF_STRUCT_PACKHEAD);
	}
	ClearOfQueue(cir_buf[dev]);//清除缓存中接收的数据
	DeleteLink(QLinkList[dev]);
	if(QLinkList[dev]->next == NULL){
		received_buffer_flag &= ~0x20;//清除spi1—rx接收标志位
	}
	return *buf;
}
/***********************************数据包的判定**************************/
u8 PackJudge(device_type *dev,struct pbuf *judge_p)
{
	if(!judge_p->head.direct){
	if(judge_p->head.DestinationCount != 0){
		return 0;
	}else{
			*dev = judge_p->head.DevicePort+5;
		if(judge_p->head.Config){
			return 1;
		}else if(judge_p->head.Query){
			return 2;
		}else{
			if(*dev >= spi1_rx){
				return 3;
			}else {
				return 4;
			}
		}
		}

	}else{
		*dev = 10;
		return 5;
	}
}

/**************************************pbuf结构数据送至spi接口缓冲区***********************/
void pbuf_to_spi(device_type dev,struct pbuf *presendbuf)
{
	uint32_t *p = (uint32_t *)presendbuf;
	uint16_t len = presendbuf->head.framelength;
	uint16_t count = 0;
	

	while(count++ < len)
	{
		EnQueue(cir_buf[dev],*p++);
	}
	InsertLink(QLinkList[dev], len ,(u32)cir_buf[dev]->rear);
	pbuf_free(presendbuf);
}	
void query_uart_data_take(device_type dev, struct pbuf *Assgin_p)
{
	Assgin_p->payload[0] = uartpara[dev].buadrate;
	Assgin_p->payload[1] = uartpara[dev].preemptionpriority;
	Assgin_p->payload[2] = uartpara[dev].subpriority;
	Assgin_p->head.framelength += 3;
	Assgin_p->head.Query = 0;
}
/************************************数据指令与对应处理流程*************************/
void AssignOpration(u8 order,device_type dev,struct pbuf *Assgin_p)
{
	struct pbuf *feedbackpack;
	switch(order)
	{
		case 0://转发至下一级板卡
			Assgin_p->head.DestinationCount -= 1;
			spi1_to_spi2(Assgin_p);		
			break;
		case 1://配置参数
			uartpara[dev-6].buadrate = *Assgin_p->payload;
			uartpara[dev-6].preemptionpriority = *((u32*)((u32)Assgin_p->payload+1));
			uartpara[dev-6].subpriority = *((u32*)(((u32)Assgin_p->payload+2)));
			uart_init(COM_USART[dev-6],uartpara[dev-6].buadrate,uartpara[dev-6].preemptionpriority,uartpara[dev-6].subpriority);
			printf("uart config success!\r\n");
		break;
		case 2://查询参数
			query_uart_data_take(dev,Assgin_p);
			pbuf_to_spi(spi1_tx,Assgin_p);
			break;
		case 3://转发至串口
			spi_to_uart(dev,Assgin_p);
			break;
		case 4:
		case 5:
			pbuf_to_spi(dev,Assgin_p);
			break;
		default:break;
	}
}

/*******************************************数据发送至设备接口******************/
void QueueDataSend(device_type dev)
{
	switch(dev)
	{
		case uart1_tx://uart1
		case uart2_tx://uart2
		case uart3_tx://uart3
		case uart4_tx://uart4
			usart_sendstring((uint8_t)dev,cir_buf[dev+6]->rear, QLinkList[dev+6]->next->size);
			DeleteLink(QLinkList[dev+6]);
			break;
		case spi1_tx://spi1
		case spi2_tx:
			spi_send(dev,QLinkList[dev]->size);
			break;
	}
	
}
/**************************************底层输入数据接口***********************/
void low_level_input(device_type dev, struct pbuf **buf)
{
	DataRec(dev,buf);//将数据从SPI缓存中取出放入pbuf结构
}
void apply_layer_ctl(device_type dev, struct pbuf *buf)
{
	u8 res;

	res=PackJudge(&dev,buf);//判定该包数据的作用对象及是否已经为目标板卡
	AssignOpration(res,dev,buf);//根据作用对象做不同处理
	pbuf_free(buf);//释放申请内存
}
/****************************************底层输出数据接口********************/
void low_level_output(device_type dev)
{
		QueueDataSend(dev);
}
void apply_layer_getdata(device_type dev, struct pbuf **buf)
{
	low_level_input(dev,buf);
}
/****************************************应用层输入数据接口********************/
void apply_layer_input(device_type dev)
{
	struct pbuf *p = NULL;
	apply_layer_getdata(dev,&p);
	apply_layer_ctl(dev,p);
}
/****************************************应用层输出数据接口********************/
void apply_layer_output(device_type dev)
{
	low_level_output(dev);
}