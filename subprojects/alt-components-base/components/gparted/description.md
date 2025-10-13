GParted is a partition editor for the graphically managing your disk partitions.

With GParted you can resize, copy, and move partitions without data loss, enabling you to:

* Grow or shrink your C: drive;
* Free up space for new operating systems;
* Create partitions to share data among operating systems;


It uses **libparted** to detect
and manipulate devices and partitiontables while several (optional)
filesystemtools provide support for filesystems not included in
libparted. These optional packages will be detected at runtime and
don't require a rebuild of GParted.
GParted is written in **C++** and uses **gtkmm** as Graphical Toolkit. The
general approach is to keep the GUI as simple as possible.

**Home page:** <https://gparted.org/>
