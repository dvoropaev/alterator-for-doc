Provides a userspace virtual filesystem where mounts are handled by separate processes that communicate via D-Bus.

Includes a GIO module that transparently adds GVFS support to all applications using the GIO API.
GVFS also allows mounted resources to be exposed to applications that do not use GIO by leveraging FUSE.

The component includes the GVFS server, the libgvfscommon library, GIO modules,
and backends for various protocols and sources: archive, computer, dav, ftp, gphoto2, http, localtest, network, sftp, and trash.

**Home page:** <https://wiki.gnome.org/Projects/gvfs>
