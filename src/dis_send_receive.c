#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>

#include "dis_send_receive.h"
#include "dis_ktest.h"

// int responder_get_gid_attr(struct gid_ctx *gid)
// {
//     pr_devel(DIS_STATUS_START);
//     gid->gid_attr = rdma_get_gid_attr(gid->ibdev, gid->port_num, gid->index);
//     if (!gid->gid_attr) {
//         pr_devel(DIS_STATUS_FAIL);
// 		return -42;
//     }
//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }

void print_cq(struct cq_ctx *cq)
{
    int i;
    struct ib_wc *cqe;
    pr_devel(DIS_STATUS_START);

    /* Print Result Of Transmission */
    for(i = 0; i < cq->cqe_c; i++) {
        cqe = &cq->cqe[i];
        switch (cqe->opcode)
        {
        case IB_WC_SEND:
            pr_info("CQE num: %d, Opcode: IB_WC_SEND, status: %s, wr_id: %d",
                    i, ib_wc_status_msg(cqe->status), (int)cqe->wr_id);
            break;
        
        case IB_WC_RECV:
            pr_info("CQE num: %d, Opcode: IB_WC_RECV, status: %s, wr_id: %d",
                    i, ib_wc_status_msg(cqe->status), (int)cqe->wr_id);
            break;
        default:
            pr_info("CQE num: %d, Opcode: Unknown", i);
            break;
        }
    }
    pr_devel(DIS_STATUS_COMPLETE);
}

