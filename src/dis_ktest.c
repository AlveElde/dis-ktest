#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include "dis_ktest.h"

MODULE_DESCRIPTION("Testing facilities for the dis-kverbs module");
MODULE_AUTHOR("Alve Elde");
MODULE_LICENSE("GPL");

static int __init dis_init_ktest(void)
{
    printk(KERN_INFO "dis_init_ktest start.\n");
    printk(KERN_INFO "dis_init_ktest complete.\n");
    return 0;
}

static void __exit dis_exit_ktest(void)
{
    printk(KERN_INFO "dis_exit_ktest start.\n");
    printk(KERN_INFO "dis_exit_ktest complete.\n");
}

module_init(dis_init_ktest);
module_exit(dis_exit_ktest);