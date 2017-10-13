#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/uaccess.h>


/* 
 * The structure to represent 'eep_dev' devices.
 *  data - data buffer;
 *  buffer_size - size of the data buffer;
 *  block_size - maximum number of bytes that can be read or written 
 *    in one call;
 *  eep_mutex - a mutex to protect the fields of this structure;
 *  cdev - character device structure.
 */
struct eep_dev {
	unsigned char *data;
	struct i2c_client *client;
	struct mutex eep_mutex;
	int current_pointer;
	struct cdev cdev;
};

#define EEP_DEVICE_NAME     "packt-mem"
#define EEP_PAGE_SIZE           128
#define EEP_SIZE            1024*64 /* 24LC512 is 64KB sized */


static unsigned int eep_major = 0;
static unsigned int minor = 0;
static struct class *eep_class = NULL;

int  eep_open(struct inode *inode, struct file *filp)
{
	struct eep_dev *dev = NULL;
	dev = container_of(inode->i_cdev, struct eep_dev, cdev);
	
	if (dev == NULL){
	    pr_err("Container_of did not found any valid data\n");
		return -ENODEV; /* No such device */
	}
    dev->current_pointer = 0;
    /* store a pointer to struct eep_dev here for other methods */
    filp->private_data = dev;

    if (inode->i_cdev != &dev->cdev){
        pr_err("Device open: internal error\n");
        return -ENODEV; /* No such device */
    }

    dev->data = (unsigned char*)kzalloc(EEP_SIZE, GFP_KERNEL);
    if (dev->data == NULL){
        pr_err("Error allocating memory\n");
        return -ENOMEM;
    }
    return 0;
}

/*
 * Release is called when device node is closed
 */
int eep_release(struct inode *inode, struct file *filp)
{
    struct eep_dev *dev = filp->private_data;
    if (dev->data != NULL){
        kfree(dev->data);
        dev->data = NULL ;
    }
    dev->current_pointer = 0;
    return 0;
}

ssize_t  eep_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{
    int _reg_addr;
    u8 reg_addr[2];
    struct i2c_msg msg[2];
    struct eep_dev *dev = filp->private_data;
    ssize_t retval = 0;

    if (mutex_lock_killable(&dev->eep_mutex))
        return -EINTR;

    if (*f_pos >= EEP_SIZE) /* EOF */
        goto end_read;

    if(dev->current_pointer >= EEP_SIZE){
        retval = 0; /* EOF */
        goto end_read;
     }
	
    if (dev->current_pointer + count > EEP_SIZE)
        count = EEP_SIZE - dev->current_pointer;

    if (count > EEP_SIZE)
        count = EEP_SIZE;

    _reg_addr = dev->current_pointer;
    reg_addr[0] = (u8)(_reg_addr >> 8);
    reg_addr[1] = (u8)(_reg_addr & 0xFF);

    msg[0].addr = dev->client->addr;
    msg[0].flags = 0;                    /* Write */
    msg[0].len = 2;                      /* Address is 2byte coded */
    msg[0].buf = reg_addr;          
    msg[1].addr = dev->client->addr;
    msg[1].flags = I2C_M_RD;             /* We need to read */
    msg[1].len = count; 
    msg[1].buf = dev->data;

    if (i2c_transfer(dev->client->adapter, msg, 2) < 0)
        pr_err("ee24lc512: i2c_transfer failed\n"); 
 
    if(copy_to_user(buf, dev->data, count) != 0){
        retval = -EIO;
        goto end_read;
    }

    retval = count;
    dev->current_pointer += count ;

end_read:
    mutex_unlock(&dev->eep_mutex);
    return retval;
}

int transacWrite(struct eep_dev *dev,
        int _reg_addr, unsigned char *data,
        int offset, unsigned int len)
{    
    unsigned char tmp[len + 2];
    struct i2c_msg msg[1];
    
    tmp[0] =  (u8)(_reg_addr >> 8);
    tmp[1] =  (u8)(_reg_addr & 0xFF);
    memcpy (tmp + 2, &(data[offset]), len);
    msg[0].addr = dev->client->addr;
    msg[0].flags = 0;                    /* Write */
    msg[0].len = len + 2; /* Address is 2 bytes coded */
    msg[0].buf = tmp;

    if (i2c_transfer(dev->client->adapter, msg, 1) < 0){
        pr_err("ee24lc512: i2c_transfer failed\n");  
        return -1;
     }
     return len;  
}

ssize_t  eep_write(struct file *filp, const char __user *buf,
                    size_t count,  loff_t *f_pos)
{
    int _reg_addr, offset, remain_in_page, nb_page, last_remain, i;
    struct eep_dev *dev = filp->private_data;
    ssize_t retval = 0;

    if (mutex_lock_killable(&dev->eep_mutex))
        return -EINTR;

    if(dev->current_pointer >= EEP_SIZE){
        retval = -EINVAL;
        goto end_write;
    }

    if (*f_pos >= EEP_SIZE) {
        /* Writing beyond the end of the buffer is not allowed. */
        retval = -EINVAL;
        goto end_write;
    }

    if (dev->current_pointer + count >= EEP_SIZE)
        count = EEP_SIZE - dev->current_pointer;

    if (count > EEP_SIZE)
        count = EEP_SIZE;

    if (copy_from_user(dev->data, buf, count) != 0){
        retval = -EFAULT;
        goto end_write;
    }

    _reg_addr =  dev->current_pointer;
    offset = 0;
    remain_in_page = (EEP_PAGE_SIZE - (dev->current_pointer % EEP_PAGE_SIZE)) % EEP_PAGE_SIZE;
    nb_page = (count - remain_in_page) / EEP_PAGE_SIZE;
    last_remain = (count - remain_in_page) % EEP_PAGE_SIZE ;

    if (remain_in_page > 0){
        retval = transacWrite(dev, _reg_addr, dev->data, offset, remain_in_page);
        if (retval < 0)
            goto end_write;
        offset += remain_in_page;
        dev->current_pointer += remain_in_page;
        _reg_addr += remain_in_page;
        retval = offset;
        mdelay(10);
    }

    if (nb_page < 1 && last_remain < 1)
        goto end_write;

    for (i=0; i < nb_page; i++){
        retval = transacWrite(dev, _reg_addr, dev->data, offset, EEP_PAGE_SIZE);
        if (retval < 0)
            goto end_write;
        offset += EEP_PAGE_SIZE;
        _reg_addr += EEP_PAGE_SIZE;
        dev->current_pointer += EEP_PAGE_SIZE;
        retval = offset;
        mdelay(10);
    }

    if (last_remain > 0){
        retval = transacWrite(dev, _reg_addr, dev->data, offset, last_remain);
        if (retval < 0)
            goto end_write;
        offset += last_remain;
        _reg_addr += last_remain;
        dev->current_pointer += last_remain;
        retval = offset;
        mdelay(10);
    }

end_write:
    mutex_unlock(&dev->eep_mutex);
    return retval;
}

