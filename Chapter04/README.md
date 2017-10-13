# Character device driver

After running `make` command, there will be one module:

* dummy-char.ko

After loading the module, there will be a `/dev/dummy_char` char device. There
is also a class created for this device, `/sys/class/dummy_char_class/`. One can
actually print info on the device using `udevadm info` command:


```bash
# insmod dummy-char.ko
# udevadm info /dev/dummy_char
P: /devices/virtual/dummy_char_class/dummy_char
N: dummy_char
E: DEVNAME=/dev/dummy_char
E: DEVPATH=/devices/virtual/dummy_char_class/dummy_char
E: MAJOR=241
E: MINOR=0
E: SUBSYSTEM=dummy_char_class


$ ls -l /sys/class/dummy_char_class/
total 0
lrwxrwxrwx 1 root root 0 oct.  12 16:05 dummy_char -> ../../devices/virtual/dummy_char_class/dummy_char
$ cat /sys/class/dummy_char_class/dummy_char/dev 
241:0
```

For testing purpose, one can use `cat` and `read` commands:

```bash
# cat /dev/dummy_char 
# echo "blabla" > /dev/dummy_char 
# rmmod dummy-char.ko 

$ dmesg
[...]
[31444.392114] dummy_char major number = 241
[31444.392217] dummy char module loaded
[31452.575938] Someone tried to open me
[31452.575945] Nothing to read guy
[31452.575950] Someone closed me
[31483.210527] Someone tried to open me
[31483.210570] Can't accept any data guy
[31483.210578] Someone closed me
[31498.998185] dummy char module Unloaded
```
