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

void cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{
    return;
}

int send_receive_init(struct send_receive_ctx *ctx)
{
    int ret, i;
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
    ctx->pd.ibdev = ctx->dev.ibdev;
    ctx->pd.flags = 0;
    ret = verbs_alloc_pd(&ctx->pd);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Create a Completion Queue */
    ctx->cq.ibdev               = ctx->dev.ibdev;
    ctx->cq.comp_handler        = cq_comp_handler,
    ctx->cq.event_handler       = NULL,
    ctx->cq.context             = NULL,

    ctx->cq.init_attr.cqe           = 10,
    ctx->cq.init_attr.comp_vector   = 0,
    ctx->cq.init_attr.flags         = 0,
    ret = verbs_create_cq(&ctx->cq);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Create Queue Pair 1*/
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
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Get DMA Memory Region */
    // mr.ibpd = pd.ibpd;
    // ret = verbs_alloc_mr(&mr);
    // if (ret) {
    //     goto get_mr_err;
    // }

    /* Transition Queue Pair to Initalize state */
    ctx->qp1.attr.qp_state          = IB_QPS_INIT;
    ctx->qp1.attr.qp_access_flags   = IB_ACCESS_REMOTE_WRITE;
    ctx->qp1.attr.qp_access_flags   |= IB_ACCESS_REMOTE_READ;
    ctx->qp1.attr.qp_access_flags   |= IB_ACCESS_LOCAL_WRITE;
    ctx->qp1.attr.pkey_index        = 0;
    ctx->qp1.attr.port_num          = ctx->dev.port_num;

    ctx->qp1.attr_mask = IB_QP_STATE;
    ctx->qp1.attr_mask |= IB_QP_ACCESS_FLAGS;
    ctx->qp1.attr_mask |= IB_QP_PKEY_INDEX;
    ctx->qp1.attr_mask |= IB_QP_PORT;
    ret = verbs_modify_qp(&ctx->qp1);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Receive state */
    ctx->qp1.attr.qp_state              = IB_QPS_RTR;
    ctx->qp1.attr.path_mtu              = IB_MTU_4096;
    ctx->qp1.attr.dest_qp_num           = 100;    // Dest QPN
    ctx->qp1.attr.rq_psn                = 10;   // RQ Packet Sequence Number
    ctx->qp1.attr.max_dest_rd_atomic    = 1;    // Responder Resources for RDMA read/atomic ops
    ctx->qp1.attr.min_rnr_timer         = 1;    // Minimum RNR NAK

    ctx->qp1.attr.ah_attr.sl            = 0;    // Service Level
    ctx->qp1.attr.ah_attr.static_rate   = 1;    // 
    ctx->qp1.attr.ah_attr.type          = RDMA_AH_ATTR_TYPE_UNDEFINED;
    ctx->qp1.attr.ah_attr.port_num      = ctx->dev.port_num;
    ctx->qp1.attr.ah_attr.ah_flags      = 0; // Required by dis
    // ctx->qp1.attr.ah_attr.ah_flags      = IB_AH_GRH; // Required by rxe

    ctx->qp1.attr.ah_attr.grh.hop_limit     = 1;
    ctx->qp1.attr.ah_attr.grh.sgid_index    = 1;
    ctx->qp1.attr.ah_attr.grh.sgid_attr     = NULL;

    ctx->qp1.attr_mask = IB_QP_STATE;
    ctx->qp1.attr_mask |= IB_QP_AV;
    ctx->qp1.attr_mask |= IB_QP_PATH_MTU;
    ctx->qp1.attr_mask |= IB_QP_DEST_QPN;
    ctx->qp1.attr_mask |= IB_QP_RQ_PSN;
    ctx->qp1.attr_mask |= IB_QP_MAX_DEST_RD_ATOMIC;
    ctx->qp1.attr_mask |= IB_QP_MIN_RNR_TIMER;
    ret = verbs_modify_qp(&ctx->qp1);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Transition Queue Pair to Ready To Send state */
    ctx->qp1.attr.qp_state      = IB_QPS_RTS;
    ctx->qp1.attr.timeout       = 10;   // Local ACK Timeout
    ctx->qp1.attr.retry_cnt     = 10;   // Retry count 
    ctx->qp1.attr.rnr_retry     = 10;   // RNR retry count
    ctx->qp1.attr.sq_psn        = 10;   // SQ Packet Sequence Number
    ctx->qp1.attr.max_rd_atomic = 1;    // Number of Outstanding RDMA Read/atomic ops at destination.

    ctx->qp1.attr_mask = IB_QP_STATE;
    ctx->qp1.attr_mask |= IB_QP_TIMEOUT;
    ctx->qp1.attr_mask |= IB_QP_RETRY_CNT;
    ctx->qp1.attr_mask |= IB_QP_RNR_RETRY;
    ctx->qp1.attr_mask |= IB_QP_SQ_PSN;
    ctx->qp1.attr_mask |= IB_QP_MAX_QP_RD_ATOMIC;
    ret = verbs_modify_qp(&ctx->qp1);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Set up connection to requester */
    //TODO: Set up socket based exchange of GID

    /* Create Receive Queue Element */
    ctx->rqe.ibqp       = ctx->qp1.ibqp;
    ctx->rqe.ibbadwr    = NULL;

    ctx->rqe.ibwr.num_sge       = 1; // Number of segments to receive
    ctx->rqe.ibwr.wr_id         = 1; // Work Request ID
    ctx->rqe.ibwr.next          = NULL;
    ctx->rqe.ibwr.sg_list       = ctx->rqe.ibsge;
    
    ctx->rqe.ibsge[0].addr      = (uintptr_t)recv_message;
    ctx->rqe.ibsge[0].length    = DIS_MAX_MSG_LEN * sizeof(char);
    ctx->rqe.ibsge[0].lkey      = 123;

    /* Post Receive Queue Element */
    ret = verbs_post_recv(&ctx->rqe);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Create Send Queue Element */
    ctx->sqe.ibqp       = ctx->qp1.ibqp;
    ctx->sqe.ibbadwr    = NULL;

	ctx->sqe.ibwr.opcode        = IB_WR_SEND;
	ctx->sqe.ibwr.send_flags    = IB_SEND_SIGNALED;
	ctx->sqe.ibwr.wr_id         = 2; // Work Request ID
    ctx->sqe.ibwr.num_sge       = 1; // Number of segments to send
    ctx->sqe.ibwr.sg_list       = ctx->sqe.ibsge;
    
    ctx->sqe.ibsge[0].addr      = (uintptr_t)send_message;
    ctx->sqe.ibsge[0].length    = DIS_MAX_MSG_LEN * sizeof(char);
    ctx->sqe.ibsge[0].lkey      = 456;

    pr_info("Sending message: %s", (char*)(ctx->sqe.ibwr.sg_list[0].addr));

    /* Post Send Queue Element */
    ret = verbs_post_send(&ctx->sqe);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    // ssleep(30);
    // return 0;

    /* Poll Completion Queue */
    ctx->cqe.ibcq           = ctx->cq.ibcq;
    ctx->cqe.num_entries    = DIS_MAX_CQE;
    ret = verbs_poll_cq(&ctx->cqe, 30);
    if (ret < 0) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }

    /* Print Result Of Transmission */
    pr_info("Responder Work Completions: %d", ret);
    for(i = 0; i < ret; i++) {
        pr_info("Responder received transmission %d with status: %s",
                i+1, ib_wc_status_msg(ctx->cqe.ibwc[i].status));
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

void send_receive_exit(struct send_receive_ctx *ctx) 
{
    pr_devel(DIS_STATUS_START);
    if (ctx->qp1.ibqp) {
        ib_destroy_qp(ctx->qp1.ibqp);
        pr_devel("ib_destroy_qp(ctx->qp1): " DIS_STATUS_COMPLETE);
    }
    if (ctx->cq.ibcq) {
        ib_destroy_cq(ctx->cq.ibcq);
        pr_devel("ib_destroy_cq: " DIS_STATUS_COMPLETE);
    }
    if (ctx->pd.ibpd) {
        ib_dealloc_pd(ctx->pd.ibpd);
        pr_devel("ib_dealloc_pd: " DIS_STATUS_COMPLETE);
    }
    pr_devel(DIS_STATUS_COMPLETE);
}