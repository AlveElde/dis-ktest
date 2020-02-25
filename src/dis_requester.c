#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_ktest.h"


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

void qp_event_handler(struct ib_event *ibevent, void *qp_context)
{
    return;
}

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

void cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
}

void cq_cq_event_handler(struct ib_event *ibevent, void *cq_context)
{
    return;
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

int send_request(struct requester_ctx *ctx)
{
    int ret;
    char message[30] = "Hello There!";
    
    pr_devel(STATUS_START);

    /* Create Protection Domain */
    memset(&ctx->pd, 0, sizeof(struct pd_ctx));
    ctx->pd.ibdev = ctx->ibdev;
    ctx->pd.flags = 0;
    ret = alloc_pd(&ctx->pd);
    if (ret) {
        goto alloc_pd_err;
    }

    /* Create a Completion Queue */
    memset(&ctx->cq, 0, sizeof(struct cq_ctx));
    ctx->cq.ibdev               = ctx->ibdev;
    ctx->cq.comp_handler        = cq_comp_handler,
    ctx->cq.event_handler       = NULL,
    ctx->cq.context             = NULL,

    ctx->cq.attr.cqe            = 10,
    ctx->cq.attr.comp_vector    = 0,
    ctx->cq.attr.flags          = 0,
    ret = create_cq(&ctx->cq);
    if (ret) {
        goto create_cq_err;
    }

    /* Create Queue Pair 1*/
    memset(&ctx->qp1, 0, sizeof(struct qp_ctx));
    ctx->qp1.ibpd = ctx->pd.ibpd;

    ctx->qp1.attr.qp_context     = NULL;
    ctx->qp1.attr.send_cq        = ctx->cq.ibcq;
    ctx->qp1.attr.recv_cq        = ctx->cq.ibcq;
    ctx->qp1.attr.srq            = NULL;
    ctx->qp1.attr.sq_sig_type    = IB_SIGNAL_ALL_WR;
    ctx->qp1.attr.qp_type        = IB_QPT_RC;
    ctx->qp1.attr.create_flags   = 0;

    ctx->qp1.attr.cap.max_send_wr        = 10;
    ctx->qp1.attr.cap.max_recv_wr        = 10;
	ctx->qp1.attr.cap.max_send_sge       = 1;
	ctx->qp1.attr.cap.max_recv_sge       = 1;
	ctx->qp1.attr.cap.max_inline_data    = 0;
    ret = create_qp(&ctx->qp1);
    if (ret) {
        goto create_qp_err;
    }

    /* Get DMA Memory Region */
    // mr.ibpd = pd.ibpd;
    // ret = alloc_mr(&mr);
    // if (ret) {
    //     goto get_mr_err;
    // }

    /* Define Segments To Send */
    ctx->sge[0].addr     = (uintptr_t)message;
    ctx->sge[0].length   = strlen(message);
    ctx->sge[0].lkey     = 1234;

    /* Create Send Work Request */
    memset(&ctx->send_wr, 0, sizeof(struct send_wr_ctx));
    ctx->send_wr.ibqp        = ctx->qp1.ibqp;
    ctx->send_wr.ibbadwr     = NULL;
	ctx->send_wr.opcode      = IB_WR_SEND;
    ctx->send_wr.num_sge     = TOTAL_SGE;
	ctx->send_wr.send_flags  = IB_SEND_SIGNALED;
	ctx->send_wr.wr_id       = 1;

    ret = create_send_wr(&ctx->send_wr, ctx->sge);
    if (ret) {
        goto create_wr_err;
    }

    /* Post Send Request */



    kfree(ctx->send_wr.ibwr->sg_list);
    pr_devel("kfree(ibwr.sg_list): " STATUS_COMPLETE);
    kfree(ctx->send_wr.ibwr);
    pr_devel("kfree(ibwr): " STATUS_COMPLETE);

create_wr_err:
    ib_destroy_qp(ctx->qp1.ibqp);
    pr_devel("ib_destroy_qp(ctx->qp1): " STATUS_COMPLETE);

// get_mr_err:
//     ib_destroy_qp(ctx->qp1.ibqp);
//     pr_devel("ib_destroy_qp(ctx->qp1): " STATUS_COMPLETE);

create_qp_err:
    ib_destroy_cq(ctx->cq.ibcq);
    pr_devel("ib_destroy_cq: " STATUS_COMPLETE);

create_cq_err:
    ib_dealloc_pd(ctx->pd.ibpd);
    pr_devel("ib_dealloc_pd: " STATUS_COMPLETE);

alloc_pd_err:
    pr_devel(STATUS_COMPLETE);
    return 0;

}

int test_requester(struct ib_device *ibdev)
{
    struct requester_ctx ctx;
    pr_devel(STATUS_START);

    ctx.ibdev = ibdev;
    send_request(&ctx);

    pr_devel(STATUS_COMPLETE);
    return 0;
}