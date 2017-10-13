#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>      /* For platform devices */
#include <linux/interrupt.h>            /* For IRQ */
#include <linux/of.h>                   /* For DT*/
#include <linux/iio/iio.h>    /* mandatory */
#include <linux/iio/sysfs.h>  /* mandatory since sysfs are used */
#include <linux/iio/events.h> /* For advanced users, to manage iio events */
#include <linux/iio/buffer.h>  /* mandatory to use triggered buffers */


#define FAKE_VOLTAGE_CHANNEL(num)   \
	{                               \
	    .type = IIO_VOLTAGE,        \
	    .indexed = 1,               \
	    .channel = (num),           \
	    .address = (num),           \
	    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),           \
	    .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE)    \
	}

struct my_private_data {
	int foo;
	int bar;
	struct mutex lock;
};

static int fake_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *channel, int *val,
			    int *val2, long mask)
{
	return 0;
}

static int fake_write_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int val, int val2, long mask)
{
	return 0;
}

static const struct iio_chan_spec fake_channels[] = {
	FAKE_VOLTAGE_CHANNEL(0),
	FAKE_VOLTAGE_CHANNEL(1),
	FAKE_VOLTAGE_CHANNEL(2),
	FAKE_VOLTAGE_CHANNEL(3),
};

static const struct of_device_id iio_dummy_ids[] = {
    { .compatible = "packt,iio-dummy-random", },
    { /* sentinel */ }
};

static const struct iio_info fake_iio_info = {
	.read_raw = fake_read_raw,
	.write_raw		= fake_write_raw,
	.driver_module = THIS_MODULE,
};

static int my_pdrv_probe (struct platform_device *pdev)
{
	struct iio_dev *indio_dev;
	struct my_private_data *data;

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*data));
	if (!indio_dev) {
		dev_err(&pdev->dev, "iio allocation failed!\n");
		return -ENOMEM;
	}

	data = iio_priv(indio_dev);
	mutex_init(&data->lock);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->info = &fake_iio_info;
	indio_dev->name = KBUILD_MODNAME;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = fake_channels;
	indio_dev->num_channels = ARRAY_SIZE(fake_channels);
	indio_dev->available_scan_masks = 0xF;
	iio_device_register(indio_dev);

	platform_set_drvdata(pdev, indio_dev);
	return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	iio_device_unregister(indio_dev);
	return 0;
}

static struct platform_driver mypdrv = {
	.probe      = my_pdrv_probe,
	.remove     = my_pdrv_remove,
	.driver     = {
		.name     = "iio-dummy-random",
		.of_match_table = of_match_ptr(iio_dummy_ids),  
		.owner    = THIS_MODULE,
	},
};
module_platform_driver(mypdrv);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
