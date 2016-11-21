#ifndef __MEMP_H__
#define __MEMP_H__

#include "opt.h"
#include "opts.h"

typedef enum {
#define LWIP_MEMPOOL(name,num,size,desc)  MEMP_##name,
#include "memp_std.h"
  MEMP_MAX
}memp_t;
#define MAX_BUF_NUM  12

#ifndef LWIP_MEM_ALIGN_SIZE
#define LWIP_MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))
#endif
#ifndef LWIP_MEM_ALIGN
#define LWIP_MEM_ALIGN(addr) ((void *)(((uint32_t)(addr) + MEM_ALIGNMENT - 1) & ~(uint32_t)(MEM_ALIGNMENT-1)))
#endif


void *memp_malloc(memp_t type);
#endif