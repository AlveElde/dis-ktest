#ifndef __DIS_VERBS_H__
#define __DIS_VERBS_H__

#include <rdma/ib_verbs.h>

#define DIS_MAX_PD      1
#define DIS_MAX_CQ      1
#define DIS_MAX_QP      1
#define DIS_MAX_SGE     1
#define DIS_MAX_SQE     1
#define DIS_MAX_RQE     1
#define DIS_MAX_CQE     DIS_MAX_SQE + DIS_MAX_RQE
#define DIS_MAX_MSG_LEN 128

#define DIS_POLL_SLEEP_MS    200
#define DIS_POLL_TIMEOUT_SEC 20
#define DIS_POLL_TIMEOUT_MS  DIS_POLL_TIMEOUT_SEC * 1000

struct buf_ctx {

};

struct dev_ctx {
    struct ib_device        *ibdev;
    struct ib_device_attr   dev_attr;
    struct ib_port_attr     port_attr;
    u8                      port_num;
};

struct pd_ctx {
    struct ib_pd        *ibpd;
    struct ib_device    *ibdev;
    int                 flags;
};

struct cq_ctx {
    struct ib_cq            *ibcq;
    struct ib_device        *ibdev;
    struct ib_cq_init_attr  init_attr;
    struct ib_wc            cqe[DIS_MAX_CQE];
    void                    *context;   
    int                     expected_cqe;
    int                     cqe_c;
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *ibevent, void *cq_context);
};

struct sqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_send_wr       ibwr;
    struct ib_sge           ibsge[DIS_MAX_SGE];
    const struct ib_send_wr *ibbadwr;
};

struct rqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_recv_wr       ibwr;
    struct ib_sge           ibsge[DIS_MAX_SGE];
    const struct ib_recv_wr *ibbadwr;
};

struct qp_ctx {
    struct ib_qp                *ibqp;
    struct ib_pd                *ibpd;
    struct ib_qp_init_attr      init_attr;
    struct ib_qp_attr           attr;
    struct sqe_ctx              sqe[DIS_MAX_SQE];
    struct rqe_ctx              rqe[DIS_MAX_RQE];
    struct cq_ctx               *send_cq;
    struct cq_ctx               *recv_cq;
    const struct ib_gid_attr    *gid_attr;
    int                         attr_mask;
    int                         sqe_c;
    int                         rqe_c;
};

struct send_receive_ctx {
    struct dev_ctx  dev;
    struct pd_ctx   pd[DIS_MAX_PD];
    struct cq_ctx   cq[DIS_MAX_CQ];
    struct qp_ctx   qp[DIS_MAX_QP];

    int pd_c;
    int cq_c;
    int qp_c;
};

int verbs_query_port(struct dev_ctx *dev);
int verbs_alloc_pd(struct pd_ctx *pd);
int verbs_create_cq(struct cq_ctx *cq);
int verbs_create_qp(struct qp_ctx *qp);
int verbs_modify_qp(struct qp_ctx *qp);
int verbs_post_send(struct sqe_ctx *sqe);
int verbs_post_recv(struct rqe_ctx *rqe);
int verbs_poll_cq(struct cq_ctx *cqe);

// int verbs_alloc_mr(struct mr_ctx *mr);
// int responder_get_gid_attr(struct gid_ctx *gid);

// struct gid_ctx {
//     const struct ib_gid_attr  *gid_attr;
//     struct ib_device    *ibdev;
//     u8                  port_num;
//     int                 index;
// };

// struct mr_ctx {
//     struct ib_mr    *ibmr;
//     struct ib_pd    *ibpd;
// };

#endif /* __DIS_VERBS_H__ */