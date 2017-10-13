# Fake pwm chip driver

After running `make` command, there will be two modules:

* fake-pwm.ko
* pwm-ins.ko

The fist module is our fake rtc driver, the second one is used
to create a plateform device that matches our first module. This way,
one can avoid updating the devicetree.

```bash
 # insmod ./fake-pwm.ko
 # insmod ./pwm-ins.ko
```

After loading both modules, one can print debug message, and see something
like below:

```bash
# dmesg
[ 3390.052469] fake-rtc fake-rtc.0: rtc core: registered fake-rtc as rtc2
[ 3390.058033] fake-rtc fake-rtc.0: loaded; begin_time is 3390, rtc_time is 0
```

Before loading the module, there were no `pwmchip4`

```bash
# ls -l /sys/class/pwm/
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip0 -> ../../devices/soc0/soc.0/2000000.aips-bus/2080000.pwm/pwm/pwmchip0
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip1 -> ../../devices/soc0/soc.0/2000000.aips-bus/2084000.pwm/pwm/pwmchip1
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip2 -> ../../devices/soc0/soc.0/2000000.aips-bus/2088000.pwm/pwm/pwmchip2
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip3 -> ../../devices/soc0/soc.0/2000000.aips-bus/208c000.pwm/pwm/pwmchip3
```

But after loading modules,

```bash
# ls /sys/class/pwm/
pwmchip0  pwmchip1  pwmchip2  pwmchip3  pwmchip4
# ls -l /sys/class/pwm/
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip0 -> ../../devices/soc0/soc.0/2000000.aips-bus/2080000.pwm/pwm/pwmchip0
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip1 -> ../../devices/soc0/soc.0/2000000.aips-bus/2084000.pwm/pwm/pwmchip1
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip2 -> ../../devices/soc0/soc.0/2000000.aips-bus/2088000.pwm/pwm/pwmchip2
lrwxrwxrwx    1 root     root             0 Aug 12 17:54 pwmchip3 -> ../../devices/soc0/soc.0/2000000.aips-bus/208c000.pwm/pwm/pwmchip3
lrwxrwxrwx    1 root     root             0 Aug 12 19:13 pwmchip4 -> ../../devices/platform/fake-pwm.0/pwm/pwmchip4
```

One can also check other parameters as below:

```bash
# cat /sys/class/pwm/pwmchip4/npwm 
3
# udevadm info /sys/class/pwm/pwmchip4/     
P: /devices/platform/fake-pwm.0/pwm/pwmchip4
E: DEVPATH=/devices/platform/fake-pwm.0/pwm/pwmchip4
E: SUBSYSTEM=pwm
```

The number of PWM that this chip provide is `3`, as specified in the driver.
One may need to export the pwn `#1` of this chip. It will then
appear in `/sys/class/pwm/pwmchip4/`.

```bash
# ls /sys/class/pwm/pwmchip4/
device     export     npwm       power      subsystem  uevent     unexport
# echo 1 > /sys/class/pwm/pwmchip4/export
# ls /sys/class/pwm/pwmchip4/
device     npwm       pwm1       uevent
export     power      subsystem  unexport

# ls /sys/class/pwm/pwmchip4/pwm1/
duty_cycle  enable      period      polarity    power/      uevent
```

One can enable a pwm as below:

```bash
# echo 1 > /sys/class/pwm/pwmchip4/pwm1/enable
# dmesg
[...]
[ 5067.023859] Somebody enabled PWM device number 1 of this chip
```

One should remember that pwm indexes start from `0`. I means for a chip that provide
`3` pwm, there could be `pwm0`, `pwm1`, and `pwm2`. Let us check that, by trying
to export `pmw3`, which will trig an error:

```bash
# echo 3 > /sys/class/pwm/pwmchip4/export 
sh: echo: write error: No such device
# echo 2 > /sys/class/pwm/pwmchip4/export 
# ls /sys/class/pwm/pwmchip4/
device     npwm       pwm1       subsystem  unexport
export     power      pwm2       uevent
```

Prior to unload the module, every exported pwm should be unexported.

```bash
# echo 1 > /sys/class/pwm/pwmchip4/unexport
# echo 2 > /sys/class/pwm/pwmchip4/unexport
```

