#include "dis_ktest.h"

int verb_poll_cq(struct cqe_ctx *cqe);
int verb_post_send(struct sqe_ctx *sqe);
// int verb_alloc_mr(struct mr_ctx *mr);
int verb_create_qp(struct qp_ctx *qp);
int verb_create_cq(struct cq_ctx *cq);
int verb_alloc_pd(struct pd_ctx *pd);
