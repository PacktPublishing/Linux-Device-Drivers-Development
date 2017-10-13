#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init fake_poll_dev_add(void)
{
    int inst_id = 0;
    pdev = platform_device_alloc("input-polled-button", inst_id);
    platform_device_add(pdev);
    pr_info("input-polled-button added");
    return 0;
}

static void __exit fake_poll_dev_put(void)
{
    pr_info("input-polled-button removed");
    platform_device_put(pdev);
}

module_init(fake_poll_dev_add);
module_exit(fake_poll_dev_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
