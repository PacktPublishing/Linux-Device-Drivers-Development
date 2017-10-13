#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/interrupt.h>            /* For IRQ */
#include <linux/gpio.h>                 /* For Legacy integer based GPIO */
#include <linux/of_gpio.h>              /* For of_gpio* functions */
#include <linux/of.h>                   /* For DT*/


/*
 * Let us consider the node bellow
 *
 *    foo_device {
 *       compatible = "packt,gpio-legacy-sample";
 *       led-gpios = <&gpio2 15 GPIO_ACTIVE_HIGH>, // red 
 *                   <&gpio2 16 GPIO_ACTIVE_HIGH>, // green 
 *
 *       btn1-gpios = <&gpio2 1 GPIO_ACTIVE_LOW>;
 *       btn2-gpios = <&gpio2 1 GPIO_ACTIVE_LOW>;
 *   };
 */

static unsigned int gpio_red, gpio_green, gpio_btn1, gpio_btn2;
static int irq;

static irqreturn_t btn1_pushed_irq_handler(int irq, void *dev_id)
{
    int state;

    /* read the button value and change the led state */
    state = gpio_get_value(gpio_btn2);
    gpio_set_value(gpio_red, state);
    gpio_set_value(gpio_green, state);

    pr_info("gpio_btn1 interrupt: Interrupt! gpio_btn2 state is %d)\n", state);
    return IRQ_HANDLED;
}

static const struct of_device_id gpio_dt_ids[] = {
    { .compatible = "packt,gpio-legacy-sample", },
    { /* sentinel */ }
};

static int my_pdrv_probe (struct platform_device *pdev)
{
    int retval;
    struct device_node *np = pdev->dev.of_node;

    if (!np)
        return -ENOENT;

    gpio_red = of_get_named_gpio(np, "led", 0);
    gpio_green = of_get_named_gpio(np, "led", 1);
    gpio_btn1 = of_get_named_gpio(np, "btn1", 0);
    gpio_btn2 = of_get_named_gpio(np, "btn2", 0);

    gpio_request(gpio_green, "green-led");
    gpio_request(gpio_red, "red-led");
    gpio_request(gpio_btn1, "button-1");
    gpio_request(gpio_btn2, "button-2");

    /*
     * Configure Button GPIOs as input
     *
     * After this, one can call gpio_set_debounce()
     * only if the controller has the feature
     *
     * For example, to debounce  a button with a delay of 200ms
     *  gpio_set_debounce(gpio_btn1, 200);
     */
    gpio_direction_input(gpio_btn1);
    gpio_direction_input(gpio_btn2);

    /*
     * Set LED GPIOs as output, with their initial values set to 0
     */
    gpio_direction_output(gpio_red, 0);
    gpio_direction_output(gpio_green, 0);

    irq = gpio_to_irq(gpio_btn1);
    retval = request_threaded_irq(irq, NULL,
                            btn1_pushed_irq_handler,
                            IRQF_TRIGGER_LOW | IRQF_ONESHOT,
                            "gpio-legacy-sample", NULL);

    pr_info("Hello world!\n");
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    free_irq(irq, NULL);
    gpio_free(gpio_red);
    gpio_free(gpio_green);
    gpio_free(gpio_btn1);
    gpio_free(gpio_btn2);

    pr_info("End of the world\n");
    return 0;
}


static struct platform_driver mypdrv = {
    .probe      = my_pdrv_probe,
    .remove     = my_pdrv_remove,
    .driver     = {
        .name     = "gpio_legacy_sample",
        .of_match_table = of_match_ptr(gpio_dt_ids),  
        .owner    = THIS_MODULE,
    },
};
module_platform_driver(mypdrv);

MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
