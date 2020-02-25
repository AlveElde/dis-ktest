#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_verbs.h"

void requester_cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
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
    ctx->cq.comp_handler        = requester_cq_comp_handler,
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

    ctx->qp1.attr.qp_context    = NULL;
    ctx->qp1.attr.send_cq       = ctx->cq.ibcq;
    ctx->qp1.attr.recv_cq       = ctx->cq.ibcq;
    ctx->qp1.attr.srq           = NULL;
    ctx->qp1.attr.sq_sig_type   = IB_SIGNAL_ALL_WR;
    ctx->qp1.attr.qp_type       = IB_QPT_RC;
    ctx->qp1.attr.create_flags  = 0;

    ctx->qp1.attr.cap.max_send_wr       = 10;
    ctx->qp1.attr.cap.max_recv_wr       = 10;
	ctx->qp1.attr.cap.max_send_sge      = 1;
	ctx->qp1.attr.cap.max_recv_sge      = 1;
	ctx->qp1.attr.cap.max_inline_data   = 0;
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
    memset(&ctx->sge[0], 0, sizeof(struct sge_ctx));
    ctx->sge[0].addr     = (uintptr_t)message;
    ctx->sge[0].length   = strlen(message);
    ctx->sge[0].lkey     = 1234;

    /* Create Send Work Request */
    memset(&ctx->wr, 0, sizeof(struct send_wr_ctx));
	ctx->wr.opcode      = IB_WR_SEND;
    ctx->wr.num_sge     = TOTAL_SGE;
	ctx->wr.send_flags  = IB_SEND_SIGNALED;
	ctx->wr.wr_id       = 1;
    ret = create_send_wr(&ctx->wr, ctx->sge);
    if (ret) {
        goto create_wr_err;
    }

    /* Post Send Queue Element */
    memset(&ctx->sqe, 0, sizeof(struct sqe_ctx));
    ctx->sqe.ibqp       = ctx->qp1.ibqp;
    ctx->sqe.ibwr       = ctx->wr.ibwr;
    ctx->sqe.ibbadwr    = NULL;
    ret = post_send(&ctx->sqe);
    if (ret) {
        goto post_send_err;
    }

    /* Poll Completion Queue */


post_send_err:
    kfree(ctx->sqe.ibwr->sg_list);
    pr_devel("kfree(ibwr.sg_list): " STATUS_COMPLETE);
    kfree(ctx->sqe.ibwr);
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
    int ret;
    struct requester_ctx ctx;
    pr_devel(STATUS_START);

    ctx.ibdev = ibdev;
    ret = send_request(&ctx);

    pr_devel(STATUS_COMPLETE);
    return ret;
}