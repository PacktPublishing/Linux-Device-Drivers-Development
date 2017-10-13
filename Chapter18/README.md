# Fake rtc driver

After running `make` command, there will be two modules:

* fake-rtc.ko
* rtc-ins.ko

The fist module is our fake rtc driver, the second one is used
to create a plateform device that matches our first module. This way,
one can avoid updating the devicetree.

```bash
 # insmod ./fake-rtc.ko
 # insmod ./rtc-ins.ko
```

After loading both modules, one can print debug message, and see something
like below:

```bash
# dmesg
[ 3390.052469] fake-rtc fake-rtc.0: rtc core: registered fake-rtc as rtc2
[ 3390.058033] fake-rtc fake-rtc.0: loaded; begin_time is 3390, rtc_time is 0
```


Before loading the module, there were no `rtc2`

```bash
# ls -l /dev/rtc*
lrwxrwxrwx    1 root     root             4 Aug 12 17:54 /dev/rtc -> rtc0
crw-------    1 root     root      254,   0 Aug 12 17:54 /dev/rtc0
crw-------    1 root     root      254,   1 Aug 12 17:54 /dev/rtc1
```

After loading modules,

```bash
# ls -l /dev/rtc*
lrwxrwxrwx    1 root     root             4 Aug 12 17:55 /dev/rtc -> rtc0
crw-------    1 root     root      254,   0 Aug 12 17:55 /dev/rtc0
crw-------    1 root     root      254,   1 Aug 12 17:55 /dev/rtc1
crw-------    1 root     root      254,   1 Aug 12 17:55 /dev/rtc2
```

One can also check other parameters as below:

```bash
# udevadm info /dev/rtc2
P: /devices/platform/fake-rtc.0/rtc/rtc2
N: rtc2
E: DEVNAME=/dev/rtc2
E: DEVPATH=/devices/platform/fake-rtc.0/rtc/rtc2
E: MAJOR=254
E: MINOR=2
E: SUBSYSTEM=rtc

# cat /sys/class/rtc/rtc2/name 
fake-rtc

# cat /sys/class/rtc/rtc2/date 
1970-01-01
```
