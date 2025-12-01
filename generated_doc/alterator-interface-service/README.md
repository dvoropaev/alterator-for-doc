**alterator-interface-service**

[English](README.md) | [Русский](README.ru_RU.md)

Defines the D-Bus interface `org.altlinux.alterator.service1` for controlling service lifecycle actions.

---

# Overview
| Component | Location | Purpose |
| --------- | -------- | ------- |
| File `org.altlinux.alterator.service1.xml` | `/usr/share/dbus-1/interfaces/org.altlinux.alterator.service1.xml` | Declares the D-Bus introspection data for service management methods and signals. |
| File `org.altlinux.alterator.service1.policy` | `/usr/share/polkit-1/actions/org.altlinux.alterator.service1.policy` | Defines PolicyKit rules for invoking `org.altlinux.alterator.service1` methods. |

# Capabilities
- Describe lifecycle operations for services: deployment, configuration, startup, shutdown, backup, restoration, removal.
- Provide asynchronous delivery of process stdout and stderr via `service_stdout_signal` and `service_stderr_signal`.
- Outline authorization defaults for interactive and automated calls through PolicyKit actions.

# Integration
- Alterator clients consume the interface description from `/usr/share/dbus-1/interfaces/org.altlinux.alterator.service1.xml` to call methods on service backends.
- Authorization for individual methods follows the defaults declared in `/usr/share/polkit-1/actions/org.altlinux.alterator.service1.policy` (`Deploy`, `Backup`, `Undeploy`, `Restore` require `auth_admin_keep`; other operations allow unrestricted access).
- Parameters for operations that require configuration data (`Deploy`, `Configure`, `Undeploy`, `Backup`, `Restore`) are passed as a single string to `stdin` of the backend process.

# Interface documentation
- `service1` — exposes generic service lifecycle management methods and status reporting. See [service1.md](./service1.md).
