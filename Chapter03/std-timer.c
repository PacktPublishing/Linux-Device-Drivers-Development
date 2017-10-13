#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

static struct timer_list my_timer;

void my_timer_callback( unsigned long data )
{
    pr_info( "%s called (%ld).\n", __FUNCTION__, jiffies );
}
 
static int __init my_init( void )
{
    int retval;
    pr_info("Timer module loaded\n");

    setup_timer( &my_timer, my_timer_callback, 0 );
    pr_info( "Setup timer to fire in 300ms (%ld)\n", jiffies );

    retval = mod_timer( &my_timer, jiffies + msecs_to_jiffies(300) );
    if (retval)
        pr_info("Timer firing failed\n");
 
    return 0;
}
 
static void my_exit( void )
{
    int retval;
    retval = del_timer( &my_timer );
    if (retval)
        pr_info("The timer is still in use...\n");

    pr_info("Timer module unloaded\n");
    return;
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Standard timer example");
MODULE_LICENSE("GPL");
