#ifndef DBUS_CONSTANTS_H
#define DBUS_CONSTANTS_H

namespace dbus
{
const char *const INFO_METHOD_NAME = "Info";
const char *const INFO_METHOD_REPLY_SIGNATURE = "ayi";

const char *const ALTERATOR_MANAGER_SERVICE_NAME = "org.altlinux.alterator";
const char *const ALTERATOR_MANAGER_PATH = "/org/altlinux/alterator";
const char *const ALTERATOR_MANAGER_INTERFACE_NAME = "org.altlinux.alterator.manager";

const char *const ALTERATOR_MANAGER_SET_ENV_METHOD_NAME = "SetEnvValue";
const char *const ALTERATOR_MANAGER_SET_ENV_METHOD_REPLY_SIGNATURE = "";

const char *const GLOBAL_PATH = "/org/altlinux/alterator/global";
const char *const COMPONENT_CATEGORIES_INTERFACE_NAME = "org.altlinux.alterator.component_categories1";

const char *const RPM_PATH = "/org/altlinux/alterator/rpm";
const char *const RPM1_INTERFACE_NAME = "org.altlinux.alterator.rpm1";
const char *const RPM1_LIST_METHOD_NAME = "List";
const char *const RPM1_LIST_METHOD_REPLY_SIGNATURE = "asasi";

const char *const APT_PATH = "/org/altlinux/alterator/apt";
const char *const APT1_INTERFACE_NAME = "org.altlinux.alterator.apt1";

const char *const APT1_UPDATE_METHOD_NAME = "UpdateAsync";
const char *const APT1_UPDATE_METHOD_REPLY_SIGNATURE = "i";

const char *const APT1_LAST_UPDATE_METHOD_NAME = "LastUpdate";
const char *const APT1_LAST_UPDATE_METHOD_REPLY_SIGNATURE = "asasi";

const char *const APT1_LAST_UPGRADE_METHOD_NAME = "LastDistUpgrade";
const char *const APT1_LAST_UPGRADE_METHOD_REPLY_SIGNATURE = APT1_LAST_UPDATE_METHOD_REPLY_SIGNATURE;

const char *const APT1_CHECK_APPLY_METHOD_NAME = "CheckApply";
const char *const APT1_CHECK_APPLY_METHOD_REPLY_SIGNATURE = "asasi";

const char *const APT1_APPLY_METHOD_NAME = "ApplyAsync";
const char *const APT1_APPLY_METHOD_REPLY_SIGNATURE = "i";

const char *const APT_INSTALL_STDOUT_SIGNAL_NAME = "apt1_install_stdout_signal";
const char *const APT_INSTALL_STDERR_SIGNAL_NAME = "apt1_install_stderr_signal";
const char *const APT_UPDATE_STDOUT_SIGNAL_NAME = "apt1_update_stdout_signal";
const char *const APT_UPDATE_STDERR_SIGNAL_NAME = "apt1_update_stderr_signal";

const char *const COMPONENT1_INTERFACE_NAME = "org.altlinux.alterator.component1";
const char *const COMPONENT_INFO_METHOD_NAME = INFO_METHOD_NAME;

const char *const COMPONENT_DESCRIPTION_METHOD_NAME = "Description";
const char *const COMPONENT_DESCRIPTION_METHOD_REPLY_SIGNATURE = "ayi";

const char *const CURRENT_EDITION_INTERFACE_NAME = "org.altlinux.alterator.current_edition1";
const char *const CURRENT_EDITION_INFO_METHOD_NAME = INFO_METHOD_NAME;
const char *const CURRENT_EDITION_INFO_METHOD_REPLY_SIGNATURE = INFO_METHOD_REPLY_SIGNATURE;

const char *const BATCH_COMPONENTS_INTERFACE_NAME = "org.altlinux.alterator.batch_components1";
const char *const BATCH_COMPONENT_CATEGORIES_INERFACE_NAME = "org.altlinux.alterator.batch_component_categories1";
const char *const BATCH_INFO_METHOD_NAME = INFO_METHOD_NAME;
const char *const BATCH_INFO_METHOD_REPLY_SIGNATURE = "asi";
const char *const BATCH_STATUS_METHOD_NAME = "Status";
const char *const BATCH_STATUS_METHOD_REPLY_SIGNATURE = "ayi";

const char *const SYSTEMINFO_PATH = "/org/altlinux/alterator/systeminfo";
const char *const SYSTEMINFO_INTERFACE_NAME = "org.altlinux.alterator.systeminfo1";
const char *const SYSTEMINFO_ARCH_METHOD_NAME = "GetArch";
const char *const SYSTEMINFO_ARCH_METHOD_REPLY_SIGNATURE = "asi";

const char *const SYSTEMINFO_KERNEL_METHOD_NAME = "GetKernel";
const char *const SYSTEMINFO_KERNEL_METHOD_REPLY_SIGNATURE = "asi";

const char *const SYSTEMINFO_LOCALE_METHOD_NAME = "GetLocale";
const char *const SYSTEMINFO_LOCALE_METHOD_REPLY_SIGNATURE = "asi";

const char *const SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_NAME = "ListDesktopEnvironments";
const char *const SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_REPLY_SIGNATURE = "asi";

const char *const ERROR_REPLY_KEY_DATA = "data";
const char *const ERROR_INVALID_SIGNATURE_KEY_EXPECTED = "expected";
const char *const ERROR_INVALID_SIGNATURE_KEY_GOT = "got";
const char *const ERROR_SERVER_KEY_DATA = "data";
} // namespace dbus
#endif // DBUS_CONSTANTS_H
