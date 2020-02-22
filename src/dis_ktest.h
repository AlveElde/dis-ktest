#include <rdma/ib_verbs.h>


struct test_context {
    // Device
    struct ib_device *ibdev;

    // PD
    int pd_flags;
    struct ib_pd *ibpd;

    // CQ
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *ibevent, void *cq_context);
    void *cq_context;
    struct ib_cq_init_attr cq_attr;
    struct ib_cq *ibcq;

    // QP
    struct ib_qp_attr attr;
    struct ib_qp_init_attr init_attr;
    struct ib_qp *ibqp;
};