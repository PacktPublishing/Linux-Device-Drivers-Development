# Platform device and driver

This code consists of converting the driver in chapter 4 into a platform
driver. After running `make` command, there will be two modules:

* platform-dummy-char.ko
* platform-dummy-ins.ko

The fist module is our platform driver. The second one is a basic module whose
aim is to create a platform device that will match the
`platform-dummy-char` driver.


Prior to testing our driver, one should load the following modules:

```bash
# insmod platform-dummy-char.ko
# insmod platform-dummy-ins.ko
```

Once the modules loaded, one can see below message in debug output:

```bash
$ dmesg
[...]
[33117.715597] dummy_char major number = 241
[33117.715662] dummy char module loaded
[33117.715670] platform-dummy-char device added
```

One can print additional information by listing the sysfs content of the device:

```bash
$ ls -l /sys/devices/platform/platform-dummy-char.0/
total 0
lrwxrwxrwx 1 root root    0 oct.  12 16:40 driver -> ../../../bus/platform/drivers/platform-dummy-char
-rw-r--r-- 1 root root 4096 oct.  12 16:42 driver_override
-r--r--r-- 1 root root 4096 oct.  12 16:42 modalias
drwxr-xr-x 2 root root    0 oct.  12 16:42 power
lrwxrwxrwx 1 root root    0 oct.  12 16:42 subsystem -> ../../../bus/platform
-rw-r--r-- 1 root root 4096 oct.  12 16:40 uevent
```

Or by using `udevadm` tool:

```bash
$ udevadm info /dev/dummy_char 
P: /devices/platform/platform-dummy-char.0/dummy_char_class/dummy_char
N: dummy_char
E: DEVNAME=/dev/dummy_char
E: DEVPATH=/devices/platform/platform-dummy-char.0/dummy_char_class/dummy_char
E: MAJOR=241
E: MINOR=0
E: SUBSYSTEM=dummy_char_class
```

Of course, the behaviour remains the same as the char device tested on chapter :

```bash
# cat /dev/dummy_char 
# echo "blabla" > /dev/dummy_char 
# rmmod dummy-char.ko 

$ dmesg
[...]
[ 6753.573560] dummy_char major number = 241
[ 6753.573611] dummy char module loaded
[ 6753.573622] platform-dummy-char device added
[ 7081.034607] Someone tried to open me
[ 7081.034641] Can't accept any data guy
[ 7081.034649] Someone closed me
[ 7084.861861] Someone tried to open me
[ 7084.861887] Nothing to read guy
[ 7084.861906] Someone closed me
```
