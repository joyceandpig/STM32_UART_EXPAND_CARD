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
uint8_t spi1_feedback_len = 0;

//input：none
//output：none
//descript：内存释放
void comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
}


//input：none
//output：none
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
//input：buffer管理链表
//output：none
//descript：初始化管理链表的头结点
void qlink_init(QueueLinkList *L)
{
	(*L) = (QueueLink *)mymalloc(SRAMIN,sizeof(QueueLink));
	(*L)->next = NULL;
	(*L)->size = 0;
}
//input：环形缓冲buffer
//output：none
//descript：环形缓冲buffer的初始化
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
//input：none
//output：none
//descript：环形缓冲buffer和buffer管理链表初始化
/**********************************环形缓冲队列初始化及管理链表初始化************************/
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
//input：管理链表名，本次插入数据的长度，该数据在buffer中的起始位置
//output：success or faild
//descript：环形buffer记录链表插入操作
/***********************************环形缓冲队列记录链表插入操作***********************/
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
//input：buffer记录链表名
//output：success or faild
//descript：从记录链表中删除一个节点信息
/*************************************环形缓冲队列记录链表删除操作*********************/
ErrorStatus DeleteLink(QueueLinkList linklist)
{
	QueueLinkList p = linklist->next,q = linklist;
	
	if(!q->next)return ERROR;
	q->next = p->next;
	free(p);
	return SUCCESS;
}
//input：记录链表名
//output：链表长度-节点个数
//descript：得到链表长度
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
//input：环形队列名
//output：true：队列已经满了，false：队列未满
//descript：判定队列是否为满队列
/******************************判定队列是否为满队列******************************/
static uint8_t IsFull(SqQueue *Q)
{
	return (Q->size >= MEMP_BUF_LAYER_TYPE_SIZE)?TRUE:FALSE;
}

//input：环形队列名
//output：true：队列已经空了，false：队列未空
//descript：判定队列是否为空队列
/******************************判定队列是否为空队列*****************************/
static uint8_t IsEmpty(SqQueue *Q)
{
	return (Q->size <= 0)?TRUE:FALSE;
}

//input：Q：环形队列名，data：本次插入环形队列的数据
//output：none
//descript：从环形buffer里将数据拷备至pbuf结构，并对数据拆包解析
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
//input：环形队列名
//output：dat：此次从环形队列里取出的数据
//descript：从环形队列取出一个数据，并将尾指针移动一位，将buffer大小减1
/**********************************环形队列 出队操作************************/
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
//input：环形队列名
//output：none
//descript：将指定队列内容清空，buffer大小置0
/*********************************清空队列操作***********************************/
void ClearOfQueue(SqQueue *Q)
{
	memset(Q->queue_buf,'\0',MEMP_BUF_LAYER_TYPE_SIZE);
	Q->front = Q->rear = Q->queue_buf;
	Q->size = 0;
}
//input：环形队列名
//output：none
//descript：从环形队列中移出一个指定类型的数据包
/*********************************从环形缓冲队列中移出一个数据包****************/
void RemovePackOfQueue(SqQueue *Q)
{
	Q->front = Q->rear;
	Q->size = 0;
}
//input：环形队列名
//output：指定队列中的数据长度
//descript：获取队列中的数据长度
/******************************获取环形队列中数据长度***************************/
uint8_t GetLengthOfQueue(SqQueue *Q)
{
	return (*Q->front-*Q->rear + MEMP_BUF_LAYER_TYPE_SIZE)%MEMP_BUF_LAYER_TYPE_SIZE;
}

