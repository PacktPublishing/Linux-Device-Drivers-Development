#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init fake_pwm_add(void)
{
    int inst_id = 0;
    pdev = platform_device_alloc("fake-pwm", inst_id);
    platform_device_add(pdev);
    pr_info("fake-pwm added");
    return 0;
}

static void __exit fake_pwm_put(void)
{
    pr_info("fake-pwm removed");
    platform_device_put(pdev);
}

module_init(fake_pwm_add);
module_exit(fake_pwm_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
