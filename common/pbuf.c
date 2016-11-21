#include "pbuf.h"
#include "err.h"
#include "sys.h"
#include "memp.h"
#include "stdio.h"


uint32_t drive_queue_alloc(memp_t type)
{
	uint32_t p;
	
	p =(uint32_t) memp_malloc(type);
	return p;
}
/**********pbuf½á¹¹ÄÚ´æÉêÇë************
*input:length 
*
*
*/
struct pbuf *pbuf_alloc(u16 length)
{
	struct pbuf *p;

	p = (struct pbuf *)memp_malloc(MEMP_pbuf_pool);
	p->next = NULL;
	p->payload = ((void *)((u8 *)p + SIZEOF_STRUCT_PBUF));
	
	return p;
}
u8 pbuf_free(struct pbuf *p)
{
	memp_free(MEMP_pbuf_pool,p);
}
err_t head_encode(struct pbuf *buf,struct pack_head *head)
{
	struct pbuf *hebuf = buf;
	struct pack_head *hehead = head;
	hebuf->head.framelength      = hehead->framelength;
	hebuf->head.direct           = hehead->direct;
	hebuf->head.DevicePort 			 = hehead->DevicePort;
	hebuf->head.Config 					 = hehead->Config;
	hebuf->head.DestinationCount = hehead->DestinationCount+1;
	hebuf->head.Expand 					 = hehead->Expand;
	hebuf->head.ProtocalVersion  = hehead->ProtocalVersion;
	
}
err_t pbuf_encode(struct pbuf *buf, const void *dataptr, u16 len, struct pack_head *head)
{
	struct pbuf *p = buf;
	struct pack_head *hp = head;
	u16 copy_len = len,rem_len;
	rem_len = len;
	
	head_encode(p,hp);
	
	for(;rem_len != 0;p=p->next)
	{
		memcpy(p->payload,dataptr,copy_len);
		rem_len -= copy_len;
	}
}








