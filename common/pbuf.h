#ifndef __PBUF_H__
#define __PBUF_H__
#include "sys.h"
#include "memp.h"


typedef enum
{
	uart1_rx,
	uart2_rx,
	uart3_rx,
	uart4_rx,
	spi1_rx,
	spi2_rx,
	
	uart1_tx,
	uart2_tx,
	uart3_tx,
	uart4_tx,
	spi1_tx,
	spi2_tx,
}device_type;
/*内存缓冲块枚举值*/
typedef enum {
	PBUF_UART1,
	PBUF_UART2,
	PBUF_UART3,
	PBUF_UART4,
	PBUF_SPI1,
	PBUF_SPI2
} pbuf_type;
/*串口波特率枚举值*/
typedef enum
{
	USART_BAUDRATE_NULL,
	USART_BAUDRATE_9600,
	USART_BAUDRATE_14400,
	USART_BAUDRATE_19200,
	USART_BAUDRATE_38400,
	USART_BAUDRATE_56000,
	USART_BAUDRATE_57600,
	USART_BAUDRATE_115200,
	USART_BAUDRATE_194000
}UsartBaudrate;
/*数据包头枚举值*/
struct pack_head
{
	uint8_t framelength;
	uint8_t direct;
	uint8_t DestinationCount;
	uint8_t DevicePort;
	uint8_t Query;
	uint8_t Config;
	uint8_t ProtocalVersion;
	uint8_t Expand;
};
/*数据包结构定义*/
struct pbuf
{
	struct pbuf *next;//链表下一个节点
	struct pack_head head;//数据包头变量
	u32 *payload;  //真实数据区地址
};
struct PBUF
{
	struct pbuf *pbuf;
	u16 pbuf_size;
	u8 dat_src_port;
	u8 dat_dst_port;
};


#define SIZEOF_STRUCT_PBUF        LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf))
#define SIZEOF_STRUCT_PACKHEAD    LWIP_MEM_ALIGN_SIZE(sizeof(struct pack_head))
#endif