int send_receive_init(struct send_receive_ctx *ctx)
{
    int ret, sleep_ms_count;
    struct dev_ctx *dev;
    struct pd_ctx *pd;
    struct cq_ctx *cq;
    struct qp_ctx *qp;
    struct rqe_ctx *rqe;
    struct sqe_ctx *sqe;
    struct sge_ctx *sge;
    pr_devel(DIS_STATUS_START);

    /* Query Device Port */
    dev = &ctx->dev;
    dev->port_num    = 1;
    ret = ib_query_port(dev->ibdev, dev->port_num, &dev->port_attr);
    if (ret) {
        return -42;
    }

    /* Create Protection Domain */
    pd = &ctx->pd[ctx->pd_c];
    pd->ibdev = dev->ibdev;
    pd->flags = 0;
    pd->ibpd = ib_alloc_pd(pd->ibdev, pd->flags);
    if (!pd->ibpd) {
        pr_devel(DIS_STATUS_FAIL);
		return -42;
    }

    /* Create a Completion Queue */
    cq = &ctx->cq[ctx->cq_c];
    cq->ibdev           = dev->ibdev;
    cq->comp_handler    = NULL,
    cq->event_handler   = NULL,
    cq->context         = NULL,

    cq->init_attr.cqe           = 10,
    cq->init_attr.comp_vector   = 0,
    cq->init_attr.flags         = 0,

    cq->ibcq = ib_create_cq(cq->ibdev,
                                cq->comp_handler,
                                cq->event_handler,
                                cq->context,
                                &cq->init_attr);
    if (!cq->ibcq) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    ctx->cq_c++;

    /* Create Queue Pair 1*/
    qp = &ctx->qp[ctx->qp_c];
    qp->ibpd    = ctx->pd[ctx->pd_c].ibpd;
    qp->send_cq = cq;
    qp->recv_cq = cq;

    qp->init_attr.qp_context    = NULL;
    qp->init_attr.send_cq       = cq->ibcq;
    qp->init_attr.recv_cq       = cq->ibcq;
    qp->init_attr.srq           = NULL;
    qp->init_attr.sq_sig_type   = IB_SIGNAL_ALL_WR;
    qp->init_attr.qp_type       = IB_QPT_RC;
    qp->init_attr.create_flags  = 0;

    qp->init_attr.cap.max_send_wr       = 10;
    qp->init_attr.cap.max_recv_wr       = 10;
	qp->init_attr.cap.max_send_sge      = 1;
	qp->init_attr.cap.max_recv_sge      = 1;
	qp->init_attr.cap.max_inline_data   = 0;

    qp->ibqp = ib_create_qp(qp->ibpd, &qp->init_attr);
    if (!qp->ibqp) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    ctx->qp_c++;

    /* Transition Queue Pair to Initalize state */
    qp->attr.qp_state           = IB_QPS_INIT;
    qp->attr.qp_access_flags    = IB_ACCESS_REMOTE_WRITE;
    qp->attr.qp_access_flags    |= IB_ACCESS_REMOTE_READ;
    qp->attr.qp_access_flags    |= IB_ACCESS_LOCAL_WRITE;
    qp->attr.pkey_index         = 0;
    qp->attr.port_num           = dev->port_num;

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_ACCESS_FLAGS;
    qp->attr_mask |= IB_QP_PKEY_INDEX;
    qp->attr_mask |= IB_QP_PORT;

    ret = ib_modify_qp(qp->ibqp, &qp->attr, qp->attr_mask);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Receive(RTR) state */
    qp->attr.qp_state               = IB_QPS_RTR;
    qp->attr.path_mtu               = IB_MTU_4096;
    qp->attr.dest_qp_num            = 100;
    qp->attr.rq_psn                 = 10;
    qp->attr.max_dest_rd_atomic     = 1;
    qp->attr.min_rnr_timer          = 1;

    qp->attr.ah_attr.sl             = 0;
    qp->attr.ah_attr.static_rate    = 1;
    qp->attr.ah_attr.type           = RDMA_AH_ATTR_TYPE_UNDEFINED;
    qp->attr.ah_attr.port_num       = dev->port_num;
    qp->attr.ah_attr.ah_flags       = 0; // Required by dis
    // qp->attr.ah_attr.ah_flags      = IB_AH_GRH; // Required by rxe

    qp->attr.ah_attr.grh.hop_limit  = 1;
    qp->attr.ah_attr.grh.sgid_index = 1;
    qp->attr.ah_attr.grh.sgid_attr  = NULL;

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_AV;
    qp->attr_mask |= IB_QP_PATH_MTU;
    qp->attr_mask |= IB_QP_DEST_QPN;
    qp->attr_mask |= IB_QP_RQ_PSN;
    qp->attr_mask |= IB_QP_MAX_DEST_RD_ATOMIC;
    qp->attr_mask |= IB_QP_MIN_RNR_TIMER;

    ret = ib_modify_qp(qp->ibqp, &qp->attr, qp->attr_mask);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Send(RTS) state */
    qp->attr.qp_state       = IB_QPS_RTS;
    qp->attr.timeout        = 10;
    qp->attr.retry_cnt      = 10;
    qp->attr.rnr_retry      = 10;
    qp->attr.sq_psn         = 10;
    qp->attr.max_rd_atomic  = 1;

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_TIMEOUT;
    qp->attr_mask |= IB_QP_RETRY_CNT;
    qp->attr_mask |= IB_QP_RNR_RETRY;
    qp->attr_mask |= IB_QP_SQ_PSN;
    qp->attr_mask |= IB_QP_MAX_QP_RD_ATOMIC;

    ret = ib_modify_qp(qp->ibqp, &qp->attr, qp->attr_mask);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    
    /* Set up connection to requester */
    //TODO: Set up socket based exchange of GID

    /* Initialize Send/Receive Segment */
    sge = &ctx->sge[ctx->sge_c];
    strncpy(sge->send_sge, "Hello There!", DIS_MAX_SGE_SIZE);
    sge->length     = DIS_MAX_SGE_SIZE;
    sge->lkey       = 123;
    ctx->sge_c++;

    /* Post Receive Queue Element */
    rqe = &qp->rqe[qp->rqe_c];
    rqe->ibqp               = qp->ibqp;
    rqe->ibbadwr            = NULL;

    rqe->ibwr.num_sge       = 1;
    rqe->ibwr.wr_id         = qp->rqe_c;
    rqe->ibwr.next          = NULL;
    rqe->ibwr.sg_list       = rqe->ibsge;
    
    rqe->ibsge[0].addr      = (uintptr_t)sge->recv_sge;
    rqe->ibsge[0].length    = sge->length;
    rqe->ibsge[0].lkey      = sge->lkey;

    ret = ib_post_recv(rqe->ibqp, &rqe->ibwr, &rqe->ibbadwr);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    qp->recv_cq->expected_cqe++;
    qp->rqe_c++;

    /* Post Send Queue Element */
    sqe = &qp->sqe[qp->sqe_c];
    sqe->ibqp               = qp->ibqp;
    sqe->ibbadwr            = NULL;

	sqe->ibwr.opcode        = IB_WR_SEND;
	sqe->ibwr.send_flags    = IB_SEND_SIGNALED;
    sqe->ibwr.num_sge       = 1;
	sqe->ibwr.wr_id         = qp->sqe_c;
    sqe->ibwr.sg_list       = sqe->ibsge;
    
    sqe->ibsge[0].addr      = (uintptr_t)sge->send_sge;
    sqe->ibsge[0].length    = sge->length;
    sqe->ibsge[0].lkey      = sge->lkey;

    ret = ib_post_send(sqe->ibqp, &sqe->ibwr, &sqe->ibbadwr);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    qp->send_cq->expected_cqe++;
    qp->sqe_c++;

    /* Poll Completion Queue */
    sleep_ms_count = 0;
    while(sleep_ms_count < DIS_POLL_TIMEOUT_MS) {
        ret = ib_poll_cq(cq->ibcq,
                            cq->expected_cqe - cq->cqe_c,
                            &cq->cqe[cq->cqe_c]);
        if (ret < 0) {
            pr_devel(DIS_STATUS_FAIL);
            return -42;
        }

        cq->cqe_c += ret;
        if(cq->cqe_c >= cq->expected_cqe) {
           break;
        }

        msleep(DIS_POLL_SLEEP_MS);
        sleep_ms_count += DIS_POLL_SLEEP_MS;
    }

    /* Print results */
    print_cq(cq);
    pr_devel("Received message: %s", sge->recv_sge);

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void send_receive_exit(struct send_receive_ctx *ctx) 
{
    int i;
    pr_devel(DIS_STATUS_START);

    for (i = 0; i < ctx->qp_c; i++) {
        ib_destroy_qp(ctx->qp[i].ibqp);
        pr_devel("ib_destroy_qp(ctx->qp[qp_i]): " DIS_STATUS_COMPLETE);
    }

    for (i = 0; i < ctx->cq_c; i++) {
        ib_destroy_cq(ctx->cq[i].ibcq);
        pr_devel("ib_destroy_cq: " DIS_STATUS_COMPLETE);
    }

    for (i = 0; i < ctx->pd_c; i++) {
        ib_dealloc_pd(ctx->pd[i].ibpd);
        pr_devel("ib_dealloc_pd: " DIS_STATUS_COMPLETE);
    }
    pr_devel(DIS_STATUS_COMPLETE);
}