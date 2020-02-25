#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_verbs.h"

int post_send(struct send_wr_ctx *wr)
{
    int ret;
    pr_devel(STATUS_START);
    ret = ib_post_send(wr->ibqp, wr->ibwr, &wr->ibbadwr);
    if (ret) {
        pr_devel(STATUS_FAIL);
        return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;
}

int create_send_wr(struct send_wr_ctx *wr, struct sge_ctx sge[])
{
    int i;
    pr_devel(STATUS_START);

    wr->ibwr = kzalloc(sizeof(struct ib_send_wr), GFP_KERNEL);
    if (!wr->ibwr) {
        pr_devel(STATUS_FAIL);
        return -42;
    }
    memset(wr->ibwr, 0, sizeof(struct ib_send_wr));

    wr->ibwr->opcode        = wr->opcode;
    wr->ibwr->num_sge       = wr->num_sge;
    wr->ibwr->send_flags    = wr->send_flags;
    wr->ibwr->wr_id         = wr->wr_id;

    wr->ibwr->sg_list = kzalloc(sizeof(struct ib_sge)*wr->num_sge, GFP_KERNEL);
    if (!wr->ibwr) {
        pr_devel(STATUS_FAIL);
        kfree(wr->ibwr);
        return -42;
    }
    memset(wr->ibwr->sg_list, 0, sizeof(struct ib_sge)*wr->num_sge);

    for(i = 0; i < wr->ibwr->num_sge; i++) {
        sge[i].ibsge = wr->ibwr->sg_list + (sizeof(struct ib_sge) * i);
        sge[i].ibsge->addr      = sge[i].addr;
        sge[i].ibsge->length    = sge[i].length;
        sge[i].ibsge->lkey      = sge[i].lkey;
    }

    pr_devel(STATUS_COMPLETE);
    return 0;
}

// int alloc_mr(struct mr_ctx *mr)
// {
//     pr_devel(STATUS_START);
//     mr->ibmr = ib_alloc_mr(&mr->ibpd, IB_ACCESS_REMOTE_READ |
//                             IB_ACCESS_REMOTE_WRITE |
//                             IB_ACCESS_LOCAL_WRITE);
//     if (!mr->ibmr) {
//         pr_devel(STATUS_FAIL);
//         return -42;
//     }
//     pr_devel(STATUS_COMPLETE);
//     return 0;
// }

int create_qp(struct qp_ctx *qp)
{
    pr_devel(STATUS_START);
    qp->ibqp = ib_create_qp(qp->ibpd, &qp->attr);
    if (!qp->ibqp) {
        pr_devel(STATUS_FAIL);
        return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;
}

int create_cq(struct cq_ctx *cq)
{
    pr_devel(STATUS_START);
    cq->ibcq = ib_create_cq(cq->ibdev,
                                cq->comp_handler,
                                cq->event_handler,
                                cq->context,
                                &cq->attr);
    if (!cq->ibcq) {
        pr_devel(STATUS_FAIL);
        return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;
}

int alloc_pd(struct pd_ctx *pd)
{
    pr_devel(STATUS_START);
    pd->ibpd = ib_alloc_pd(pd->ibdev, pd->flags);
    if (!pd->ibpd) {
        pr_devel(STATUS_FAIL);
		return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;
}