Alterator module for control mounting of USB block devices. Allows restricting access to the USB device's file system by UID/GID.
The module also provides the ability to view a log of USB device connection/disconnection events.

Module features:

* if no rule is created for a device, the service does not interfere with the USB device's mounting logic;
* if a rule is created for a device, the service mounts the block devices of the assigned USB device
to the directory /media/alt-usb-mount/$user_$group or /media/alt-usb-mount/root_$group if no user is specified;
* the service sets ACLs for the specified user and group on the directory where the mount point will be created;
* rules can only include existing local users and user groups;
* full (read/write) access to the USB device is granted to the assigned user and group;
* any user can unmount the device using standard OS tools;
* the service does not interfere with the file system permissions of the block devices;
* the recommended file system for removable media is exFAT.

**ALT Linux Wiki:** <https://www.altlinux.org/Alterator-usbmount>
