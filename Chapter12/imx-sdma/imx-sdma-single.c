/*
 * Copyright 2006-2014 Freescale Semiconductor, Inc. All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/platform_data/dma-imx.h>
#include <linux/dmaengine.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/delay.h>

static int gMajor; /* major number of device */
static struct class *dma_tm_class;
u32 *wbuf;
u32 *rbuf;

struct dma_chan *dma_m2m_chan;
struct completion dma_m2m_ok;

/*
 * For single mapping, the buffer size does not need to be a multiple of page
 * size.
 *
 */
#define SDMA_BUF_SIZE  1024

static bool dma_m2m_filter(struct dma_chan *chan, void *param)
{
    if (!imx_dma_is_general_purpose(chan))
        return false;
    chan->private = param;
    return true;
}

int sdma_open(struct inode * inode, struct file * filp)
{
    dma_cap_mask_t dma_m2m_mask;
    struct imx_dma_data m2m_dma_data = {0};

    init_completion(&dma_m2m_ok);

    /* Initialize capabilities */
    dma_cap_zero(dma_m2m_mask);
    dma_cap_set(DMA_MEMCPY, dma_m2m_mask);
    m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
    m2m_dma_data.priority = DMA_PRIO_HIGH;

    dma_m2m_chan = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
    if (!dma_m2m_chan) {
        pr_err("Error opening the SDMA memory to memory channel\n");
        return -EINVAL;
    } else {
		pr_info("opened channel %d, req lin %d\n", dma_m2m_chan->chan_id, m2m_dma_data.dma_request);
	}

    wbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
    if(!wbuf) {
        pr_err("error wbuf !!!!!!!!!!!\n");
        return -1;
    }

    rbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
    if(!rbuf) {
        pr_err("error rbuf !!!!!!!!!!!\n");
        return -1;
    }

    return 0;
}

int sdma_release(struct inode * inode, struct file * filp)
{
    dma_release_channel(dma_m2m_chan);
    dma_m2m_chan = NULL;
    kfree(wbuf);
    kfree(rbuf);
    return 0;
}

ssize_t sdma_read (struct file *filp, char __user * buf, size_t count,
                                loff_t * offset)
{
    int i;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,35))
    for (i=0; i<SDMA_BUF_SIZE/4; i++) {
        if (*(rbuf+i) != *(wbuf+i)) {
            pr_err("Single DMA buffer copy falled!,r=%x,w=%x,%d\n", *(rbuf+i), *(wbuf+i), i);
            return 0;
        }
    }
    pr_info("buffer copy passed!\n");
#endif
    return 0;
}

static void dma_m2m_callback(void *data)
{
    pr_info("in %s\n",__func__);
    complete(&dma_m2m_ok);
    return ;
}

ssize_t sdma_write(struct file * filp, const char __user * buf, size_t count,
                                loff_t * offset)
{
    u32 *index, i;
    struct dma_slave_config dma_m2m_config = {0};
    struct dma_async_tx_descriptor *dma_m2m_desc;
    dma_addr_t dma_src, dma_dst;
    dma_cookie_t cookie;

    index = wbuf;
    for (i=0; i<SDMA_BUF_SIZE/4; i++) {
        *(index + i) = 0x56565656;
    }

    /* 2- Set slave and controller specific parameters */
    dma_m2m_config.direction = DMA_MEM_TO_MEM;
    dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    dmaengine_slave_config(dma_m2m_chan, &dma_m2m_config);

    /* dma_src = dma_map_single(NULL, wbuf, SDMA_BUF_SIZE, dma_m2m_config.direction); */
    /* dma_dst = dma_map_single(NULL, rbuf, SDMA_BUF_SIZE, dma_m2m_config.direction); */
    dma_src = dma_map_single(NULL, wbuf, SDMA_BUF_SIZE, DMA_TO_DEVICE);
    dma_dst = dma_map_single(NULL, rbuf, SDMA_BUF_SIZE, DMA_FROM_DEVICE);

    /* 3- Get a descriptor for the transaction. */
    dma_m2m_desc = dma_m2m_chan->device->device_prep_dma_memcpy(dma_m2m_chan, dma_dst, dma_src, SDMA_BUF_SIZE,0);
    dma_m2m_desc->callback = dma_m2m_callback;
    if (dma_m2m_desc)
		pr_info("Got a DMA descriptor\n");
	else
		pr_err("error in prep_dma_sg\n");


    /* 4- Submit the transaction */
    cookie = dmaengine_submit(dma_m2m_desc);
    pr_info("Got this cookie: %d\n", cookie);
 
    /* 5- Issue pending DMA requests and wait for callback notification */
    dma_async_issue_pending(dma_m2m_chan);
    pr_info("waiting for DMA transaction...\n");

    /* One can use wait_for_completion_timeout() also */
    wait_for_completion(&dma_m2m_ok);

    /* dma_unmap_single(NULL, dma_src, SDMA_BUF_SIZE, dma_m2m_config.direction); */
    /* dma_unmap_single(NULL, dma_dst, SDMA_BUF_SIZE, dma_m2m_config.direction); */
    dma_unmap_single(NULL, dma_src, SDMA_BUF_SIZE, DMA_TO_DEVICE);
    dma_unmap_single(NULL, dma_dst, SDMA_BUF_SIZE, DMA_FROM_DEVICE);

    return count;
}

struct file_operations dma_fops = {
    open:       sdma_open,
    release:    sdma_release,
    read:       sdma_read,
    write:      sdma_write,
};

int __init sdma_init_module(void)
{
    struct device *temp_class;
    int error;

    /* register a character device */
    error = register_chrdev(0, "sdma_test", &dma_fops);
    if (error < 0) {
        pr_err("SDMA test driver can't get major number\n");
        return error;
    }
    gMajor = error;
    pr_info("SDMA test major number = %d\n",gMajor);

    dma_tm_class = class_create(THIS_MODULE, "sdma_test");
    if (IS_ERR(dma_tm_class)) {
        pr_err(KERN_ERR "Error creating sdma test module class.\n");
        unregister_chrdev(gMajor, "sdma_test");
        return PTR_ERR(dma_tm_class);
    }

    temp_class = device_create(dma_tm_class, NULL,
                   MKDEV(gMajor, 0), NULL, "sdma_test");

    if (IS_ERR(temp_class)) {
        pr_err(KERN_ERR "Error creating sdma test class device.\n");
        class_destroy(dma_tm_class);
        unregister_chrdev(gMajor, "sdma_test");
        return -1;
    }

    pr_info("SDMA test Driver Module loaded\n");
    return 0;
}

static void sdma_cleanup_module(void)
{
    unregister_chrdev(gMajor, "sdma_test");
    device_destroy(dma_tm_class, MKDEV(gMajor, 0));
    class_destroy(dma_tm_class);

    pr_info("SDMA test Driver Module Unloaded\n");
}

module_init(sdma_init_module);
module_exit(sdma_cleanup_module);

MODULE_AUTHOR("Freescale Semiconductor");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("SDMA test driver");
MODULE_LICENSE("GPL");
