# Fake regulator chip driver

After running `make` command, there will be two modules:

* dummy-regulator.ko
* reg-ins.ko

The fist module is our fake regulator driver, the second one is used
to create a plateform device that matches our first module. This way,
one can avoid updating the devicetree.

```bash
# insmod dummy-regulator.ko
# insmod reg-ins.ko
```

After loading both modules, one can print debug message, and see something
like below:

```bash
#dmesg
[...]
Dummy Core: Voltage range but no REGULATOR_CHANGE_VOLTAGE
Dummy Core: at 850 mV 
Dummy Fixed: 1300 mV 
```

Before loading the module, there were no `regulator.13` nor `regulator.14`

```bash
$ ls /sys/class/regulator/
regulator.0   regulator.11  regulator.3   regulator.6   regulator.9
regulator.1   regulator.12  regulator.4   regulator.7
regulator.10  regulator.2   regulator.5   regulator.8
```

After loading modules,

```bash
# ls /sys/class/regulator/
regulator.0   regulator.11  regulator.14  regulator.4   regulator.7
regulator.1   regulator.12  regulator.2   regulator.5   regulator.8
regulator.10  regulator.13  regulator.3   regulator.6   regulator.9

$ ls -l /sys/class/regulator/
[...]
lrwxrwxrwx 1 root root 0 oct.  11 17:52 regulator.13 -> ../../devices/platform/regulator-dummy.0/regulator/regulator.13
lrwxrwxrwx 1 root root 0 oct.  11 17:52 regulator.14 -> ../../devices/platform/regulator-dummy.0/regulator/regulator.14
```

One can also check some parameters, as defined in the driver:

```bash
$ cat /sys/class/regulator/regulator.13/name 
Dummy Core
$ cat /sys/class/regulator/regulator.14/name 
Dummy Fixed

$ cat /sys/class/regulator/regulator.14/type 
voltage
$ cat /sys/class/regulator/regulator.14/microvolts 
1300000
$ cat /sys/class/regulator/regulator.13/microvolts 
850000
```
