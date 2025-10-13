The QEMU Guest Agent is a daemon intended to be run within virtual machines.
It allows the hypervisor host to perform various operations in the guest, such as:

* get information from the guest
* set the guest’s system time
* read/write a file
* sync and freeze the filesystems
* suspend the guest
* reconfigure guest local processors
* set user’s password
* …

qemu-ga will read a system configuration file on startup (located at etc/qemu/qemu-ga.conf by default),
then parse remaining configuration options on the command line. For the same key, the last option wins, but the lists accumulate.

If an allowed RPCs list is defined in the configuration, then all RPCs will be blocked by default, except for the allowed list.

If a blocked RPCs list is defined in the configuration, then all RPCs will be allowed by default, except for the blocked list.

If both allowed and blocked RPCs lists are defined in the configuration,
then all RPCs will be blocked by default, then the allowed list will be applied, followed by the blocked list.

While filesystems are frozen, all except for a designated safe set of RPCs will blocked, regardless of what the general configuration declares.

**Home page:** <https://www.qemu.org/>
