# Input device drivers test

There are three drivers example in this chapter. After running `make` command,
there will be two modules:

* input-button.ko
* input-polled-button.ko
* polled-ins.ko

The fist module implements the IRQ based input device, and the second one
implements a polled input device. Both rely on i.MX6 from NXP. The third one
should be used only on x86 platform, and will create a platform device that
matches the polled input module.

The polled device can be partially tested on a PC. Prior to (partially) test
this module on a PC, one should comment each line below in the code
(`input-polled-button.c`):

```c
    input_report_key(poll_dev->input, BTN_0, gpiod_get_value(priv->btn_gpiod) & 1);
    input_sync(poll_dev->input);
    gpiod_put(priv->btn_gpiod);
```

## Polled input driver

On i.MX6 platforms, one can use the devicetree node below:

```json
input_button_device {
    compatible = "packt,input-polled-button";
    button-gpios = <&gpio2 1 GPIO_ACTIVE_LOW>;
};
```
The module can then be loaded:

```bash
# insmod input-polled-button.ko
```
On x86 platform, one need to load polled-ins.ko too.

```bash
# insmod polled-ins.ko
```

after the module is loded, one can see the following debug message:

```bash
$ dmesg
[...]
[  335.044171] input-polled-button added
[  342.053497] input: Packt input polled Btn as /devices/platform/input-polled-button.0/input/input14
```

Now let's print some informations about the device using `udevadm` command:

```bash
$ udevadm info /dev/input/event14
P: /devices/platform/input-polled-button.0/input/input14/event14
N: input/event14
S: input/by-path/platform-input-polled-button.0-event
E: DEVLINKS=/dev/input/by-path/platform-input-polled-button.0-event
E: DEVNAME=/dev/input/event14
E: DEVPATH=/devices/platform/input-polled-button.0/input/input14/event14
E: ID_INPUT=1
E: ID_PATH=platform-input-polled-button.0
E: ID_PATH_TAG=platform-input-polled-button_0
E: MAJOR=13
E: MINOR=78
E: SUBSYSTEM=input
E: USEC_INITIALIZED=342093232
```

since the polled input driver can be partially tested on PC, the module loading
may fail with below error in debug:

```bash
$ dmesg
[...]
[  277.561986] input_polled_button: Unknown symbol input_allocate_polled_device (err 0)
[  277.562012] input_polled_button: Unknown symbol input_free_polled_device (err 0)
[  277.562033] input_polled_button: Unknown symbol input_register_polled_device (err 0)
[  277.562050] input_polled_button: Unknown symbol input_unregister_polled_device (err 0)
```

This means our module tries to use non exported symbols. By using `depmod`,
one can see our module dependencies, and load these prior to load our polled
input module

```bash
$ modinfo ./input-polled-button.ko 
filename:       /home/ldd/sources/chapter-17/./input-polled-button.ko
description:    Polled input device
author:         John Madieu <john.madieu@gmail.com>
license:        GPL
srcversion:     9B24B13C64ECB6B10C912B3
depends:        input-polldev
vermagic:       4.4.0-97-generic SMP mod_unload modversions 

$ sudo modprobe  input-polldev 
```
That is all for x86 based platform. Now on our i.MX6 based board, one can check
the states of the gpio associated with this device as following:

```bash
# cat /sys/kernel/debug/gpio  | grep button
 gpio-193 (button-gpio         ) in  hi
# cat /sys/kernel/debug/gpio  | grep button
 gpio-193 (button-gpio         ) in  lo
```

One should run this command with the button pushed first, and then with the
button released. The tool `evtest` cant be used to show the keycode
corresponding to the button states:

```bash
# evtest /dev/input/event14 
Input driver version is 1.0.1
Input device ID: bus 0x0 vendor 0x0 product 0x0 version 0x0
Input device name: "Packt input polled Btn"
Supported events:
  Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 256 (BTN_0)
Properties:
Testing ... (interrupt to exit)

```

## IRQ based input driver

One can use the below node in the devicetree:

```json
input_button_device {
    compatible = "packt,input-button";
    button-gpios = <&gpio2 1 GPIO_ACTIVE_LOW>;
};
```
You should make sure the GPIO is actually free. The command below will load the
module:

```bash
# insmod input-polled-button.ko
```

using `udevadm` on the device will print the following informations:

```bash
# udevadm info /dev/input/event0 
P: /devices/platform/input-button.0/input/input0/event0
N: input/event0
S: input/by-path/platform-input-button.0-event
E: DEVLINKS=/dev/input/by-path/platform-input-button.0-event
E: DEVNAME=/dev/input/event0
E: DEVPATH=/devices/platform/input-button.0/input/input0/event0
E: ID_INPUT=1
E: ID_PATH=platform-input-button.0
E: ID_PATH_TAG=platform-input-button_0
E: MAJOR=13
E: MINOR=64
E: SUBSYSTEM=input
E: USEC_INITIALIZED=74842430
```

One can check whether the IRQ line has been acquired by the driver using the
below command:

```bash
# cat /proc/interrupts
147:          0          0          0          0       GIC 147  20e0000.hdmi_video
150:        481          0          0          0       GIC 150  2188000.ethernet
151:          0          0          0          0       GIC 151  2188000.ethernet
152:         75          0          0          0       GIC 152  mx6-pcie-msi
160:          0          0          0          0  gpio-mxc   0  packt-input-button
161:          0          0          0          0  gpio-mxc   1  2198000.usdhc cd
250:          0          0          0          0  gpio-mxc  26  2-0026
286:          0          0          0          0  gpio-mxc  30  ad7606, ad7606, ad7606
351:          0          0          0          0  gpio-mxc  31  2-0027
352:          0          0          0          0  gpio-mxc   0  2194000.usdhc cd
```

the line with `packt-input-button` correspond to the IRQ line for this input device
