#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
 
#define MS_TO_NS(x) (x * 1E6L)

static struct hrtimer hr_timer;

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{
    pr_info( "my_hrtimer_callback called (%ld).\n", jiffies );
    return HRTIMER_NORESTART;
}
 
static int hrt_init_module( void )
{
    ktime_t ktime;
    unsigned long delay_in_ms = 200L;

    pr_info("HR Timer module installing\n");

    /*
     * ktime = ktime_set(0, 200 * 1000 * 1000);
     * 200 ms = 10 * 1000 * 1000 ns
     */
    ktime = ktime_set( 0, MS_TO_NS(delay_in_ms) );

    hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    hr_timer.function = &my_hrtimer_callback;
    pr_info( "Starting timer to fire in %ldms (%ld)\n", \
           delay_in_ms, jiffies );

    hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
    return 0;
}
 
static void hrt_cleanup_module( void )
{
    int ret;
    ret = hrtimer_cancel( &hr_timer );
    if (ret)
        pr_info("The timer was still in use...\n");

    pr_info("HR Timer module uninstalling\n");
    return;
}

module_init(hrt_init_module);
module_exit(hrt_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Standard timer example");
