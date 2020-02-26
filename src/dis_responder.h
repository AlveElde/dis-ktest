#ifndef __DIS_REQUESTER_H__
#define __DIS_REQUESTER_H__

#include <rdma/ib_verbs.h>

int responder_test(struct ib_device *ibdev);

#endif /* __DIS_REQUESTER_H__ */