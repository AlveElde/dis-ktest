#include "pr_fmt.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "dis_requester.h"
#include "dis_responder.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static bool is_responder = true;

module_param(is_responder, bool, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

static void dis_ktest_add(struct ib_device *ibdev)
{
    int ret;
    pr_devel(DIS_STATUS_START);

    if(is_responder) {
        ret = responder_test(ibdev);
    } else {
        ret = requester_test(ibdev);
    }

    pr_devel(DIS_STATUS_COMPLETE);
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    pr_devel(DIS_STATUS_COMPLETE);
}

static struct ib_client disibclient = {
	.name   = "dis-ktest",
    .add    = dis_ktest_add,
	.remove = dis_ktest_remove,
};

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