# Fake ethernet driver

After running `make` command, there will be two modules:

* fake-eth.ko
* eth-ins.ko

The fist module is ou fake ethernet driver, the second one is used
to create a plateform device that matches our first module. This way,
one can avoid updating the devicetree.

```bash
 # insmod ./fake-eth.ko
 # insmod ./eth-ins.ko
```

After loading both modules, one can print debug message, and see something
like below:

```bash
#dmesg
[146626.222072] fake-eth added
[146689.422000] fake-fake removed
[146698.060074] fake eth device initialized
[146698.060285] fake-eth added
[146698.087297] IPv6: ADDRCONF(NETDEV_UP): eth0: link is not ready
```


Before loading the module, there were no `eth0`

```bash
$ ifconfig -a

enp3s0f1  Link encap:Ethernet  HWaddr 34:97:f6:ba:5e:d7  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:49727 errors:0 dropped:0 overruns:0 frame:0
          TX packets:49727 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1 
          RX bytes:8431650 (8.4 MB)  TX bytes:8431650 (8.4 MB)

[...]
```

After loading modules,

```bash
$ ifconfig -a

enp3s0f1  Link encap:Ethernet  HWaddr 34:97:f6:ba:5e:d7  
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

eth0      Link encap:Ethernet  HWaddr 00:00:00:00:00:00  
          BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:49727 errors:0 dropped:0 overruns:0 frame:0
          TX packets:49727 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1 
          RX bytes:8431650 (8.4 MB)  TX bytes:8431650 (8.4 MB)

[...]
```

One can also configure the interface like `ifconfig eth0 192.168.1.45`,
and then

```bash
$ ifconfig eth0

eth0      Link encap:Ethernet  HWaddr 00:00:00:00:00:00  
          inet addr:192.168.1.45  Bcast:192.168.1.255  Mask:255.255.255.0
          BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
```
