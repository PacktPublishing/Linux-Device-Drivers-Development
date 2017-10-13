#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/rtc.h>
#include <linux/of.h>


static struct timespec begin_time;
static unsigned long time = 0;

static inline unsigned long timespec_to_ulong(struct timespec *ts)
{
	return ts->tv_nsec < NSEC_PER_SEC/2 ? ts->tv_sec : ts->tv_sec + 1;
}

static inline void get_uptime(struct timespec* ts)
{
	getrawmonotonic(ts);
}

static int fake_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct timespec now, diff;

	get_uptime(&now);
	diff = timespec_sub(now, begin_time);

	rtc_time_to_tm(time + timespec_to_ulong(&diff), tm);
	return rtc_valid_tm(tm);
}

static int fake_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	get_uptime(&begin_time);
	rtc_tm_to_time(tm, &time);
	return 0;
}

static const struct rtc_class_ops fake_rtc_ops = {
	.read_time = fake_rtc_read_time,
	.set_time = fake_rtc_set_time
};

static const struct of_device_id rtc_dt_ids[] = {
    { .compatible = "packt,rtc-fake", },
    { /* sentinel */ }
};

static int fake_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;

	get_uptime(&begin_time);

	rtc = rtc_device_register(pdev->name, &pdev->dev, &fake_rtc_ops,
			THIS_MODULE);

	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	platform_set_drvdata(pdev, rtc);
	dev_info(&pdev->dev, "loaded; begin_time is %lu, rtc_time is %lu\n",
			timespec_to_ulong(&begin_time), time);

	return 0;
}

static int fake_rtc_remove(struct platform_device *pdev)
{
	rtc_device_unregister(platform_get_drvdata(pdev));
	return 0;
}


static struct platform_driver fake_rtc_drv = {
	.probe = fake_rtc_probe,
	.remove = fake_rtc_remove,
	.driver = {
		.name = "fake-rtc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rtc_dt_ids),
	},
};
module_platform_driver(fake_rtc_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com");
MODULE_DESCRIPTION("Fake RTC module");
