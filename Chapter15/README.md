# GPIO controller drivers test

After running `make` command, there will be three modules:

* mcp23016.ko
* fake-gpio-chip.ko
* fake-gpio-ins.ko

The fist module is the driver for the MCP23016, which is an I2C expander. The
second one is a fake gpiochip driver, for people not having such MCP chip.
The goal of the third module is just to create a platform device that matches
our fake-gpio-chip module.

## Fake gpiochip driver

Prior to testing the fake gpio driver, one should load the following modules:

```bash
# insmod fake-gpio-chip.ko
# insmod fake-gpio-ins.ko
```

For testing purpose, one can list available gpiochip on the system

```bash
# ls -l /sys/class/gpio/gpiochip*
[...]
lrwxrwxrwx    1 root     root             0 Aug 13 17:30 /sys/class/gpio/gpiochip448 -> ../../devices/platform/fake-gpio-chip.0/gpio/gpiochip448
[...]
```

Additionally, `udevadm` may print more informations:

```bash
# udevadm info /sys/class/gpio/gpiochip448
P: /devices/platform/fake-gpio-chip.0/gpio/gpiochip448
E: DEVPATH=/devices/platform/fake-gpio-chip.0/gpio/gpiochip448
E: SUBSYSTEM=gpio
```

Parameters defined in the drivers may be checked as below:

```bash
# cat /sys/class/gpio/gpiochip448/
base       device/    label      ngpio      power/     subsystem/ uevent
# cat /sys/class/gpio/gpiochip448/ngpio 
16
# cat /sys/class/gpio/gpiochip448/base 
448
# cat /sys/class/gpio/gpiochip448/label 
fake-gpio-chip
```

Feel free to improve this driver, adding for example debug message when one
exports GPIOs, change their directions or their values.


## MCP23016 driver test

Prior to loading this module utomatically, one should declare the device
in the devicetree, as the child of i2c controller to which the expander is
connected. The node should look like:

```json
&i2c3 {
    [...]

    expander_0: mcp23016@20 {
        compatible = "microchip,mcp23016";
        gpio-controller;
        #gpio-cells = <2>;
        reg = <0x20>;
    };
};
```
If the system is not aware of the module, one can manually load it as below:

```bash
# insmod mcp23016.ko
```

After this module loaded, it creates a `/sys/class/gpio/gpiochip464` entry
on my system (this may be different on yours).

```bash
# udevadm info /sys/class/gpio/gpiochip464
P: /devices/soc0/soc.0/2100000.aips-bus/21a8000.i2c/i2c-2/2-0026/gpio/gpiochip464
E: DEVPATH=/devices/soc0/soc.0/2100000.aips-bus/21a8000.i2c/i2c-2/2-0026/gpio/gpiochip464
E: SUBSYSTEM=gpio
```
Some properties of the chip can be checked as below:

```bash
# ls /sys/class/gpio/gpiochip464/ 
base       device/    label      ngpio      power/     subsystem/ uevent
# cat /sys/class/gpio/gpiochip464/base 
464
# cat /sys/class/gpio/gpiochip464/label 
mcp23016
# cat /sys/class/gpio/gpiochip464/ngpio 
16
*/
```

The base of this gpio controller is 464, which is global to the system.
It means that, in one need to export the first (gpio0) and third (gpio2) gpios
of this ontroller, on should write 464 and 464 into `/sys/class/gpio/export`
file. 
