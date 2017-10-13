#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static char *mystr = "hello";
static int myint = 1;
static int myarr[3] = {0, 1, 2};


module_param(myint, int, S_IRUGO);
module_param(mystr, charp, S_IRUGO);
module_param_array(myarr, int,NULL, S_IWUSR|S_IRUSR);

MODULE_PARM_DESC(myint,"this is my int variable");
MODULE_PARM_DESC(mystr,"this is my char pointer variable");
MODULE_PARM_DESC(myarr,"this is my array of int");
MODULE_INFO(my_field_name, "What eeasy value");

static int __init hellowolrd_init(void) {
    pr_info("Hello world with parameters!\n");
    pr_info("The *mystr* parameter: %s\n", mystr);
    pr_info("The *myint* parameter: %d\n", myint);
    pr_info("The *myarr* parameter: %d, %d, %d\n", myarr[0], myarr[1], myarr[2]);
    return 0;
}

static void __exit hellowolrd_exit(void) {
    pr_info("End of the world\n");
}

module_init(hellowolrd_init);
module_exit(hellowolrd_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
