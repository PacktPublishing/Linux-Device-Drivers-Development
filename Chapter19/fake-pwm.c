#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

struct fake_chip {
	struct pwm_chip chip;
	int foo;
	int bar;
	/* put the client structure here (SPI/I2C) */
};

static inline struct fake_chip *to_fake_chip(struct pwm_chip *chip)
{
	return container_of(chip, struct fake_chip, chip);
}


static int fake_pwm_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
	/*
	 * One may need to do some initialization when a PWM channel
	 * of the controller is requested. This should be done here.
	 *
	 * One may do something like 
	 *     prepare_pwm_device(struct pwm_chip *chip, pwm->hwpwm);
	 */

	return 0;
}

static int fake_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
			       int duty_ns, int period_ns)
{

    /*
     * In this function, one ne can do something like:
     *      struct fake_chip *priv = to_fake_chip(chip);
     *
	 *      return send_command_to_set_config(priv,
     *                      duty_ns, period_ns);
     */

	return 0;
}

static int fake_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    /*
     * In this function, one ne can do something like:
     *  struct fake_chip *priv = to_fake_chip(chip);
     *
     * return foo_chip_set_pwm_enable(priv, pwm->hwpwm, true);
     */
    
    pr_info("Somebody enabled PWM device number %d of this chip", pwm->hwpwm);
	return 0;
}

static void fake_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
    /*
     * In this function, one ne can do something like:
     *  struct fake_chip *priv = to_fake_chip(chip);
     *
     * return foo_chip_set_pwm_enable(priv, pwm->hwpwm, false);
     */
    
    pr_info("Somebody disabled PWM device number %d of this chip", pwm->hwpwm);
}

static const struct pwm_ops fake_pwm_ops = {
	.request = fake_pwm_request,
	.config = fake_pwm_config,
	.enable = fake_pwm_enable,
	.disable = fake_pwm_disable,
	.owner = THIS_MODULE,
};


static int fake_pwm_probe(struct platform_device *pdev)
{
	struct fake_chip *priv;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->chip.ops = &fake_pwm_ops;
	priv->chip.dev = &pdev->dev;
	priv->chip.base = -1;   /* Dynamic base */
	priv->chip.npwm = 3;    /* 3 channel controller */ 

	platform_set_drvdata(pdev, priv);
	return pwmchip_add(&priv->chip);
}

static int fake_pwm_remove(struct platform_device *pdev)
{
	struct fake_chip *priv = platform_get_drvdata(pdev);
	return pwmchip_remove(&priv->chip);
}

static const struct of_device_id fake_pwm_dt_ids[] = {
	{ .compatible = "packt,fake-pwm", },
	{ }
};
MODULE_DEVICE_TABLE(of, fake_pwm_dt_ids);

static struct platform_driver fake_pwm_driver = {
	.driver = {
		.name = "fake-pwm",
		.of_match_table = of_match_ptr(fake_pwm_dt_ids),
	},
	.probe = fake_pwm_probe,
	.remove = fake_pwm_remove,
};
module_platform_driver(fake_pwm_driver);

MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Fake pwm driver");
MODULE_LICENSE("GPL");
