#include "pr_fmt.h"

#include <linux/types.h>
#include <linux/string.h>

#include "dis_ktest.h"

int test_responder(struct ib_device *ibdev)
{
    pr_devel(STATUS_START);
    pr_devel(STATUS_COMPLETE);
    return 0;
}