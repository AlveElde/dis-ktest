#ifndef __DIS_SEND_RECEIVE_H__
#define __DIS_SEND_RECEIVE_H__

#include <rdma/ib_verbs.h>

#include "dis_ktest.h"

int send_receive_init(struct send_receive_ctx *ctx);
void send_receive_exit(struct send_receive_ctx *ctx);

#endif /* __DIS_SEND_RECEIVE_H__ */