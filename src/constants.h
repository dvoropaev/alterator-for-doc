#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Qt>

const char *const ALTERATOR_SERVICE_NAME           = "org.altlinux.alterator";
const char *const ALTERATOR_PATH                   = "/org/altlinux/alterator";
const char *const ALTERATOR_INTERFACE_NAME         = "org.altlinux.alterator.manager";
const char *const ALTERATOR_GETOBJECTS_METHOD_NAME = "GetObjects";

const char *const BASE_OBJECT_PATH    = "/org/altlinux/alterator/_base";
const char *const RPM_OBJECT_PATH     = "/org/altlinux/alterator/rpm";
const char *const APT_OBJECT_PATH     = "/org/altlinux/alterator/apt";
const char *const REPO_OBJECT_PATH    = "/org/altlinux/alterator/repo";
const char *const DEFAULT_OBJECT_PATH = "_default";
const char *const LOAD_OBJECT_PATH    = "_load";

const char *const BASE_INTERFACE_NAME    = "org.altlinux.alterator._base";
const char *const RPM_INTERFACE_NAME     = "org.altlinux.alterator.rpm1";
const char *const APT_INTERFACE_NAME     = "org.altlinux.alterator.apt1";
const char *const REPO_INTERFACE_NAME    = "org.altlinux.alterator.repo1";
const char *const DEFAULT_INTERFACE_NAME = "_default";
const char *const LOAD_INTERFACE_NAME    = "_load";

const char *const RPM_INTERFACE_DISPLAY_NAME  = "RPM";
const char *const APT_INTERFACE_DISPLAY_NAME  = "APT";
const char *const REPO_INTERFACE_DISPLAY_NAME = "Repo";

const char *const RPM_LIST_METHOD_NAME    = "List";
const char *const RPM_INSTALL_METHOD_NAME = "Install";
const char *const RPM_REMOVE_METHOD_NAME  = "Remove";
const char *const RPM_INFO_METHOD_NAME    = "PackageInfo";
const char *const RPM_FILES_METHOD_NAME   = "Files";

const char *const APT_CHECK_APPLY_METHOD_NAME        = "CheckApply";
const char *const APT_CHECK_INSTALL_METHOD_NAME      = "CheckInstall";
const char *const APT_CHECK_DIST_UPGRADE_METHOD_NAME = "CheckDistUpgrade";
const char *const APT_APPLY_ASYNC_METHOD_NAME        = "ApplyAsync";
const char *const APT_INSTALL_METHOD_NAME            = "Install";
const char *const APT_INSTALL_ASYNC_METHOD_NAME      = "InstallAsync";
const char *const APT_DIST_UPGRADE_ASYNC_METHOD_NAME = "DistUpgradeAsync";
const char *const APT_REMOVE_METHOD_NAME             = "Remove";
const char *const APT_UPDATE_METHOD_NAME             = "Update";
const char *const APT_UPDATE_ASYNC_METHOD_NAME       = "UpdateAsync";
const char *const APT_REINSTALL_METHOD_NAME          = "Reinstall";
const char *const APT_LIST_ALL_PACKAGES_METHOD_NAME  = "ListAllPackages";
const char *const APT_LAST_UPDATE_METHOD_NAME        = "LastUpdate";

const char *const APT_INSTALL_STDOUT_SIGNAL_NAME      = "apt1_install_stdout_signal";
const char *const APT_INSTALL_STDERR_SIGNAL_NAME      = "apt1_install_stderr_signal";
const char *const APT_UPDATE_STDOUT_SIGNAL_NAME       = "apt1_update_stdout_signal";
const char *const APT_UPDATE_STDERR_SIGNAL_NAME       = "apt1_update_stderr_signal";
const char *const APT_DIST_UPGRADE_STDOUT_SIGNAL_NAME = "apt1_dist_upgrade_stdout_signal";
const char *const APT_DIST_UPGRADE_STDERR_SIGNAL_NAME = "apt1_dist_upgrade_stderr_signal";

const char *const REPO_ADD_METHOD_NAME    = "Add";
const char *const REPO_REMOVE_METHOD_NAME = "Remove";
const char *const REPO_LIST_METHOD_NAME   = "List";

const char *const MANAGER_SET_ENV_VALUE_METHOD_NAME = "SetEnvValue";

const int INTERFACE_ROLE = Qt::UserRole + 1;

#endif // CONSTANTS_H
