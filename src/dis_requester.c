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

int test_requester(struct ib_device *ibdev)
{
    int ret;
    char message[30] = "Hello There!";
    struct pd_ctx pd;
    struct cq_ctx cq;
    struct qp_ctx qp1;
    struct sge_ctx sge[TOTAL_SGE];
    struct send_wr_ctx send_wr;

    pr_devel(STATUS_START);

    /* Create Protection Domain */
    memset(&pd, 0, sizeof(struct pd_ctx));
    pd.ibdev = ibdev;
    pd.flags = 0;
    ret = alloc_pd(&pd);
    if (ret) {
        goto alloc_pd_err;
    }

    /* Create a Completion Queue */
    memset(&cq, 0, sizeof(struct cq_ctx));
    cq.ibdev            = ibdev;
    cq.comp_handler     = cq_comp_handler,
    cq.event_handler    = NULL,
    cq.context          = NULL,

    cq.attr.cqe         = 10,
    cq.attr.comp_vector = 0,
    cq.attr.flags       = 0,
    ret = create_cq(&cq);
    if (ret) {
        goto create_cq_err;
    }

    /* Create Queue Pair 1*/
    memset(&qp1, 0, sizeof(struct qp_ctx));
    qp1.ibpd = pd.ibpd;

    qp1.attr.qp_context     = NULL;
    qp1.attr.send_cq        = cq.ibcq;
    qp1.attr.recv_cq        = cq.ibcq;
    qp1.attr.srq            = NULL;
    qp1.attr.sq_sig_type    = IB_SIGNAL_ALL_WR;
    qp1.attr.qp_type        = IB_QPT_RC;
    qp1.attr.create_flags   = 0;

    qp1.attr.cap.max_send_wr        = 10;
    qp1.attr.cap.max_recv_wr        = 10;
	qp1.attr.cap.max_send_sge       = 1;
	qp1.attr.cap.max_recv_sge       = 1;
	qp1.attr.cap.max_inline_data    = 0;
    ret = create_qp(&qp1);
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
    sge[0].addr     = (uintptr_t)message;
    sge[0].length   = strlen(message);
    sge[0].lkey     = 1234;

    /* Create Send Work Request */
    memset(&send_wr, 0, sizeof(struct send_wr_ctx));
    send_wr.ibqp        = qp1.ibqp;
    send_wr.ibbadwr     = NULL;
	send_wr.opcode      = IB_WR_SEND;
    send_wr.num_sge     = TOTAL_SGE;
	send_wr.send_flags  = IB_SEND_SIGNALED;
	send_wr.wr_id       = 1;

    ret = create_send_wr(&send_wr, sge);
    if (ret) {
        goto create_wr_err;
    }

    /* Post Send Request */



    kfree(send_wr.ibwr->sg_list);
    pr_devel("kfree(ibwr.sg_list): " STATUS_COMPLETE);
    kfree(send_wr.ibwr);
    pr_devel("kfree(ibwr): " STATUS_COMPLETE);

create_wr_err:
    ib_destroy_qp(qp1.ibqp);
    pr_devel("ib_destroy_qp(qp1): " STATUS_COMPLETE);

// get_mr_err:
//     ib_destroy_qp(qp1.ibqp);
//     pr_devel("ib_destroy_qp(qp1): " STATUS_COMPLETE);

create_qp_err:
    ib_destroy_cq(cq.ibcq);
    pr_devel("ib_destroy_cq: " STATUS_COMPLETE);

create_cq_err:
    ib_dealloc_pd(pd.ibpd);
    pr_devel("ib_dealloc_pd: " STATUS_COMPLETE);

alloc_pd_err:
    pr_devel(STATUS_COMPLETE);
    return 0;
}