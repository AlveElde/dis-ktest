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
    int ret, i;
    char message[30] = "Hello There!";
    
    pr_devel(DIS_STATUS_START);

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

    /* Create Send Queue Element */
    memset(&ctx->sqe, 0, sizeof(struct sqe_ctx));
    ctx->sqe.ibqp       = ctx->qp1.ibqp;
    ctx->sqe.ibbadwr    = NULL;

	ctx->sqe.ibwr.opcode      = IB_WR_SEND;
	ctx->sqe.ibwr.send_flags  = IB_SEND_SIGNALED;
	ctx->sqe.ibwr.wr_id       = 1;
    ctx->sqe.ibwr.num_sge     = 1;
    
    /* Create Segment List */
    ctx->sge[0].addr     = (uintptr_t)message;
    ctx->sge[0].length   = strlen(message);
    ctx->sge[0].lkey     = 1234;
    ret = create_sg_list(ctx->sge, &ctx->sqe);
    if (ret) {
        goto create_sg_list_err;
    }

    /* Post Send Queue Element */
    ret = post_send(&ctx->sqe);
    if (ret) {
        goto post_send_err;
    }

    /* Poll Completion Queue */
    memset(&ctx->cqe, 0, sizeof(struct cqe_ctx));
    ctx->cqe.ibcq           = ctx->cq.ibcq;
    ctx->cqe.num_entries    = DIS_MAX_CQE;
    ret = poll_cq(&ctx->cqe);
    if (ret) {
        goto poll_cq_err;
    }

    /* Print Result Of Transmission */
    for(i = 0; i < DIS_MAX_CQE; i++) {
        pr_info("Requester completed transmission %d with status: %s",
                i, ib_wc_status_msg(ctx->cqe.ibwc[i].status));
    }

poll_cq_err:
post_send_err:
    kfree(ctx->sqe.ibwr.sg_list);
    pr_devel("kfree(ibwr.sg_list): " DIS_STATUS_COMPLETE);

create_sg_list_err:
    ib_destroy_qp(ctx->qp1.ibqp);
    pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);

// get_mr_err:
//     ib_destroy_qp(ctx->qp1.ibqp);
//     pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);

create_qp_err:
    ib_destroy_cq(ctx->cq.ibcq);
    pr_devel("ib_destroy_cq: " DIS_STATUS_COMPLETE);

create_cq_err:
    ib_dealloc_pd(ctx->pd.ibpd);
    pr_devel("ib_dealloc_pd: " DIS_STATUS_COMPLETE);

alloc_pd_err:
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int test_requester(struct ib_device *ibdev)
{
    int ret;
    struct requester_ctx ctx;
    pr_devel(DIS_STATUS_START);

    ctx.ibdev = ibdev;
    ret = send_request(&ctx);

    pr_devel(DIS_STATUS_COMPLETE);
    return ret;
}