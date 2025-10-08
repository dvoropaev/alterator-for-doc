Provides a universal package management tool that enables secure and convenient software installation through a unified D-Bus interface,
regardless of distribution or system architecture.

PackageKit uses the native package management tools of the distribution (such as dnf, apt, etc.) via a set of compiled and scripted helpers.
It does not replace these tools, but offers a common abstraction layer that can be used by standard graphical and command-line package managers.

PackageKit itself is a system-activated daemon called packagekitd, which is launched on demand via systemd.
This means it runs only when a graphical or text-based package management interface is used, and stops when no longer needed.
As a result, it does not slow down system startup or consume resources in the background.

**ALT Linux Wiki:** <https://www.altlinux.org/Packagekit>

**Home page:**  
<https://www.freedesktop.org/software/PackageKit/pk-intro.html>
