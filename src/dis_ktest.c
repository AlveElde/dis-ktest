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

void cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{

}

void cq_event_handler(struct ib_event *ibevent, void *cq_context)
{

}

static int create_cq(struct test_context *ctx)
{
    pr_devel(STATUS_START);
    ctx->comp_handler = cq_comp_handler;
    ctx->event_handler = NULL;
    ctx->cq_context = NULL;
    ctx->cq_attr.cqe = 10; // Length of queue
    ctx->cq_attr.comp_vector = 0;
    ctx->cq_attr.flags = 0;
    ctx->ibcq = ib_create_cq(ctx->ibdev,
                                ctx->comp_handler,
                                ctx->event_handler,
                                ctx->cq_context,
                                &ctx->cq_attr);
    if (!ctx->ibcq) {
        pr_devel(STATUS_FAIL);
            return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;

}

static int alloc_pd(struct test_context *ctx)
{
    pr_devel(STATUS_START);
    ctx->pd_flags = 0;
    ctx->ibpd = ib_alloc_pd(ctx->ibdev, ctx->pd_flags);
    if (!ctx->ibpd) {
        pr_devel(STATUS_FAIL);
		return -42;
    }
    pr_devel(STATUS_COMPLETE);
    return 0;
}

static int perform_test(struct test_context *ctx)
{
    int ret;
    pr_devel(STATUS_START);

    ret = alloc_pd(ctx);
    if (ret) {
        goto alloc_pd_err;
    }

    // ret = create_cq(ctx);
    // if (ret) {
    //     goto create_cq_err;
    // }

    // ibqp = ib_create_qp(ibpd, &init_attr);

    // ib_query_qp(ibqp, &attr, IB_QP_CAP, &init_attr);

    // ret = ib_post_send(ibqp, ibwq, ibbadwr);
    // if(ret) {

    // }

    //ib_destroy_cq(ctx->ibcq);
    pr_devel("ib_destroy_cq " STATUS_COMPLETE);

create_cq_err:
    ib_dealloc_pd(ctx->ibpd);
    pr_devel("ib_dealloc_pd " STATUS_COMPLETE);

alloc_pd_err:
    pr_devel(STATUS_COMPLETE);
    return 0;
}

static void dis_ktest_add(struct ib_device *ibdev)
{
    int ret;
    struct test_context ctx;
    pr_devel(STATUS_START);

    ctx.ibdev = ibdev;
    ret = perform_test(&ctx);

    pr_devel(STATUS_COMPLETE);
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    pr_devel(STATUS_START);


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