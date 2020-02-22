#define DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": fn: %s, ln: %d: " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "dis_ktest.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

#define STATUS_START    "Started.\n"
#define STATUS_COMPLETE "Completed.\n"
#define STATUS_FAIL     "Failed.\n"


void qp_event_handler(struct ib_event *ibevent, void *qp_context)
{
    return;
}

static int create_qp(struct qp_ctx *qp)
{

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

static int create_cq(struct cq_ctx *cq)
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

static int alloc_pd(struct pd_ctx *pd)
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

static int perform_test(struct ib_device *ibdev)
{
    int ret;
    struct pd_ctx pd;
    struct cq_ctx cq;
    struct qp_ctx qp1, qp2;
    pr_devel(STATUS_START);

    /* Create Protection Domain */
    pd.ibdev = ibdev;
    pd.flags = 0;
    ret = alloc_pd(&pd);
    if (ret) {
        goto alloc_pd_err;
    }

    /* Create a Completion Queue */
    cq.ibdev = ibdev;
    cq.comp_handler = cq_comp_handler,
    cq.event_handler = NULL,
    cq.context = NULL,
    cq.attr.cqe = 10,
    cq.attr.comp_vector = 0,
    cq.attr.flags = 0,
    ret = create_cq(&cq);
    if (ret) {
        goto create_cq_err;
    }


    // ib_query_qp(ibqp, &attr, IB_QP_CAP, &init_attr);

    // ret = ib_post_send(ibqp, ibwq, ibbadwr);
    // if(ret) {

    // }

    ib_destroy_cq(cq.ibcq);
    pr_devel("ib_destroy_cq " STATUS_COMPLETE);

create_cq_err:
    ib_dealloc_pd(pd.ibpd);
    pr_devel("ib_dealloc_pd " STATUS_COMPLETE);

alloc_pd_err:
    pr_devel(STATUS_COMPLETE);
    return 0;
}

static void dis_ktest_add(struct ib_device *ibdev)
{
    int ret;
    pr_devel(STATUS_START);

    ret = perform_test(ibdev);

    pr_devel(STATUS_COMPLETE);
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    pr_devel(STATUS_COMPLETE);
}

static struct ib_client disibclient = {
	.name   = "dis-ktest",
    .add    = dis_ktest_add,
	.remove = dis_ktest_remove,
};

static int __init dis_ktest_init(void)
{
    int ret;

    pr_info(STATUS_START);

	ret = ib_register_client(&disibclient);
	if (ret) {
        pr_info(STATUS_FAIL);
		return -42;
    }

    pr_info(STATUS_COMPLETE);
    return 0;
}

static void __exit dis_ktest_exit(void)
{
    pr_info(STATUS_START);

    ib_unregister_client(&disibclient);

    pr_info(STATUS_COMPLETE);
}

module_init(dis_ktest_init);
module_exit(dis_ktest_exit);