#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

#define GPIO_NUM 16
#define INPUT 1
#define OUTPUT 0

/* Chip Control registers */

/*
 * GP0 ang GP1 provides access to GPIO ports
 * A read from these register provide status of pin of these ports
 * A write to these register will modify the output latch register (OLAT0, OLAT1)
 * and data register.
 */
#define GP0		0x0
#define GP1		0x1

/*
 * IOLAT0 and IOLAT1 control the output value of GP0 and GP1
 * Write into one of these register will result in affecting 
 */ 
#define OLAT0	0x2
#define OLAT1	0x3
#define IOPOL0	0x4
#define IOPOL1	0x5

/*
 * IODIR0 and IODIR1 registers control GP0 and GP1 IOs direction
 * 1 => input; 0 => output
 * default value are 0xFF in each reg.
 */
#define IODIR0	0x6
#define IODIR1	0x7

/*
 * INTCAP0 and INTCAP1 register contain value of the port that generated the interupt
 * INTCAP0 contains value of GPO at time of GPO change interrupt
 * INTCAP1 contains value of GP1 at time of GP1 change interrupt
 */
#define INTCAP0	0x8
#define INTCAP1	0x9

struct mcp23016 {
	struct i2c_client *client;
	struct gpio_chip chip;
};

static inline struct mcp23016 *to_mcp23016(struct gpio_chip *gc)
{
	return container_of(gc, struct mcp23016, chip);
}

static int mcp23016_get_value(struct gpio_chip *gc, unsigned offset)
{
	s32 value;
	struct mcp23016 *mcp = to_mcp23016(gc);
	unsigned bank = offset / 8 ;
	unsigned bit = offset % 8 ;
	
	u8 reg_intcap = (bank == 0) ? INTCAP0 : INTCAP1;
	value = i2c_smbus_read_byte_data(mcp->client, reg_intcap);
	return (value >= 0) ? (value >> bit) & 0x1 : 0;
}

static int mcp23016_set(struct mcp23016 *mcp, unsigned offset, int val)
{
	s32 value;

	unsigned bank = offset / 8 ;
	u8 reg_gpio = (bank == 0) ? GP0 : GP1;
	unsigned bit = offset % 8 ;

	value = i2c_smbus_read_byte_data(mcp->client, reg_gpio);
	if (value >= 0) {
		if (val)
			value |= 1 << bit;
		else
			value &= ~(1 << bit);

		return i2c_smbus_write_byte_data(mcp->client, reg_gpio, value);
	}

	return value;
}


static void mcp23016_set_value(struct gpio_chip *gc, unsigned offset, int val)
{
	struct mcp23016 *mcp = to_mcp23016(gc);
	mcp23016_set(mcp, offset, val);
}

/*
 * direction = 1 => input
 * direction = 0 => output
 */
static int mcp23016_direction(struct gpio_chip *gc, unsigned offset,
                                unsigned direction, int val)
{
	struct mcp23016 *mcp = to_mcp23016(gc);
	unsigned bank = offset / 8 ;
	unsigned bit = offset % 8 ;
	u8 reg_iodir = (bank == 0) ? IODIR0 : IODIR1;
	s32 iodirval = i2c_smbus_read_byte_data(mcp->client, reg_iodir);

	if (direction)
		iodirval |= 1 << bit;
	else
		iodirval &= ~(1 << bit);

	i2c_smbus_write_byte_data(mcp->client, reg_iodir, iodirval);
	if (direction)
		return iodirval ;
	else
		return mcp23016_set(mcp, offset, val);    
}

static int mcp23016_direction_output(struct gpio_chip *gc,
                                    unsigned offset, int val)
{
	return mcp23016_direction(gc, offset, OUTPUT, val);
}

static int mcp23016_direction_input(struct gpio_chip *gc,
                                    unsigned offset)
{
	return mcp23016_direction(gc, offset, INPUT, 0);
}

static const struct of_device_id mcp23016_ids[] = {
	{ .compatible = "microchip,mcp23016", },
	{ /* sentinel */ }
};

static int mcp23016_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	struct mcp23016 *mcp;

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	mcp = devm_kzalloc(&client->dev, sizeof(*mcp), GFP_KERNEL);
	if (!mcp)
		return -ENOMEM;

	mcp->chip.label = client->name;
	mcp->chip.base = -1;
	mcp->chip.dev = &client->dev;
	mcp->chip.owner = THIS_MODULE;
	mcp->chip.ngpio = GPIO_NUM;
	mcp->chip.can_sleep = 1;
	mcp->chip.get = mcp23016_get_value;
	mcp->chip.set = mcp23016_set_value;
	mcp->chip.direction_output = mcp23016_direction_output;
	mcp->chip.direction_input = mcp23016_direction_input;
	mcp->client = client;
	i2c_set_clientdata(client, mcp);

	return gpiochip_add(&mcp->chip);
}

static int mcp23016_remove(struct i2c_client *client)
{
	struct mcp23016 *mcp;
	mcp = i2c_get_clientdata(client);
	gpiochip_remove(&mcp->chip);
	return 0;
}

static const struct i2c_device_id mcp23016_id[] = {
	{"mcp23016", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, mcp23016_id);

static struct i2c_driver mcp23016_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "mcp23016",
		.of_match_table = of_match_ptr(mcp23016_ids),
	},
	.probe = mcp23016_probe,
	.remove = mcp23016_remove,
	.id_table = mcp23016_id,
};

module_i2c_driver(mcp23016_i2c_driver);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
