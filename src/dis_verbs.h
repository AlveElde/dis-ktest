#include "dis_ktest.h"

int post_send(struct send_wr_ctx *wr);
int create_send_wr(struct send_wr_ctx *wr, struct sge_ctx sge[]);
// int alloc_mr(struct mr_ctx *mr);
int create_qp(struct qp_ctx *qp);
int create_cq(struct cq_ctx *cq);
int alloc_pd(struct pd_ctx *pd);