#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init hellowolrd_init(void) {
    pr_info("Hello world!\n");
    return 0;
}

static void __exit hellowolrd_exit(void) {
    pr_info("End of the world\n");
}

module_init(hellowolrd_init);
module_exit(hellowolrd_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
