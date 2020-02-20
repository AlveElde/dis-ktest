#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "dis_ktest.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

struct ib_device *ibdev;
struct ib_pd *ibpd;


static void dis_ktest_add(struct ib_device *device)
{
    printk(KERN_INFO "dis_ktest_add start.\n");

    ibdev = device;

    ibpd = ib_alloc_pd(device, 0);
    if (IS_ERR(ibpd)) {
        printk(KERN_ERR "ib_alloc_pd failed!.\n");
		return;
    }
    printk(KERN_ERR "ib_alloc_pd complete.\n");

    printk(KERN_INFO "dis_ktest_add complete.\n");
}

static void dis_ktest_remove(struct ib_device *ib_device, void *client_data)
{
    printk(KERN_INFO "dis_ktest_remove start.\n");

    ib_dealloc_pd(ibpd);

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