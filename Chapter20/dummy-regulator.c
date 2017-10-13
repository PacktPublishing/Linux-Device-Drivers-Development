#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/interrupt.h>            /* For IRQ */
#include <linux/of.h>                   /* For DT*/
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define DUMMY_VOLTAGE_MIN    850000
#define DUMMY_VOLTAGE_MAX    1600000
#define DUMMY_VOLTAGE_STEP   50000

struct my_private_data {
    int foo;
    int bar;
    struct mutex lock;
};

static const struct of_device_id regulator_dummy_ids[] = {
    { .compatible = "packt,regulator-dummy", },
    { /* sentinel */ }
};

static struct regulator_init_data dummy_initdata[] = {
    [0] = {
        .constraints = {
            .always_on = 0,
            .min_uV = DUMMY_VOLTAGE_MIN,
            .max_uV = DUMMY_VOLTAGE_MAX,
        },
    },
    [1] = {
        .constraints = {
            .always_on = 1,
        },
    },
};

static int isl6271a_get_voltage_sel(struct regulator_dev *dev)
{
    return 0;
}

static int isl6271a_set_voltage_sel(struct regulator_dev *dev,
                    unsigned selector)
{
    return 0;
}


static struct regulator_ops dummy_fixed_ops = {
    .list_voltage   = regulator_list_voltage_linear,
};


static struct regulator_ops dummy_core_ops = {
    .get_voltage_sel = isl6271a_get_voltage_sel,
    .set_voltage_sel = isl6271a_set_voltage_sel,
    .list_voltage   = regulator_list_voltage_linear,
    .map_voltage    = regulator_map_voltage_linear,
};


static const struct regulator_desc dummy_desc[] = {
    {
        .name       = "Dummy Core",
        .id     = 0,
        .n_voltages = 16,
        .ops        = &dummy_core_ops,
        .type       = REGULATOR_VOLTAGE,
        .owner      = THIS_MODULE,
        .min_uV     = DUMMY_VOLTAGE_MIN,
        .uV_step    = DUMMY_VOLTAGE_STEP,
    }, {
        .name       = "Dummy Fixed",
        .id     = 1,
        .n_voltages = 1,
        .ops        = &dummy_fixed_ops,
        .type       = REGULATOR_VOLTAGE,
        .owner      = THIS_MODULE,
        .min_uV     = 1300000,
    },
};

static int my_pdrv_probe (struct platform_device *pdev)
{

    struct regulator_config config = { };
    struct regulator_dev *dummy_regulator_rdev[2];
    int ret, i;
    config.dev = &pdev->dev;

    for (i = 0; i < 2; i++){
        config.init_data = &dummy_initdata[i];
        dummy_regulator_rdev[i] = regulator_register(&dummy_desc[i], &config);
        if (IS_ERR(dummy_regulator_rdev)) {
            ret = PTR_ERR(dummy_regulator_rdev);
            pr_err("Failed to register regulator: %d\n", ret);
            return ret;
        }
    }

    platform_set_drvdata(pdev, dummy_regulator_rdev);
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    int i;
    struct regulator_dev *dummy_regulator_rdev = platform_get_drvdata(pdev);
    for (i = 0; i < 2; i++)
        regulator_unregister(&dummy_regulator_rdev[i]);
    return 0;
}

static struct platform_driver mypdrv = {
    .probe      = my_pdrv_probe,
    .remove     = my_pdrv_remove,
    .driver     = {
        .name     = "regulator-dummy",
        .of_match_table = of_match_ptr(regulator_dummy_ids),  
        .owner    = THIS_MODULE,
    },
};
module_platform_driver(mypdrv);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
