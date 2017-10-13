#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 /* For Legacy integer based GPIO */
#include <linux/interrupt.h>            /* For IRQ */

/*
 * Please choose values that are free on your system
 */
static unsigned int GPIO_LED_RED = 49;
static unsigned int GPIO_BTN1 = 115;
static unsigned int GPIO_BTN2 = 116;
static unsigned int GPIO_LED_GREEN = 120;
static int irq;

static irqreturn_t btn1_pushed_irq_handler(int irq, void *dev_id)
{
    int state;

    /* read the button value and change the led state */
    state = gpio_get_value(GPIO_BTN2);
    gpio_set_value(GPIO_LED_RED, state);
    gpio_set_value(GPIO_LED_GREEN, state);

    pr_info("GPIO_BTN1 interrupt: Interrupt! GPIO_BTN2 state is %d)\n", state);
    return IRQ_HANDLED;
}

static int __init hellowolrd_init(void)
{
    int retval;

    /*
     * One could have checked whether the GPIO is valid on the controller or not,
     * using gpio_is_valid() function.
     * Ex:
     *  if (!gpio_is_valid(GPIO_LED_RED)) {
     *       pr_infor("Invalid Red LED\n");
     *       return -ENODEV;
     *   }
     */
    gpio_request(GPIO_LED_GREEN, "green-led");
    gpio_request(GPIO_LED_RED, "red-led");
    gpio_request(GPIO_BTN1, "button-1");
    gpio_request(GPIO_BTN2, "button-2");

    /*
     * Configure Button GPIOs as input
     *
     * After this, one can call gpio_set_debounce()
     * only if the controller has the feature
     *
     * For example, to debounce  a button with a delay of 200ms
     *  gpio_set_debounce(GPIO_BTN1, 200);
     */
    gpio_direction_input(GPIO_BTN1);
    gpio_direction_input(GPIO_BTN2);

    /*
     * Set LED GPIOs as output, with their initial values set to 0
     */
    gpio_direction_output(GPIO_LED_RED, 0);
    gpio_direction_output(GPIO_LED_GREEN, 0);

    irq = gpio_to_irq(GPIO_BTN1);
    retval = request_threaded_irq(irq, NULL,\
                            btn1_pushed_irq_handler, \
                            IRQF_TRIGGER_LOW | IRQF_ONESHOT, \
                            "device-name", NULL);

    pr_info("Hello world!\n");
    return 0;
}

static void __exit hellowolrd_exit(void)
{
    free_irq(irq, NULL);
    gpio_free(GPIO_LED_RED);
    gpio_free(GPIO_LED_GREEN);
    gpio_free(GPIO_BTN1);
    gpio_free(GPIO_BTN2);

    pr_info("End of the world\n");
}


module_init(hellowolrd_init);
module_exit(hellowolrd_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
