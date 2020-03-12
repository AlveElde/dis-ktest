#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_requester.h"
#include "dis_verbs.h"

int requester_create_sg_list(struct sge_ctx sge[], struct sqe_ctx *sqe)
{
    int i;
    size_t sg_list_size;
    pr_devel(DIS_STATUS_START);

    //TODO: Check num sge against max
    sg_list_size = sizeof(struct ib_sge) * sqe->ibwr.num_sge;

    sqe->ibwr.sg_list = kzalloc(sg_list_size, GFP_KERNEL);
    if (!sqe->ibwr.sg_list) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    memset(sqe->ibwr.sg_list, 0, sg_list_size);

    for(i = 0; i < sqe->ibwr.num_sge; i++) {
        sge[i].ibsge = sqe->ibwr.sg_list + (sizeof(struct ib_sge) * i);
        sge[i].ibsge->addr      = sge[i].addr;
        sge[i].ibsge->length    = sge[i].length;
        sge[i].ibsge->lkey      = sge[i].lkey;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void requester_cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
}

int requester_send_request(struct requester_ctx *ctx)
{
    int ret, i;
    char message[DIS_MAX_MSG_LEN] = "Hello There!";
    
    pr_devel(DIS_STATUS_START);

    /* Create Protection Domain */
    memset(&ctx->pd, 0, sizeof(struct pd_ctx));
    ctx->pd.ibdev = ctx->dev.ibdev;
    ctx->pd.flags = 0;
    ret = verbs_alloc_pd(&ctx->pd);
    if (ret) {
        goto verbs_alloc_pd_err;
    }

    /* Create a Completion Queue */
    memset(&ctx->cq, 0, sizeof(struct cq_ctx));
    ctx->cq.ibdev               = ctx->dev.ibdev;
    ctx->cq.comp_handler        = requester_cq_comp_handler,
    ctx->cq.event_handler       = NULL,
    ctx->cq.context             = NULL,

    ctx->cq.init_attr.cqe           = 10,
    ctx->cq.init_attr.comp_vector   = 0,
    ctx->cq.init_attr.flags         = 0,
    ret = verbs_create_cq(&ctx->cq);
    if (ret) {
        goto verbs_create_cq_err;
    }

    /* Create Queue Pair 1*/
    memset(&ctx->qp1, 0, sizeof(struct qp_ctx));
    ctx->qp1.ibpd = ctx->pd.ibpd;

    ctx->qp1.init_attr.qp_context   = NULL;
    ctx->qp1.init_attr.send_cq      = ctx->cq.ibcq;
    ctx->qp1.init_attr.recv_cq      = ctx->cq.ibcq;
    ctx->qp1.init_attr.srq          = NULL;
    ctx->qp1.init_attr.sq_sig_type  = IB_SIGNAL_ALL_WR;
    ctx->qp1.init_attr.qp_type      = IB_QPT_RC;
    ctx->qp1.init_attr.create_flags = 0;

    ctx->qp1.init_attr.cap.max_send_wr      = 10;
    ctx->qp1.init_attr.cap.max_recv_wr      = 10;
	ctx->qp1.init_attr.cap.max_send_sge     = 1;
	ctx->qp1.init_attr.cap.max_recv_sge     = 1;
	ctx->qp1.init_attr.cap.max_inline_data  = 0;
    ret = verbs_create_qp(&ctx->qp1);
    if (ret) {
        goto verbs_create_qp_err;
    }

    /* Get DMA Memory Region */
    // mr.ibpd = pd.ibpd;
    // ret = verbs_alloc_mr(&mr);
    // if (ret) {
    //     goto get_mr_err;
    // }

    /* Create Send Queue Element */
    memset(&ctx->sqe, 0, sizeof(struct sqe_ctx));
    ctx->sqe.ibqp       = ctx->qp1.ibqp;
    ctx->sqe.ibbadwr    = NULL;

	ctx->sqe.ibwr.opcode        = IB_WR_SEND;
	ctx->sqe.ibwr.send_flags    = IB_SEND_SIGNALED;
	ctx->sqe.ibwr.wr_id         = 1;
    ctx->sqe.ibwr.num_sge       = 1;
    
    /* Create Segment List */
    ctx->sge[0].addr    = (uintptr_t)message;
    ctx->sge[0].length  = strlen(message);
    ctx->sge[0].lkey    = 1234;
    ret = requester_create_sg_list(ctx->sge, &ctx->sqe);
    if (ret) {
        goto requester_create_sg_list_err;
    }

    /* Set up connection to responder */

    /* Post Send Queue Element */
    ret = verbs_post_send(&ctx->sqe);
    if (ret) {
        goto verbs_post_send_err;
    }

    /* Poll Completion Queue */
    memset(&ctx->cqe, 0, sizeof(struct cqe_ctx));
    ctx->cqe.ibcq           = ctx->cq.ibcq;
    ctx->cqe.num_entries    = DIS_MAX_CQE;
    ret = verbs_poll_cq(&ctx->cqe, 10);
    if (ret) {
        goto verbs_poll_cq_err;
    }

    /* Print Result Of Transmission */
    for(i = 0; i < DIS_MAX_CQE; i++) {
        pr_info("Requester completed transmission %d with status: %s",
                i, ib_wc_status_msg(ctx->cqe.ibwc[i].status));
    }

verbs_poll_cq_err:
verbs_post_send_err:
    kfree(ctx->sqe.ibwr.sg_list);
    pr_devel("kfree(ibwr.sg_list): " DIS_STATUS_COMPLETE);

requester_create_sg_list_err:
    ib_destroy_qp(ctx->qp1.ibqp);
    pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);

// get_mr_err:
//     ib_destroy_qp(ctx->qp1.ibqp);
//     pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);

verbs_create_qp_err:
    ib_destroy_cq(ctx->cq.ibcq);
    pr_devel("ib_destroy_cq: " DIS_STATUS_COMPLETE);

verbs_create_cq_err:
    ib_dealloc_pd(ctx->pd.ibpd);
    pr_devel("ib_dealloc_pd: " DIS_STATUS_COMPLETE);

verbs_alloc_pd_err:
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int requester_test(struct ib_device *ibdev)
{
    int ret;
    struct requester_ctx ctx;
    pr_devel(DIS_STATUS_START);

    /* Query Device Port */
    ctx.dev.ibdev       = ibdev;
    ctx.dev.port_num    = 1;
    ret = verbs_query_port(&ctx.dev);
    if (ret) {
        return 0;
    }

    ret = requester_send_request(&ctx);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}