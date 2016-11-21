#include "memp.h"
#include "sys.h"
#include "opts.h"


#define MEMP_SIZE           0
#define MEMP_ALIGN_SIZE(x) (LWIP_MEM_ALIGN_SIZE(x))

struct memp {
  struct memp *next;
};
static struct memp *memp_tab[MEMP_MAX];

/** This array holds the element sizes of each pool. */

static const uint16_t memp_sizes[MEMP_MAX] = {
#define LWIP_MEMPOOL(name,num,size,desc)  LWIP_MEM_ALIGN_SIZE(size),
#include "memp_std.h"
};


/** This array holds the number of elements in each pool. */
static const uint16_t memp_num[MEMP_MAX] = {
#define LWIP_MEMPOOL(name,num,size,desc)  (num),
#include "memp_std.h"
};

/** This array holds a textual description of each pool. */
static const char *memp_desc[MEMP_MAX] = {
#define LWIP_MEMPOOL(name,num,size,desc)  (desc),
#include "memp_std.h"
};


/** This is the actual memory used by the pools (all pools in one big block). */
//static u8_t memp_memory[MEM_ALIGNMENT - 1 
//#define LWIP_MEMPOOL(name,num,size,desc) + ( (num) * (MEMP_SIZE + MEMP_ALIGN_SIZE(size) ) )
//#include "lwip/memp_std.h"
//];
//memp_memory在lwip_comm.c文件中的lwip_comm_mem_malloc()函数采用ALIENTEK动态内存管理函数分配内存
uint8_t *memp_memory; 


//得到memp_memory数组大小
uint32_t memp_get_memorysize(void)
{
	uint32_t length=0;
	length=(
			MEM_ALIGNMENT-1 //全局型数组 为所有POOL分配的内存空间
			//MEMP_SIZE表示需要在每个POOL头部预留的空间  MEMP_SIZE = 0
			#define LWIP_MEMPOOL(name,num,size,desc) +((num)*(MEMP_SIZE+MEMP_ALIGN_SIZE(size)))
			#include "memp_std.h"
			);
	return length;
}

/**
 * Initialize this module.
 * 
 * Carves out memp_memory into linked lists for each pool-type.
 */
void
memp_init(void)
{
  struct memp *memp;
  uint16_t i, j;
	memp = (struct memp *)LWIP_MEM_ALIGN(memp_memory);
  /* for every pool: */
  for (i = 0; i < MEMP_MAX; ++i) {
    memp_tab[i] = NULL;
    /* create a linked list of memp elements */
    for (j = 0; j < memp_num[i]; ++j) {
      memp->next = memp_tab[i];
      memp_tab[i] = memp;
      memp = (struct memp *)(void *)((uint8_t *)memp + MEMP_SIZE + memp_sizes[i]);
    }
  }
}
void *
memp_malloc(memp_t type)
{
  struct memp *memp;

  memp = memp_tab[type];
  
  if (memp != NULL) {
    memp_tab[type] = memp->next;

    memp = (struct memp*)(void *)((uint8_t*)memp + MEMP_SIZE);
  } 
  return memp;
}

/**
 * Put an element back into its pool.
 *
 * @param type the pool where to put mem
 * @param mem the memp element to free
 */
void
memp_free(memp_t type, void *mem)
{
  struct memp *memp;


  if (mem == NULL) {
    return;
  }


  memp = (struct memp *)(void *)((uint8_t*)mem - MEMP_SIZE);


  memp->next = memp_tab[type]; 
  memp_tab[type] = memp;
}