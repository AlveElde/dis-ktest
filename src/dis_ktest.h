#ifndef __DIS_KTEST_H__
#define __DIS_KTEST_H__

#include <rdma/ib_verbs.h>

#define PD_NUM  1
#define CQ_NUM  1
#define QP_NUM  1
#define WQE_PER_QP 1
#define SGE_PER_WQE 2

#define SGE_NUM     QP_NUM * WQE_PER_QP * SGE_PER_WQE
#define MR_NUM      SGE_NUM * 2
#define CQE_PER_CQ  WQE_PER_QP * 2

#define SGE_LENGTH  20000

#define POLL_TIMEOUT_SEC    20
#define POLL_INTERVAL_MSEC  200


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
    struct ib_wc            cqe[CQE_PER_CQ];
    void                    *context;   
    int                     cqe_expected;
    int                     cqe_c;
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *ibevent, void *cq_context);
};

struct rqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_recv_wr       ibwr;
    struct ib_sge           ibsge[SGE_PER_WQE];
    const struct ib_recv_wr *ibbadwr;
};

struct sqe_ctx {
    struct ib_qp            *ibqp;
    struct ib_send_wr       ibwr;
    struct ib_sge           ibsge[SGE_PER_WQE];
    const struct ib_send_wr *ibbadwr;
};

struct qp_ctx {
    struct ib_qp                *ibqp;
    struct ib_pd                *ibpd;
    struct ib_qp_init_attr      init_attr;
    struct ib_qp_attr           attr;
    struct sqe_ctx              sqe[WQE_PER_QP];
    struct rqe_ctx              rqe[WQE_PER_QP];
    struct cq_ctx               *send_cq;
    struct cq_ctx               *recv_cq;
    const struct ib_gid_attr    *gid_attr;
    int                         attr_mask;
    int                         sqe_c;
    int                         rqe_c;
};

struct sge_ctx {
    char    send_sge[SGE_LENGTH];
    char    recv_sge[SGE_LENGTH];
    int     length;
    int     lkey;
};

struct mr_ctx {
    struct ib_mr    *ibmr;
    struct ib_pd    *ibpd;
    int             access;
};

struct send_receive_ctx {
    struct dev_ctx  dev;
    struct pd_ctx   pd[PD_NUM];
    struct cq_ctx   cq[CQ_NUM];
    struct qp_ctx   qp[QP_NUM];
    struct sge_ctx  sge[SGE_NUM];
    struct mr_ctx   mr[MR_NUM];
    int             pd_c;
    int             cq_c;
    int             qp_c;
    int             sge_c;
    int             mr_c;
};

#endif /* __DIS_KTEST_H__ */