Designed to assign roles (include a group in other groups).

This is the equivalent of nested groups in Windows.
A role should be understood as a group that is a member of other groups - privileges (in terms of the libnss-role module, privilege groups are assigned to role groups).
Role groups are assigned directly to users, while privilege groups can be assigned both directly to users and to role groups.

A system role is a role defined in a [role name].role file located in the /etc/role.d/ directory.

**ALT Linux Wiki:** <https://www.altlinux.org/Alterator-roles>
