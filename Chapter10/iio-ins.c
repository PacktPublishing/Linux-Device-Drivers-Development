#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init fake_iio_add(void)
{
    int inst_id = 0; /* instance unique ID: base address would be a good choice */
    pdev = platform_device_alloc("iio-dummy-random", inst_id);
    platform_device_add(pdev);
    pr_info("iio-dummy-random added");
    return 0;
}

static void __exit fake_iio_put(void)
{
    pr_info("iio-dummy-random removed");
	platform_device_put(pdev);
}

module_init(fake_iio_add);
module_exit(fake_iio_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
