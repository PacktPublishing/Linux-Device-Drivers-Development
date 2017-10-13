#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/of.h>                   /* For DT*/

#define GPIO_NUM 16
static struct gpio_chip chip;

static int fake_get_value(struct gpio_chip *gc, unsigned offset)
{
	return 0;
}

static void fake_set_value(struct gpio_chip *gc, unsigned offset, int val)
{
}

static int fake_direction_output(struct gpio_chip *gc,
				       unsigned offset, int val)
{
	return 0;
}

static int fake_direction_input(struct gpio_chip *gc,
				       unsigned offset)
{
    return 0;
}

static const struct of_device_id fake_gpiochip_ids[] = {
    { .compatible = "packt,fake-gpio-chip", },
    { /* sentinel */ }
};

static int my_pdrv_probe (struct platform_device *pdev)
{
	chip.label = pdev->name;
	chip.base = -1;
	chip.dev = &pdev->dev;
	chip.owner = THIS_MODULE;
	chip.ngpio = GPIO_NUM;
	chip.can_sleep = 1;
	chip.get = fake_get_value;
	chip.set = fake_set_value;
	chip.direction_output = fake_direction_output;
	chip.direction_input = fake_direction_input;

	return gpiochip_add(&chip);
}

static int my_pdrv_remove(struct platform_device *pdev)
{
	gpiochip_remove(&chip);
    return 0;
}

static struct platform_driver mypdrv = {
    .probe      = my_pdrv_probe,
    .remove     = my_pdrv_remove,
    .driver     = {
        .name     = "fake-gpio-chip",
        .of_match_table = of_match_ptr(fake_gpiochip_ids),
        .owner    = THIS_MODULE,
    },
};
module_platform_driver(mypdrv);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