loff_t eep_llseek(struct file *filp, loff_t off, int whence)
{
	struct eep_dev *dev = (struct eep_dev *)filp->private_data;
	loff_t newpos = 0;
	
	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END - Not supported */
		return -EINVAL;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0 || EEP_SIZE)
		return -EINVAL;
	
	dev->current_pointer = newpos;	
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations eep_fops = {
	.owner =    THIS_MODULE,
	.read =     eep_read,
	.write =    eep_write,
	.open =     eep_open,
	.release =  eep_release,
	.llseek =   eep_llseek,
};

static const struct of_device_id ee24lc512_ids[] = {
	{ .compatible = "microchip,ee24lc512", },
	{ /* sentinel */ }
};

static int ee24lc512_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
    unsigned char data[5];
    u8 reg_addr[2];
    struct i2c_msg msg[2];
    int err = 0;
    dev_t devno = 0;
    struct eep_dev *eep_device = NULL;
    struct device *device = NULL;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

   /* 
    * We send a simple i2c transaction. If it fails,
    * it meants there is no eeprom
    */
    
    reg_addr[0] =  0x00;
    reg_addr[1] =  0x00;

    msg[0].addr = client->addr;
    msg[0].flags = 0;                    /* Write */
    msg[0].len = 2;                      /* Address is 2byte coded */
    msg[0].buf = reg_addr;          

    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;             /* We need to read */
    msg[1].len = 5; //count; 
    msg[1].buf = data;

    if (i2c_transfer(client->adapter, msg, 2) < 0)
        pr_err("ee24lc512: i2c_transfer failed\n");

    /* Get a range of minor numbers (starting with 0) to work with */
    err = alloc_chrdev_region(&devno, 0, 1, EEP_DEVICE_NAME);
    if (err < 0) {
        pr_err("alloc_chrdev_region() failed for %s\n", EEP_DEVICE_NAME);
        return err;
    }
    eep_major = MAJOR(devno);

    /* Create device class */
    eep_class = class_create(THIS_MODULE, EEP_DEVICE_NAME);
    if (IS_ERR(eep_class)) {
        err = PTR_ERR(eep_class);
        goto fail;
    }

    eep_device = (struct eep_dev *)kzalloc(sizeof(struct eep_dev), GFP_KERNEL);
    if (eep_device == NULL) {
        err = -ENOMEM;
        goto fail;
    }

    /* Memory is to be allocated when the device is opened the first time */
    eep_device->data = NULL;
    eep_device->client = client;
    eep_device->current_pointer = 0;
    mutex_init(&eep_device->eep_mutex);

    cdev_init(&eep_device->cdev, &eep_fops);
    eep_device->cdev.owner = THIS_MODULE;
    err = cdev_add(&eep_device->cdev, devno, 1);

    if (err){
        pr_err("Error while trying to add %s", EEP_DEVICE_NAME);
        goto fail;
    }

    device = device_create(eep_class, NULL, /* no parent device */
                            devno, NULL, /* no additional data */
                            EEP_DEVICE_NAME);

    if (IS_ERR(device)) {
        err = PTR_ERR(device);
        pr_err("failure while trying to create %s device", EEP_DEVICE_NAME);
        cdev_del(&eep_device->cdev);
        goto fail;
    }

    i2c_set_clientdata(client, eep_device);
    return 0; /* success */

fail:
    if(eep_class != NULL){
        device_destroy(eep_class, MKDEV(eep_major, minor));
        class_destroy(eep_class);
    }
    if (eep_device != NULL)
        kfree(eep_device);
    return err;
}

static int ee24lc512_remove(struct i2c_client *client)
{
    struct eep_dev *eep_device = i2c_get_clientdata(client);
    device_destroy(eep_class, MKDEV(eep_major, 0));

    kfree(eep_device->data);
    mutex_destroy(&eep_device->eep_mutex);
    kfree(eep_device);
    class_destroy(eep_class);
    unregister_chrdev_region(MKDEV(eep_major, 0), 1);
	return 0;
}

static const struct i2c_device_id ee24lc512_id[] = {
	{"ee24lc512", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ee24lc512_id);

static struct i2c_driver ee24lc512_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ee24lc512",
		.of_match_table = of_match_ptr(ee24lc512_ids),
	},
	.probe = ee24lc512_probe,
	.remove = ee24lc512_remove,
	.id_table = ee24lc512_id,
};

module_i2c_driver(ee24lc512_i2c_driver);

MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
