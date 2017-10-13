#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init platform_dummy_char_add(void)
{
    int inst_id = 0; /* instance unique ID: base address would be a good choice */
    pdev = platform_device_alloc("fake-gpio-chip", inst_id);
    platform_device_add(pdev);
    pr_info("fake-gpio-chip added\n");
    return 0;
}

static void __exit fplatform_dummy_char_put(void)
{
    pr_info("fake-gpio-chip removed\n");
	platform_device_put(pdev);
}

module_init(platform_dummy_char_add);
module_exit(fplatform_dummy_char_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
