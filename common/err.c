#include "err.h"

#ifdef _DEBUG

static const char *err_strerr[] = {
           "Ok.",                    /* ERR_OK          0  */
           "Out of memory error.",   /* ERR_MEM        -1  */
           "Buffer error.",          /* ERR_BUF        -2  */
           "Timeout.",               /* ERR_TIMEOUT    -3  */
           "Routing problem.",       /* ERR_RTE        -4  */
           "Operation in progress.", /* ERR_INPROGRESS -5  */
           "Illegal value.",         /* ERR_VAL        -6  */
           "Operation would block.", /* ERR_WOULDBLOCK -7  */
           "Address in use.",        /* ERR_USE        -8  */
           "Already connected.",     /* ERR_ISCONN     -9  */
           "Connection aborted.",    /* ERR_ABRT       -10 */
           "Connection reset.",      /* ERR_RST        -11 */
           "Connection closed.",     /* ERR_CLSD       -12 */
           "Not connected.",         /* ERR_CONN       -13 */
           "Illegal argument.",      /* ERR_ARG        -14 */
           "Low-level netif error.", /* ERR_IF         -15 */
};

/**
 * Convert an lwip internal error to a string representation.
 *
 * @param err an lwip internal err_t
 * @return a string representation for err
 */
const char *
strerr(err_t err)
{
  return err_strerr[-err];

}

#endif /* LWIP_DEBUG */