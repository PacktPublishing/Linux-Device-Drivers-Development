# IIO driver testing


After running `make` command, there will be two modules:

* iio-dummy-random.ko
* iio-ins.ko

The fist module is our dummy IIO driver, for a fake device, which exposes four
channels. The second one is a basic module whose aim is to create a platform
device that will match the `iio-dummy-random` driver. That, one can also declre
the device in the device tree.

First, one should load the following modules:

```bash
# iio-dummy-random.ko
# iio-ins.ko
```

Since the board on which I tested this driver had lready three IIO devices
(device0, device1, and device2), after loading our test drivers, another
IIO device (device3) will be added to the system:

On can check this through the sysfs interface:

```bash
~# ls -l /sys/bus/iio/devices/
[...]
lrwxrwxrwx 1 root root 0 Jul 31 20:26 iio:device3 -> ../../../devices/platform/iio-dummy-random.0/iio:device3
lrwxrwxrwx 1 root root 0 Jul 31 20:23 iio_sysfs_trigger -> ../../../devices/iio_sysfs_trigger
```
Now one knows the device exists, one can check or list the available channels
like below:

```bach
# ls /sys/bus/iio/devices/iio\:device3/
dev               in_voltage2_raw   name              uevent
in_voltage0_raw   in_voltage3_raw   power
in_voltage1_raw   in_voltage_scale  subsystem

# cat /sys/bus/iio/devices/iio:device0/name
iio_dummy_random
```

In case one need to print more information, one can use `udevadm` command as
following:

```bash
# udevadm info  /dev/iio\:device3 
P: /devices/platform/iio-dummy-random.0/iio:device3
N: iio:device3
E: DEVNAME=/dev/iio:device3
E: DEVPATH=/devices/platform/iio-dummy-random.0/iio:device3
E: DEVTYPE=iio_device
E: MAJOR=250
E: MINOR=3
E: SUBSYSTEM=iio
```
