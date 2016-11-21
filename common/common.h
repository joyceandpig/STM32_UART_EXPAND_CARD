#ifndef __COMMON_H__
#define __COMMON_H__ 

#include "sys.h"
#include "memp.h"

extern uint8_t received_buffer_flag;
extern uint8_t sending_buffer_flag;
extern uint8_t StartSPI2RecData_flag;
typedef struct
{
	uint16_t size;
	uint32_t *front;
	uint32_t *rear;
	uint32_t *queue_buf;
}SqQueue,*Queue;
typedef struct queuelink
{
	struct queuelink *next;
	uint16_t size;
	uint32_t start_postion;
}QueueLink,*QueueLinkList;

extern SqQueue *cir_buf[MAX_BUF_NUM];
extern QueueLinkList QLinkList[MAX_BUF_NUM];;

typedef enum {FALSE = 0, TRUE = !FALSE,Failture = 0,Success = !Failture}RESULT;


u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);

#endif