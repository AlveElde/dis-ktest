#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>

#include "dis_send_receive.h"
#include "dis_verbs.h"

int create_socket(void)
{  
    //struct socket *sock = NULL;
    return 0;
}

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

// void print_cqe(struct cq_ctx *cq)
// {
//     /* Print Result Of Transmission */
//     pr_info("Responder Work Completions: %d", ret);
//     for(i = 0; i < cq->cqe_c; i++) {
//         switch (cq->cqe[i].opcode)
//         {
//         case IB_WC_SEND:
//             pr_info("CQE num: %d, Opcode: IB_WC_SEND, status: %s, message: %s",
//                     i, ib_wc_status_msg(cq->cqe[i].status), send_message);
//             break;
        
//         case IB_WC_RECV:
//             pr_info("CQE num: %d, Opcode: IB_WC_RECV, status: %s, message: %s",
//                     i, ib_wc_status_msg(cq->cqe[i].status), recv_message);
//             break;
//         default:
//             pr_info("CQE num: %d, Opcode: Unknown", i);
//             break;
//         }
//     }
// }

void cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
}

int send_receive_init(struct send_receive_ctx *ctx)
{
    int ret;
    //  int ret, pd_i, cq_i, qp_i, rqe_i, sqe_i, cqe_i;
    struct pd_ctx *pd;
    struct cq_ctx *cq;
    struct qp_ctx *qp;
    struct rqe_ctx *rqe;
    struct sqe_ctx *sqe;
    char send_message[DIS_MAX_MSG_LEN] = "Hello There!\n";
    char recv_message[DIS_MAX_MSG_LEN];
    
    pr_devel(DIS_STATUS_START);


    /* Query Device Port */
    ctx->dev.port_num    = 1;
    ret = verbs_query_port(&ctx->dev);
    if (ret) {
        return -42;
    }

    /* Create Protection Domain */
    // pd_i = ctx->pd_c;
    pd = &ctx->pd[ctx->pd_c];
    pd->ibdev = ctx->dev.ibdev;
    pd->flags = 0;
    ret = verbs_alloc_pd(pd);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Create a Completion Queue */
    // cq_i = ctx->cq_c;
    cq = &ctx->cq[ctx->cq_c];
    cq->ibdev            = ctx->dev.ibdev;
    cq->comp_handler     = cq_comp_handler,
    cq->event_handler    = NULL,
    cq->context          = NULL,

    cq->init_attr.cqe            = 10,
    cq->init_attr.comp_vector    = 0,
    cq->init_attr.flags          = 0,
    ret = verbs_create_cq(cq);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    ctx->cq_c++;

    /* Create Queue Pair 1*/
    // qp_i = ctx->qp_c;
    qp = &ctx->qp[ctx->qp_c];
    qp->ibpd    = ctx->pd[ctx->pd_c].ibpd;
    qp->send_cq = cq;
    qp->recv_cq = cq;

    qp->init_attr.qp_context     = NULL;
    qp->init_attr.send_cq        = cq->ibcq;
    qp->init_attr.recv_cq        = cq->ibcq;
    qp->init_attr.srq            = NULL;
    qp->init_attr.sq_sig_type    = IB_SIGNAL_ALL_WR;
    qp->init_attr.qp_type        = IB_QPT_RC;
    qp->init_attr.create_flags   = 0;

    qp->init_attr.cap.max_send_wr        = 10;
    qp->init_attr.cap.max_recv_wr        = 10;
	qp->init_attr.cap.max_send_sge       = 1;
	qp->init_attr.cap.max_recv_sge       = 1;
	qp->init_attr.cap.max_inline_data    = 0;
    ret = verbs_create_qp(qp);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    ctx->qp_c++;

    /* Transition Queue Pair to Initalize state */
    qp->attr.qp_state        = IB_QPS_INIT;
    qp->attr.qp_access_flags = IB_ACCESS_REMOTE_WRITE;
    qp->attr.qp_access_flags |= IB_ACCESS_REMOTE_READ;
    qp->attr.qp_access_flags |= IB_ACCESS_LOCAL_WRITE;
    qp->attr.pkey_index      = 0;
    qp->attr.port_num        = ctx->dev.port_num;

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_ACCESS_FLAGS;
    qp->attr_mask |= IB_QP_PKEY_INDEX;
    qp->attr_mask |= IB_QP_PORT;
    ret = verbs_modify_qp(qp);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Receive state */
    qp->attr.qp_state            = IB_QPS_RTR;
    qp->attr.path_mtu            = IB_MTU_4096;
    qp->attr.dest_qp_num         = 100;    // Dest QPN
    qp->attr.rq_psn              = 10;   // RQ Packet Sequence Number
    qp->attr.max_dest_rd_atomic  = 1;    // Responder Resources for RDMA read/atomic ops
    qp->attr.min_rnr_timer       = 1;    // Minimum RNR NAK

    qp->attr.ah_attr.sl          = 0;    // Service Level
    qp->attr.ah_attr.static_rate = 1;    // 
    qp->attr.ah_attr.type        = RDMA_AH_ATTR_TYPE_UNDEFINED;
    qp->attr.ah_attr.port_num    = ctx->dev.port_num;
    qp->attr.ah_attr.ah_flags    = 0; // Required by dis
    // qp->attr.ah_attr.ah_flags      = IB_AH_GRH; // Required by rxe

    qp->attr.ah_attr.grh.hop_limit   = 1;
    qp->attr.ah_attr.grh.sgid_index  = 1;
    qp->attr.ah_attr.grh.sgid_attr   = NULL;

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_AV;
    qp->attr_mask |= IB_QP_PATH_MTU;
    qp->attr_mask |= IB_QP_DEST_QPN;
    qp->attr_mask |= IB_QP_RQ_PSN;
    qp->attr_mask |= IB_QP_MAX_DEST_RD_ATOMIC;
    qp->attr_mask |= IB_QP_MIN_RNR_TIMER;
    ret = verbs_modify_qp(qp);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Send state */
    qp->attr.qp_state        = IB_QPS_RTS;
    qp->attr.timeout         = 10;   // Local ACK Timeout
    qp->attr.retry_cnt       = 10;   // Retry count 
    qp->attr.rnr_retry       = 10;   // RNR retry count
    qp->attr.sq_psn          = 10;   // SQ Packet Sequence Number
    qp->attr.max_rd_atomic   = 1;    // Number of Outstanding RDMA Read/atomic ops at destination.

    qp->attr_mask = IB_QP_STATE;
    qp->attr_mask |= IB_QP_TIMEOUT;
    qp->attr_mask |= IB_QP_RETRY_CNT;
    qp->attr_mask |= IB_QP_RNR_RETRY;
    qp->attr_mask |= IB_QP_SQ_PSN;
    qp->attr_mask |= IB_QP_MAX_QP_RD_ATOMIC;
    ret = verbs_modify_qp(qp);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    

    /* Set up connection to requester */
    //TODO: Set up socket based exchange of GID

    /* Create Receive Queue Element */
    // rqe_i = ctx->rqe_c;
    rqe = &qp->rqe[qp->rqe_c];
    rqe->ibqp               = qp->ibqp;
    rqe->ibbadwr            = NULL;

    rqe->ibwr.num_sge       = 1; // Number of segments to receive
    rqe->ibwr.wr_id         = 1; // Work Request ID
    rqe->ibwr.next          = NULL;
    rqe->ibwr.sg_list       = rqe->ibsge;
    
    rqe->ibsge[0].addr      = (uintptr_t)recv_message;
    rqe->ibsge[0].length    = DIS_MAX_MSG_LEN * sizeof(char);
    rqe->ibsge[0].lkey      = 123;

    /* Post Receive Queue Element */
    ret = verbs_post_recv(rqe);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    qp->recv_cq->expected_cqe++;
    qp->rqe_c++;

    /* Create Send Queue Element */
    // sqe_i = qp->sqe_c;
    sqe = &qp->sqe[qp->sqe_c];
    sqe->ibqp               = qp->ibqp;
    sqe->ibbadwr            = NULL;

	sqe->ibwr.opcode        = IB_WR_SEND;
	sqe->ibwr.send_flags    = IB_SEND_SIGNALED;
	sqe->ibwr.wr_id         = 2; // Work Request ID
    sqe->ibwr.num_sge       = 1; // Number of segments to send
    sqe->ibwr.sg_list       = sqe->ibsge;
    
    sqe->ibsge[0].addr      = (uintptr_t)send_message;
    sqe->ibsge[0].length    = DIS_MAX_MSG_LEN * sizeof(char);
    sqe->ibsge[0].lkey      = 456;

    // pr_info("Sending message: %s", (char*)(sqe->ibwr.sg_list[0].addr));

    /* Post Send Queue Element */
    ret = verbs_post_send(sqe);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    qp->send_cq->expected_cqe++;
    qp->sqe_c++;

    /* Poll Completion Queue */
    // i = ctx->cqe_c;
    // cqe = &cq->cqe[cq->cqe_c];
    // cqe->ibcq = cq->ibcq;
    ret = verbs_poll_cq(cq);
    if (ret < 0) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

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