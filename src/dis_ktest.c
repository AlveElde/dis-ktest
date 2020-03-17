#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "dis_send_receive.h"
#include "dis_verbs.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static void dis_ktest_add(struct ib_device *ibdev);
static void dis_ktest_remove(struct ib_device *ib_device, void *client_data);

static struct send_receive_ctx ctx;
static struct ib_client disibclient = {
	.name   = "dis-ktest",
    .add    = dis_ktest_add,
	.remove = dis_ktest_remove,
};

static void dis_ktest_add(struct ib_device *ibdev)
{
    pr_devel(DIS_STATUS_START);

    memset(&ctx, 0, sizeof(struct send_receive_ctx));
    ctx.dev.ibdev = ibdev;
    send_receive_init(&ctx);
    ib_set_client_data(ibdev, &disibclient, (void *)&ctx);

    pr_devel(DIS_STATUS_COMPLETE);
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    pr_devel(DIS_STATUS_START);

    send_receive_exit((struct send_receive_ctx*)client_data);
    
    pr_devel(DIS_STATUS_COMPLETE);
}

static int __init dis_ktest_init(void)
{
    int ret;
    pr_devel(DIS_STATUS_START);

	ret = ib_register_client(&disibclient);
	if (ret) {
        pr_err(DIS_STATUS_FAIL);
		return -42;
    }

    pr_devel(DIS_STATUS_COMPLETE);
    return 0;
}

static void __exit dis_ktest_exit(void)
{
    pr_devel(DIS_STATUS_START);

    ib_unregister_client(&disibclient);

    pr_devel(DIS_STATUS_COMPLETE);
}

module_init(dis_ktest_init);
module_exit(dis_ktest_exit);