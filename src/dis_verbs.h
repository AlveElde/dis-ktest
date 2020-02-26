#ifndef __DIS_VERBS_H__
#define __DIS_VERBS_H__

#include <rdma/ib_verbs.h>

#define DIS_MAX_PD      1
#define DIS_MAX_CQ      1
#define DIS_MAX_QP      1
#define DIS_MAX_SGE     1
#define DIS_MAX_SQE     1
#define DIS_MAX_CQE     DIS_MAX_SQE
#define DIS_MAX_MSG_LEN 128

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

struct sge_ctx {
    struct ib_sge   *ibsge;
	u64	            addr;
	u32	            length;
	u32	            lkey;
};

struct sqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_send_wr       ibwr;
    const struct ib_send_wr *ibbadwr;
};

struct rqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_recv_wr       ibwr;
    const struct ib_recv_wr *ibbadwr;
};

struct cqe_ctx {
    struct ib_cq    *ibcq;
    int             num_entries;
    struct ib_wc    ibwc[DIS_MAX_CQE];
};

struct requester_ctx {
    struct ib_device    *ibdev;
    struct pd_ctx       pd;
    struct cq_ctx       cq;
    struct qp_ctx       qp1;
    struct sge_ctx      sge[DIS_MAX_SGE];
    struct sqe_ctx      sqe;
    struct cqe_ctx      cqe;
};

struct responder_ctx {
    struct ib_device    *ibdev;
    struct pd_ctx       pd;
    struct cq_ctx       cq;
    struct qp_ctx       qp1;
    struct sge_ctx      sge[DIS_MAX_SGE];
    struct rqe_ctx      rqe;
    struct cqe_ctx      cqe;
};

int verbs_poll_cq(struct cqe_ctx *cqe);
int verbs_post_send(struct sqe_ctx *sqe);
int verbs_post_recv(struct rqe_ctx *rqe);
// int verbs_alloc_mr(struct mr_ctx *mr);
int verbs_create_qp(struct qp_ctx *qp);
int verbs_create_cq(struct cq_ctx *cq);
int verbs_alloc_pd(struct pd_ctx *pd);


#endif /* __DIS_VERBS_H__ */