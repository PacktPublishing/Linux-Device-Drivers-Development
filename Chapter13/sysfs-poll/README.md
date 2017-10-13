# Pollable sysfs attributes

After running `make` command, there will be one module:

* sysfs-poll.ko

After loading the module, this will create two sysfs attributes in `/sys/hello`:

* /sys/hello/trigger
* /sys/hello/notify

```bash
# insmod ./sysfs-poll.ko

# ls -l /sys/hello/
total 0
-rw-r--r-- 1 root root 4096 oct.  13 11:54 notify
-rw-r--r-- 1 root root 4096 oct.  13 11:54 trigger
```

For testing, one can build either compile either the file `sysfs-select-user.c`
or `sysfs-poll-user.c`.

```bash
$ gcc sysfs-select-user.c -o sysfs-select
```
Now one should execute sysfs-select binary as sudo:

```bash
# sudo ./sysfs-select 
```

Open another console, and then write anything into either `/sys/hello/trigger`
or `/sys/hello/notify`.

```bash
# echo "john" > /sys/hello/trigger
```
The app wainting will then print something like:

```bash
# sudo ./sysfs-select 
Change detected in /sys/hello/trigger
```
Same for writing in `/sys/hello/notify` (in console 1):

```bash
# echo "john" > /sys/hello/notify
```
which will produce below output (in console 2):

```bash
# ./sysfs-select 
Change detected in /sys/hello/notify
```

Additionally, one can use `dmesg` command for debug messages.
