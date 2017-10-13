# i.MX6 SDMA driver test

There are two drivers example in this chapter. After running `make` command,
there will be two modules:

* imx-sdma-scatter-gather.ko
* imx-sdma-single.ko

The fist module implements the scatter/gather DMA, and the second one implement
a single buffer mapping. Both rely on i.MX6 from NXP.
These two modules are mutally excluse. These can't be load at the same time.
Prior to load one module, make sure the other one is not loaded.

Whatever module is loaded, it will create a character device, `/dev/sdma_test`.

```bash
# udevadm info /dev/sdma_test 
P: /devices/virtual/sdma_test/sdma_test
N: sdma_test
E: DEVNAME=/dev/sdma_test
E: DEVPATH=/devices/virtual/sdma_test/sdma_test
E: MAJOR=244
E: MINOR=0
E: SUBSYSTEM=sdma_test
```

For testing purpose, one just has to write a dummy string into it, and then
read anything from it.

## Single mapping DMA

Below is what one can do for testing the single buffer mapping module:

```bash
# insmod imx-sdma-single.ko 
SDMA test major number = 244
SDMA test Driver Module loaded

# echo "" > /dev/sdma_test  
opened channel 6, req lin 0
Got a DMA descriptor
Got this cookie: 2
waiting for DMA transaction...
in dma_m2m_callback

# cat /dev/sdma_test 
opened channel 6, req lin 0
buffer copy passed!

# rmmod imx-sdma-single.ko
SDMA test Driver Module Unloaded
```

This will produce the below debug message

```bash
# dmesg
[...]
[  847.101301] SDMA test major number = 244
[  847.107798] SDMA test Driver Module loaded
[  860.294889] opened channel 6, req lin 0
[  860.297779] Got a DMA descriptor
[  860.299721] Got this cookie: 2
[  860.301481] waiting for DMA transaction...
[  860.301583] in dma_m2m_callback
[  878.820893] opened channel 6, req lin 0
[  878.823654] buffer copy passed!
[  889.551947] SDMA test Driver Module Unloaded
```

## Scatter list DMA

Prior to load `imx-sdma-scatter-gather.ko`, one should unlod
`imx-sdma-single.ko`, and vice versa.

```bash
# insmod imx-sdma-scatter-gather.ko 
SDMA test major number = 244
SDMA test Driver Module loaded

# echo "" > /dev/sdma_test 
opened channel 6, req lin 0
Got a DMA descriptor
Got this cookie: 5
waiting for DMA transaction...
in dma_m2m_callback
end - start = 6490

# cat /dev/sdma_test 
opened channel 6, req lin 0
buffer 1 copy passed!
buffer 2 copy passed!
buffer 3 copy passed!

# rmmod imx-sdma-scatter-gather.ko 
SDMA test Driver Module Unloaded
```
This will produce the below debug message:

```bash
# dmesg
[...]
[ 4199.952354] SDMA test major number = 244
[ 4199.955523] SDMA test Driver Module loaded
[ 4205.304792] opened channel 6, req lin 0
[ 4205.308519] Got a DMA descriptor
[ 4205.310461] Got this cookie: 5
[ 4205.312221] waiting for DMA transaction...
[ 4205.313355] in dma_m2m_callback
[ 4205.316952] end - start = 6490
[ 4221.220570] opened channel 6, req lin 0
[ 4221.223762] buffer 1 copy passed!
[ 4221.225834] buffer 2 copy passed!
[ 4221.227934] buffer 3 copy passed!
[ 4256.051855] SDMA test Driver Module Unloaded
```
