# Script for Managing Samba Shared Folders

*(This file is the English version of the documentation. The Russian version is available here: [README.ru.md](README.ru.md))*

This script is designed for convenient and reliable management of Samba shared folders (shares).
It allows you to create, delete, modify, start, stop and inspect Samba shares, as well as manage the corresponding Samba services.

---

## Main Features

* **Creating and deleting Samba shares** with configuration of:

  * share name (`share_name`);
  * directory path (`share_path`);
  * owner (`access_user`);
  * access type (`ro` — read-only, `rw` — read/write);
  * additional users list (`allowed_users`);
* **Managing Samba services** (`smb.service`, `nmb.service`);
* **Deploying and undeploying Samba** (enabling and disabling services);
* **ACL support** — automatic application of filesystem permissions;
* **Configuration backup and restore**;
* **Configuration validation** before applying changes (`testparm -s`);
* **Output of Samba state and all shares in JSON format**;
* **Detailed logging** of all actions for diagnostics and audit.

---

## File Locations

| File                           | Description                              | Path                                                            |
| ------------------------------ | ---------------------------------------- | --------------------------------------------------------------- |
| Main script                    | CLI script for managing Samba shares     | `/usr/bin/service-samba-shares`                                 |
| Backend file                   | Backend logic for Alterator              | `/usr/share/alterator/backends/service-samba-shares.backend`    |
| Alterator service description  | `.service` file for Alterator interface  | `/usr/share/alterator/service/service-samba-shares.service`     |
| Configuration backup directory | Storage for backups (used by the module) | `/var/lib/alterator/service/service-samba-shares/config-backup` |
| Data backup/restore directory  | Additional data used for backup/restore  | `/var/lib/alterator/service/service-samba-shares/backup`        |

---

## How the Script Works

### 1. Reading Input Data

The script expects **JSON** input:

* First, it reads JSON from **stdin** with a **10-second timeout**.
* If no data is provided within this time, an empty object is used:
  `"{}"`.
* The `operation` field can be:

  * either inside JSON (`{"operation": "create", ...}`),
  * or passed as the **first command-line argument**.

If `operation` is missing, **`status`** is executed by default.

---

### 2. Parameter Parsing

For convenience, parameters are grouped by operations inside JSON:

```json
{
  "operation": "create",
  "create": {
    "share_name": "data",
    "share_path": "/srv/data",
    "access_user": "alice",
    "access_type": "rw",
    "allowed_users": "bob,carol",
    "anonymous_enabled": true
  }
}
```

### Used Fields

For create (`.create.*`):

* `share_name` — share name (required);
* `share_path` — directory path (required);
* `access_user` — owner (required if anonymous access is disabled);
* `access_type` — `ro` or `rw` (required);
* `allowed_users` — additional users (optional);
* `anonymous_enabled` — enable guest access.

For update (`.update.*`):

* `share_name` — share name (required);
* other fields (`share_path`, `access_user`, `access_type`, `allowed_users`)
  are optional — if omitted, current values are used;
* `delete_prev_allowed` — if `true`, the old user list
  is cleared and replaced with the new one;
* `anonymous_enabled` — enables/disables guest access.

For delete (`.delete.*`):

* `share_name` — share name (required).

For backup/restore (top-level):

* `backup_name` — backup set name (optional).

---

### 3. Working with Samba Configuration and ACL

The script works with the main Samba configuration file:

```
File: /etc/samba/smb.conf
```

To read the current shares, it uses `testparm -s` and parses the output with `awk`.

For each share it collects:

* `share_name`,
* `share_path`,
* `access_type` (based on `writable` / `read only`),
* `valid users` → `allowed_users`.

After that:

#### create:

* checks that a share with such name does not already exist;

* creates the directory, sets owner (`chown`) and base permissions (`chmod 2750/2770`);

* forms the user list for `valid users` taking into account:

  ```text
  owner,
  allowed_users,
  ```

* `anonymous_enabled` (adds/removes `nobody`);

* applies ACL to the directory (using `setfacl`):

  * owner gets full access through standard permissions (`chmod`);
  * other users get `rx` or `rwx` depending on `access_type`;

* appends a new share section to `smb.conf`;

* validates the configuration with `testparm -s`;

* restarts `smb.service`.

#### update:

* reads current share parameters from `smb.conf` and filesystem;
* merges or replaces the `allowed_users` list depending on `delete_prev_allowed`;
* updates directory permissions and ACL (similar to `create`);
* removes the old share section and creates a new one with actual parameters;
* validates `smb.conf` and restarts `smb.service`.

#### delete:

* checks that the share section exists;
* removes only the configuration from `smb.conf`
  (the directory on disk is **not** deleted at this stage);
* validates the result using `testparm -s`;
* restarts `smb.service`.

In all cases, on errors (invalid config, failed service restart, etc.)
the script returns JSON with an error and a detailed message.

---

### 4. Service Management and Backups

The script can manage Samba services and configuration:

#### deploy:

* checks that Samba is installed;
* saves the original `/etc/samba/smb.conf` once
  to `/var/lib/alterator/service/service-samba-shares/config-backup/smb-original.conf`;
* enables and starts `smb` and `nmb` (`systemctl enable --now smb nmb`);
* returns JSON with services state.

#### undeploy:

* automatically performs a full backup of the current configuration and data (via `backup_all`);
* stops and disables `smb` and `nmb`;
* if the original `smb-original.conf` exists, restores it;
* returns JSON with information about auto-backup and service state.

#### backup:

* creates a backup of:

  * `smb.conf`,
  * directories of existing shares (if they exist on disk);