//input：pbuf结构数据包地址
//output：none
//descript：将pbuf结构数据从spi2接口复制到spi1发送缓冲区
/*********************************SPI2接口数据送到SPI1缓冲区****************************/
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
//input：dev：spi端口号，spi1或者spi2
//       sendlen：本次发送的数据长度
//output：none
//descript：从指定的spi发送指定长度数据
/***********************************SPI发送数据**************************/
#include "delay.h"
void spi_send(device_type dev, uint16_t sendlen)
{
	if(dev == spi1_tx){
		GPIO_SetBits(GPIOA,GPIO_Pin_12);//拉高主机中断线，告诉主机有数据回传，使主机提供时钟
		spi1_feedback_len = sendlen;
		SPI2_WriteByte(SPI1,spi1_feedback_len);
//		SPI2_WriteByte(SPI1,sendlen);
//		SPI_SendRec_Data(SPI1,sendlen);
		GPIO_ResetBits(GPIOA,GPIO_Pin_12);//拉低主机中断线，告诉主机数据传输完成，使主机断开时钟
	}else if(dev == spi2_tx){
		SPI_SendRec_Data(SPI2,sendlen);
	}
}
//input：pbuf结构数据地址
//output：none
//descript：将pbuf结构数据从spi1复制到spi2的缓冲区
/***********************************SPI1数据送到SPI2缓冲区**************************/
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
//input：dev：本次数据的作用对象buffer号，取值为6-9。
//       judge_p：pbuf结构数据的地址
//output：none
//descript：将spi数据发送至指定串口的缓冲区
/**************************************SPI数据送至串口区***********************/
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
//input：head：数据包组成中固定头结构
//output：none
//descript：将头结构成员重置
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
//input：dev：本次数据接受来源端口，buf：pbuf结构地址，由于要对buf成员进行变化，故传入双重指针
//output：本次数据包需要作用的端口号
//descript：数据接收（从buffer缓存至pbuf结构），并对pbuf进行填充。
/****************************************数据接收判定与处理*********************/
device_type DataRec(device_type dev, struct pbuf **buf)
{
	u32 *p = NULL;
	device_type dev_port_detination;
	
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
		dev_port_detination = spi1_tx;
	}else{
		memcpy((u32*)&(*buf)->head,(u32*)QLinkList[dev]->next->start_postion,SIZEOF_STRUCT_PACKHEAD);//QLinkList[dev]->next->size);//将接收缓存数据拷备至新建内存空间
		memcpy((*buf)->payload,(u32*)(QLinkList[dev]->next->start_postion+SIZEOF_STRUCT_PACKHEAD),QLinkList[dev]->next->size-SIZEOF_STRUCT_PACKHEAD);
		dev_port_detination = (*buf)->head.DevicePort+uart1_tx-1;
		(*buf)->head.framelength = QLinkList[dev]->next->size;
	}
	ClearOfQueue(cir_buf[dev]);//清除缓存中接收的数据
	DeleteLink(QLinkList[dev]);
	
	if(QLinkList[dev]->next == NULL){
		received_buffer_flag &= ~0x20;//清除spi1—rx接收标志位
	}
	return dev_port_detination;
}
//input：dev：pbuf结构中指定的数据端口
//       judge_p：pbuf结构数据包地址
//output：判定结果
//descript：对接收到的数据还是先判定，确定其作用目标及功能判定
/***********************************数据包的判定**************************/
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
//input：dev：pbuf中指定的数据端口。 presendbuf：pbuf结构数据地址
//output：none
//descript：pbuf结构数据送至spi接口缓冲区
/**************************************pbuf结构数据送至spi接口缓冲区***********************/
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
//input：dev：查询的数据端口，assgin_p：pbuf结构
//output：none
//descript：对查询指令还是先数据包的填充
void query_uart_data_take(device_type dev, struct pbuf *Assgin_p)
{
	Assgin_p->payload[0] = uartpara[dev].buadrate;
	*(u32*)((u32)(Assgin_p->payload)+1) = uartpara[dev].preemptionpriority;
	*(u32*)((u32)(Assgin_p->payload)+2) = uartpara[dev].subpriority;
	Assgin_p->head.framelength += 3;
	Assgin_p->head.Query = 0;
}
//input：order：根据功能码判定得出的返回值。dev：指定数据端口。assgin_p：pbuf结构数据地址
//output：none
//descript：数据指令与对应处理流程
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
			printf("uart config baud = 9600 success!\r\n");
		break;
		case 2://查询参数
			query_uart_data_take(dev-6,Assgin_p);
			pbuf_to_spi(spi1_tx,Assgin_p);
			break;
		case 3://转发至串口
			spi_to_uart(dev,Assgin_p);
			break;
		case 4://回传至spi1，上传上级板卡
			pbuf_to_spi(dev,Assgin_p);
			break;
		default:break;
	}
}
//input：output_dev_port:输出的指定端口
//output：none
//descript：数据发送至指定数据端口
/*******************************************数据发送至设备接口******************/
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
//input：dev：输入的数据端口。buf：pbuf结构数据地址
//output：none
//descript：底层输入数据接口
/**************************************底层输入数据接口***********************/
device_type low_level_input(device_type dev, struct pbuf **buf)
{
	device_type dev_port_destination;
	dev_port_destination = DataRec(dev,buf);//将数据从SPI缓存中取出放入pbuf结构
	return dev_port_destination;
}
//input：dev：输入的数据端口号。buf：pbuf结构数据地址
//output：none
//descript：应用层对接收到pbuf结构中的数据进行处理
void apply_layer_ctl(device_type dev, struct pbuf *buf)
{
	u8 res;

	res=PackJudge(&dev,buf);//判定该包数据的作用对象及是否已经为目标板卡
	AssignOpration(res,dev,buf);//根据作用对象做不同处理
	pbuf_free(buf);//释放申请内存
}
//input：output_dev_port：输出指定的端口号
//output：none
//descript：底层输出数据接口
/****************************************底层输出数据接口********************/
void low_level_output(device_type output_dev_port)
{
		QueueDataSend(output_dev_port);
}
//input：data_input_dev_port:输入数据的端口号。buf：pbuf结构地址，用于数据的接收
//output：返回该数据包的最终作用端口
//descript：从环形buffer中将数据取出并放入pbuf结构中
device_type apply_layer_getdata(device_type data_input_dev_port, struct pbuf **buf)
{
	device_type dev_port_destination;
	dev_port_destination = low_level_input(data_input_dev_port,buf);
	return dev_port_destination;
}
//input：data_pack_src_dev：数据包输入时的端口号
//output：none
//descript：应用层输入数据接口
/****************************************应用层输入数据接口********************/
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
//input：output_dev_port：输出数据端口号
//output：none
//descript：应用层输出数据接口
/****************************************应用层输出数据接口********************/
void apply_layer_output(device_type output_dev_port)
{
	low_level_output(output_dev_port);
}