#ifndef __DIS_SEND_RECEIVE_H__
#define __DIS_SEND_RECEIVE_H__

#include <rdma/ib_verbs.h>

int send_receive_program(struct ib_device *ibdev);

#endif /* __DIS_SEND_RECEIVE_H__ */