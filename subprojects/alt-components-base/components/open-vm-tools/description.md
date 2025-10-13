Provides a set of services and modules that enable several features in VMware products for better management of, and seamless user interactions with, guests.
It includes kernel modules for enhancing the performance of virtual machines running Linux or other VMware supported Unix like guest operating systems.

open-vm-tools enables the following features in VMware products:

* graceful execution of power operations (reboot and shutdown) in the gues
* execution of built-in or user configured scripts in the guest during various power operations
* running programs, commands and file system operations in the guest to enhance guest automation
* authentication for guest operations
* generation of heartbeat from guest to host for vSphere HA solution to determine guest's availabilty
* clock synchronization between guest and host
* quiescing guest file systems to allow host to capture file-system-consistent guest snapshot
* execution of pre-freeze and post-thaw scripts while quiescing guest file systems
* customization of the guest immediately after power on
* periodic collection of network, disk, and memory usage information from the guest
* resizing the graphical desktop screen of the guest
* shared Folders operations between host and guest file systems on VMware Workstation and VMware Fusion
* copying and pasting text, graphics, and files between guest and host or client desktops
* dragging and dropping files between guest and host UI
* periodic collection of running applications, services, and containers in the guest
* accessing content from GuestStore
* publishing data to Guest Data Publisher
* managing Salt-Minion desired state specified in a guest variable

**Home page:** <https://open-vm-tools.sourceforge.net/>
