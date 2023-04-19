#ifndef __MJPEGD_DECL_H__
#define __MJPEGD_DECL_H__
#include "lwip/err.h"

typedef struct client_state_struct client_state_t;
typedef struct request_handler_struct request_handler_t;

typedef err_t (*get_nextfile_func)(client_state_t* cs);
typedef err_t (*recv_request_func)(client_state_t* cs);
typedef err_t (*clsd_request_func)(client_state_t* cs);

#endif // __MJPEGD_DECL_H__
