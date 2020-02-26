#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_responder.h"
#include "dis_verbs.h"

int responder_create_sg_list(struct sge_ctx sge[], struct rqe_ctx *rqe)
{
    int i;
    size_t sg_list_size;
    pr_devel(DIS_STATUS_START);

    //TODO: Check num sge against max
    sg_list_size = sizeof(struct ib_sge) * rqe->ibwr.num_sge;

    rqe->ibwr.sg_list = kzalloc(sg_list_size, GFP_KERNEL);
    if (!rqe->ibwr.sg_list) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    memset(rqe->ibwr.sg_list, 0, sg_list_size);

    for(i = 0; i < rqe->ibwr.num_sge; i++) {
        sge[i].ibsge = rqe->ibwr.sg_list + (sizeof(struct ib_sge) * i);
        sge[i].ibsge->addr      = sge[i].addr;
        sge[i].ibsge->length    = sge[i].length;
        sge[i].ibsge->lkey      = sge[i].lkey;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}


void responder_cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
}

int responder_receive_request(struct responder_ctx *ctx)
{
    int ret, i;
    char message[DIS_MAX_MSG_LEN];
    
    pr_devel(DIS_STATUS_START);

    /* Create Protection Domain */
    memset(&ctx->pd, 0, sizeof(struct pd_ctx));
    ctx->pd.ibdev = ctx->ibdev;
    ctx->pd.flags = 0;
    ret = verbs_alloc_pd(&ctx->pd);
    if (ret) {
        goto verbs_alloc_pd_err;
    }

    /* Create a Completion Queue */
    memset(&ctx->cq, 0, sizeof(struct cq_ctx));
    ctx->cq.ibdev               = ctx->ibdev;
    ctx->cq.comp_handler        = responder_cq_comp_handler,
    ctx->cq.event_handler       = NULL,
    ctx->cq.context             = NULL,

    ctx->cq.attr.cqe            = 10,
    ctx->cq.attr.comp_vector    = 0,
    ctx->cq.attr.flags          = 0,
    ret = verbs_create_cq(&ctx->cq);
    if (ret) {
        goto verbs_create_cq_err;
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
    ret = verbs_create_qp(&ctx->qp1);
    if (ret) {
        goto verbs_create_qp_err;
    }

    /* Create Receive Queue Element */
    memset(&ctx->rqe, 0, sizeof(struct rqe_ctx));
    ctx->rqe.ibqp       = ctx->qp1.ibqp;
    ctx->rqe.ibbadwr    = NULL;

    ctx->rqe.ibwr.num_sge       = 1;
    ctx->rqe.ibwr.wr_id         = 1;
    ctx->rqe.ibwr.next          = NULL;
    
    /* Create Segment List */
    ctx->sge[0].addr    = (uintptr_t)message;
    ctx->sge[0].length  = DIS_MAX_MSG_LEN;
    ctx->sge[0].lkey    = 1234;
    ret = responder_create_sg_list(ctx->sge, &ctx->rqe);
    if (ret) {
        goto responder_create_sg_list_err;
    }

    /* Post Receive Queue Element */
    ret = verbs_post_recv(&ctx->rqe);
    if (ret) {
        goto verbs_post_recv_err;
    }

// verbs_poll_cq_err:
verbs_post_recv_err:
    kfree(ctx->rqe.ibwr.sg_list);
    pr_devel("kfree(ibwr.sg_list): " DIS_STATUS_COMPLETE);

responder_create_sg_list_err:
    ib_destroy_qp(ctx->qp1.ibqp);
    pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);

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

int responder_test(struct ib_device *ibdev)
{
    int ret;
    struct responder_ctx ctx;
    pr_devel(DIS_STATUS_START);

    ctx.ibdev = ibdev;
    ret = responder_receive_request(&ctx);

    pr_devel(DIS_STATUS_COMPLETE);
    return ret;
}