* stores them in:

  ```text
  /var/lib/alterator/service/service-samba-shares/config-backup/
  /var/lib/alterator/service/service-samba-shares/data-backup/
  ```

* returns JSON with paths to the created files.

#### restore:

* restores `smb.conf` from the specified or most recent backup;
* validates it using `testparm -s`;
* restarts `smb.service`;
* if a data archive exists — unpacks it into the filesystem;
* returns JSON with restore details.

#### status:

* queries `systemctl is-active smb.service` and `nmb.service`;
* collects the list of all shares via `get_samba_shares` + `augment_shares_with_owner`;
* returns JSON with the list of shares, their parameters and services state;
---

## `.service` File Description (Service Definition in Management System)

The `.service` file describes the **samba_shares** service, which provides a unified interface for managing Samba shares via the configuration system.

### Main Fields

* `type = "Service"` — defines that this is a service object.
* `name = "samba_shares"` — unique system name of the service.
* `category = "X-Alterator-Servers"` — category for grouping in the management UI.
* `persistent = true` — the service keeps its state and parameters between reboots.
* `display_name` and `comment` — localized names and descriptions (EN/RU).
* `icon = "network-server"` — icon used in the UI.

### Parameters

Service parameters are used to manage configuration of individual Samba shares.

* `share_name` — unique share name (required for creation).
* `share_path` — path to the shared directory.
* `access_user` — user granted access.
* `access_type` — access type: `ro` (read-only) or `rw` (read/write).
* `enabled` — flag indicating whether the share is visible/enabled in “Network”.

These parameters can be used in different contexts: configuration (`configure`), backup (`backup`), restore (`restore`), and status (`status`).

### Resources

* `smb_conf` — path to Samba configuration file `/etc/samba/smb.conf`.
* `smb_service` and `nmb_service` — references to system services (managed via systemd: `smb.service`, `nmb.service`).

### `shares` Array

Contains a list of all current Samba shares with their parameters:

* `share_name`
* `share_path`
* `access_user`
* `access_type`
* `enabled`

This array is shown in `status` mode, allowing convenient monitoring of all created shares in the UI.

---

## ACL

The script uses `setfacl` and `getfacl` to manage permissions.
The function responsible for this is `apply_fs_acl_for_valid_users`.

The workflow consists of several steps:

### 1. Clearing Old ACL

```bash
setfacl -b "$path"
```

All old ACLs are removed to avoid accumulating duplicates and garbage.

### 2. Setting Permission Mask

```bash
setfacl -m m::rwx "$path"
setfacl -d -m m::rwx "$path"
```

The mask (`mask`) defines the maximum allowed ACL permissions.
Here it is set to `rwx` so ACLs are not restricted by the mask.

### 3. Assigning Permissions to Each User

For each user in `allowed_users` (except the owner):

* if access type is `rw` — they receive `rwx`;
* if access type is `ro` — they receive `rx`.

```bash
setfacl -m u:"$user":"$perms" "$path"
setfacl -d -m u:"$user":"$perms" "$path"
```

The `-d` flag sets **default ACL**, so new files and directories
created inside automatically inherit these permissions.

---

### 3. Permission Structure After Applying

After running `create` or `update`,
you can inspect the resulting ACL using:

```bash
getfacl /home/test/test-create-upd
```

Example output:

```json
# file: home/test/test-create-upd
# owner: admin
# group: admin
# flags: -s-
user::rwx
user:user1:rwx
user:user2:rwx
user:nobody:rwx
group::rwx
mask::rwx
other::r-x
default:user::rwx
default:user:user1:rwx
default:user:user2:rwx
default:user:nobody:rwx
default:group::rwx
default:mask::rwx
default:other::r-x
```

* `user::rwx` — directory owner `admin`, full access.
* `user:user1:rwx` — `user1` can read, modify and create files.
* `user:user2:rwx` — `user2` can read, modify and create files.
* `user:nobody:rwx` — anonymous access is enabled; `nobody` has full access.
* `default:` — “inherited permissions” that will be automatically applied to new files and subdirectories.

---

### 4. Interaction Between ACL and Samba

When Samba reads `/etc/samba/smb.conf`, it gets the user list
from the `valid users` parameter.
ACL ensures consistency between this list and the actual filesystem permissions:

* if a user is present in `valid users`, but does not have filesystem access,
  Samba will not be able to work with the directory (access error);
* if ACL grants access, but the user is not in `valid users`,
  Samba will not allow access to the share.

Therefore, the script always synchronizes both levels:

* `valid users` parameter in Samba configuration;
* ACL on the filesystem.

---

### 5. Example of Mapping

| Access Type           | Samba (`valid users`) | Filesystem ACL                                  |
| --------------------- | --------------------- | ----------------------------------------------- |
| rw (read/write)       | admin, user1, user2   | admin:rwx, user1:rwx, user2:rwx                 |
| ro (read-only)        | admin, user1, user2   | admin:rwx, user1:rx, user2:rx                   |
| anonymous_access=true | `nobody` is added     | nobody:rx (read) or nobody:rwx (if share is rw) |

---

### 6. Implementation Details

* ACL is applied only to the share directory itself, not individual files inside.
  New files automatically inherit permissions thanks to `default:` ACL.
* When permissions are changed via `update`, old ACL are fully cleared
  to avoid conflicts between old and new users.
* The owner (`access_user`) does **not** receive ACL explicitly —
  they are controlled via standard permissions `chmod 2750/2770`.
* If anonymous access is enabled (`anonymous_enabled=true`),
  the user `nobody` is added to ACL.

---

## Localization

Documentation is available in two languages:

* 🇬🇧 English — this file [README.en.md](README.en.md)
* 🇷🇺 Russian — [README.ru.md](README.ru.md)

---