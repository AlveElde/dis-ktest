#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "dis_ktest.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

struct test_context {
    // Device
    struct ib_device *ibdev;
    // PD
    int pd_flags;
    struct ib_pd *ibpd;

    // CQ
    void (*comp_handler)(struct ib_cq *ibcq, void *cq_context);
    void (*event_handler)(struct ib_event *, void *);
    void *cq_context;
    struct ib_cq_init_attr *cq_attr;
    struct ib_cq *ibcq;

    // QP
    struct ib_qp_attr attr;
    struct ib_qp_init_attr init_attr;
    struct ib_qp *ibqp;
};


void cq_comp_handler(struct ib_cq *ibcq, void *cq_context)
{

}

static int create_cq(struct test_context *ctx)
{
    ctx->comp_handler = cq_comp_handler;
    ctx->event_handler = NULL;
    ctx->cq_context = NULL;
    ctx->cq_attr = 0;
    ctx->ibcq = ib_create_cq(ctx->ibdev, 
                                ctx->comp_handler, 
                                ctx->event_handler, 
                                ctx->cq_context,
                                ctx->cq_attr);
    if (!ctx->ibcq) {
        printk(KERN_ERR "create_cq failed!.\n");
            return -42;
    }
    printk(KERN_INFO "create_cq complete.\n");
    return 0;

}

static int alloc_pd(struct test_context *ctx)
{
    printk(KERN_INFO "alloc_pd start.\n");
    ctx->pd_flags = 0;
    ctx->ibpd = ib_alloc_pd(ctx->ibdev, ctx->pd_flags);
    if (!ctx->ibpd) {
        printk(KERN_ERR "alloc_pd failed!.\n");
		return -42;
    }
    printk(KERN_INFO "alloc_pd complete.\n");
    return 0;
}


static int perform_test(struct test_context *ctx)
{
    int ret;
    printk(KERN_INFO "perform_test start.\n");

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

create_cq_err:
    ib_dealloc_pd(ctx->ibpd);
    printk(KERN_ERR "ib_dealloc_pd complete.\n");

alloc_pd_err:
    printk(KERN_INFO "perform_test complete.\n");
    return 0;
}




static void dis_ktest_add(struct ib_device *ibdev)
{
    int ret;
    struct test_context ctx;
    printk(KERN_INFO "dis_ktest_add start.\n");
    ctx.ibdev = ibdev;
    ret = perform_test(&ctx);
    printk(KERN_INFO "dis_ktest_add complete.\n");
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    printk(KERN_INFO "dis_ktest_remove start.\n");


    printk(KERN_INFO "dis_ktest_remove complete.\n");
}

static struct ib_client disibclient = {
	.name   = "dis-ktest",
    .add    = dis_ktest_add,
	.remove = dis_ktest_remove,
};


static int __init dis_ktest_init(void)
{
    int ret;

    printk(KERN_INFO "dis_ktest_init start.\n");

	ret = ib_register_client(&disibclient);
	if (ret) {
        printk(KERN_INFO "ib_register_client failed!.\n");
		return -42;
    }


    printk(KERN_INFO "dis_ktest_init complete.\n");
    return 0;
}

static void __exit dis_ktest_exit(void)
{
    printk(KERN_INFO "dis_ktest_exit start.\n");

    ib_unregister_client(&disibclient);

    printk(KERN_INFO "dis_ktest_exit complete.\n");
}

module_init(dis_ktest_init);
module_exit(dis_ktest_exit);