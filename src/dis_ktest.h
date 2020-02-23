#include <rdma/ib_verbs.h>

#define TOTAL_PDS 1
#define TOTAL_CQS 1
#define TOTAL_QPS 1

struct pd_ctx {
    struct ib_pd        *ibpd;
    struct ib_device    *ibdev;
    int                 flags;
};

struct cq_ctx {
    struct ib_cq        *ibcq;
    struct ib_device    *ibdev;
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *ibevent, void *cq_context);
    void *context;
    struct ib_cq_init_attr attr;
};

struct qp_ctx {
    struct ib_qp            *ibqp;
    struct ib_pd            *ibpd;
    struct ib_qp_init_attr  attr;
};

// struct mr_ctx {
//     struct ib_mr    *ibmr;
//     struct ib_pd    *ibpd;
// };

struct send_wr_ctx {
    struct ib_qp        *ibqp;
    struct ib_send_wr   *ibwr;
    struct ib_send_wr   *ibbadwr;
};


// struct ib_sge sge;
//     
//     struct ib_send_wr wr;
//     struct ib_send_wr bad_wr;