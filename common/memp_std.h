

#ifndef LWIP_MALLOC_MEMPOOL
/* This treats "malloc pools" just like any other pool.
   The pools are a little bigger to provide 'size' as the amount of user data. */
#define LWIP_MALLOC_MEMPOOL(num, size) LWIP_MEMPOOL(POOL_##size, num, (size + sizeof(struct memp_malloc_helper)), "MALLOC_"#size)
#define LWIP_MALLOC_MEMPOOL_START
#define LWIP_MALLOC_MEMPOOL_END
#endif /* LWIP_MALLOC_MEMPOOL */ 

#ifndef LWIP_PBUF_MEMPOOL
/* This treats "pbuf pools" just like any other pool.
 * Allocates buffers for a pbuf struct AND a payload size */
#define LWIP_PBUF_MEMPOOL(name, num, payload, desc) LWIP_MEMPOOL(name, num, (MEMP_ALIGN_SIZE(sizeof(struct pbuf)) + MEMP_ALIGN_SIZE(payload)), desc)
#endif /* LWIP_PBUF_MEMPOOL */

#if UART1_BUF
LWIP_MEMPOOL(uart1_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart1_PCB")
#endif /* LWIP_RAW */
#if UART2_BUF
LWIP_MEMPOOL(uart2_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart2_PCB")
#endif /* LWIP_RAW */
#if UART3_BUF
LWIP_MEMPOOL(uart3_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart3_PCB")
#endif /* LWIP_RAW */
#if UART4_BUF
LWIP_MEMPOOL(uart4_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart4_PCB")
#endif /* LWIP_RAW */

#if SPI1_BUF
LWIP_MEMPOOL(spi1_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart1_PCB")
#endif /* LWIP_RAW */
#if SPI2_BUF
LWIP_MEMPOOL(spi2_pool,        2,         MEMP_BUF_LAYER_TYPE_SIZE, "uart1_PCB")
#endif /* LWIP_RAW */


LWIP_MEMPOOL(pbuf_pool,        MEMP_BUF_LAYER_TYPE_NUM,         MEMP_BUF_LAYER_TYPE_SIZE, "pbuf_PCB")


#undef LWIP_MEMPOOL
#undef LWIP_MALLOC_MEMPOOL
#undef LWIP_MALLOC_MEMPOOL_START
#undef LWIP_MALLOC_MEMPOOL_END
#undef LWIP_PBUF_MEMPOOL