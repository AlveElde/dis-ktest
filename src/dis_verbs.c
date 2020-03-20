#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "dis_ktest.h"

//TODO: This file has become obselete, may be deleted.

int verbs_query_port(struct dev_ctx *dev)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    ret = ib_query_port(dev->ibdev, dev->port_num, &dev->port_attr);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
		return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_alloc_pd(struct pd_ctx *pd)
{
    pr_devel(DIS_STATUS_START);
    pd->ibpd = ib_alloc_pd(pd->ibdev, pd->flags);
    if (!pd->ibpd) {
        pr_devel(DIS_STATUS_FAIL);
		return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_create_cq(struct cq_ctx *cq)
{
    pr_devel(DIS_STATUS_START);
    cq->ibcq = ib_create_cq(cq->ibdev,
                                cq->comp_handler,
                                cq->event_handler,
                                cq->context,
                                &cq->init_attr);
    if (!cq->ibcq) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_create_qp(struct qp_ctx *qp)
{
    pr_devel(DIS_STATUS_START);
    qp->ibqp = ib_create_qp(qp->ibpd, &qp->init_attr);
    if (!qp->ibqp) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_modify_qp(struct qp_ctx *qp)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    ret = ib_modify_qp(qp->ibqp, &qp->attr, qp->attr_mask);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_post_send(struct sqe_ctx *sqe)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    ret = ib_post_send(sqe->ibqp, &sqe->ibwr, &sqe->ibbadwr);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_post_recv(struct rqe_ctx *rqe)
{
    int ret;
    pr_devel(DIS_STATUS_START);
    ret = ib_post_recv(rqe->ibqp, &rqe->ibwr, &rqe->ibbadwr);
    if (ret) {
        pr_devel(DIS_STATUS_FAIL);
        return -42;
    }
    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

int verbs_poll_cq(struct cq_ctx *cq)
{
    int ret, sleep_ms_count;
    pr_devel(DIS_STATUS_START);

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
            pr_devel(DIS_STATUS_COMPLETE);
            return 0;
        }

        msleep(DIS_POLL_SLEEP_MS);
        sleep_ms_count += DIS_POLL_SLEEP_MS;
    }

    pr_devel(DIS_STATUS_FAIL);
    return -42;
}

// int verbs_alloc_mr(struct mr_ctx *mr)
// {
//     pr_devel(DIS_STATUS_START);
//     mr->ibmr = ib_alloc_mr(&mr->ibpd, IB_ACCESS_REMOTE_READ |
//                             IB_ACCESS_REMOTE_WRITE |
//                             IB_ACCESS_LOCAL_WRITE);
//     if (!mr->ibmr) {
//         pr_devel(DIS_STATUS_FAIL);
//         return -42;
//     }
//     pr_devel(DIS_STATUS_COMPLETE);
//     return 0;
// }
