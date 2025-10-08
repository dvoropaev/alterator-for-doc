#ifndef CONSTANTS_H
#define CONSTANTS_H

const char *const ALTERATOR_MANAGER_SERVICE_NAME = "org.altlinux.alterator";
const char *const ALTERATOR_MANAGER_PATH = "/org/altlinux/alterator";
const char *const ALTERATOR_MANAGER_INTERFACE_NAME = "org.altlinux.alterator.manager";
const char *const ALTERATOR_MANAGER_GET_OBJECTS_METHOD_NAME = "GetObjects";
const char *const ALTERATOR_MANAGER_SET_ENV_METHOD_NAME = "SetEnvValue";

const char *const GLOBAL_PATH = "/org/altlinux/alterator/global";
const char *const COMPONENT_CATEGORIES_INTERFACE_NAME = "org.altlinux.alterator.component_categories1";
const char *const COMPONENT_CATEGORIES_LIST_METHOD_NAME = "List";
const char *const COMPONENT_CATEGORIES_INFO_METHOD_NAME = "Info";

const char *const APT1_INTERFACE_NAME = "org.altlinux.alterator.apt1";
const char *const APT1_PATH = "/org/altlinux/alterator/apt";
const char *const APT1_INSTALL_METHOD_NAME = "InstallAsync";
const char *const APT1_REMOVE_METHOD_NAME = "RemoveAsync";
const char *const APT1_UPDATE_METHOD_NAME = "UpdateAsync";
const char *const APT1_UPGRADE_METHOD_NAME = "Upgrade";
const char *const APT1_LAST_UPDATE_METHOD_NAME = "LastUpdate";
const char *const APT1_LAST_UPGRADE_METHOD_NAME = "LastDistUpgrade";
const char *const APT1_CHECK_INSTALL_METHOD_NAME = "CheckInstall";
const char *const APT1_CHECK_APPLY_METHOD_NAME = "CheckApply";
const char *const APT1_APPLY_METHOD_NAME = "ApplyAsync";

const char *const APT_INSTALL_STDOUT_SIGNAL_NAME = "apt1_install_stdout_signal";
const char *const APT_INSTALL_STDERR_SIGNAL_NAME = "apt1_install_stderr_signal";
const char *const APT_REMOVE_STDOUT_SIGNAL_NAME = "apt1_remove_stdout_signal";
const char *const APT_REMOVE_STDERR_SIGNAL_NAME = "apt1_remove_stderr_signal";
const char *const APT_UPDATE_STDOUT_SIGNAL_NAME = "apt1_update_stdout_signal";
const char *const APT_UPDATE_STDERR_SIGNAL_NAME = "apt1_update_stderr_signal";

const char *const RPM1_PATH = "/org/altlinux/alterator/rpm";
const char *const RPM1_INTERFACE_NAME = "org.altlinux.alterator.rpm1";
const char *const RPM1_INTERFACE_LIST_METHOD_NAME = "List";

const char *const COMPONENT1_INTERFACE_NAME = "org.altlinux.alterator.component1";
const char *const COMPONENT_INFO_METHOD_NAME = "Info";
const char *const COMPONENT_DESCRIPTION_METHOD_NAME = "Description";
const char *const COMPONENT_STATUS_METHOD_NAME = "Status";

const char *const ALTERATOR_SECTION_NAME = "Alterator Entry";

const char *const COMPONENT_NAME_KEY_NAME = "name";
const char *const COMPONENT_DRAFT_KEY_NAME = "draft";
const char *const COMPONENT_OBJECT_TYPE_KEY_NAME = "type";
const char *const COMPONENT_TYPE_KEY_VALUE = "component";
const char *const COMPONENT_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const COMPONENT_TYPE_KEY_NAME = "component";
const char *const COMPONENT_COMMENT_KEY_NAME = "comment";
const char *const COMPONENT_CATEGORY_KEY_NAME = "category";
const char *const COMPONENT_ICON_KEY_NAME = "icon";
const char *const COMPONENT_PACKAGES_KEY_NAME = "packages";
const char *const COMPONENT_TAGS_KEY_NAME = "tags";
const char *const COMPONENT_DEFAULT_LANUAGE = "en";

const char *const PACKAGE_META_KEY = "meta";
const char *const PACKAGE_KERNEL_MODULE_KEY = "kernel_module";
const char *const PACKAGE_IMAGE_IGNORE_KEY = "image_ignore";
const char *const PACKAGE_LANGUAGE_KEY = "language";
const char *const PACKAGE_DESKTOP_KEY = "desktop";
const char *const PACKAGE_ARCH_KEY = "arch";
const char *const PACKAGE_EXCLUDE_ARCH_KEY = "exclude_arch";

const char *const DEFAULT_CATEGORY_ID = "__default";
const char *const DEFAULT_CATEGORY_DISPLAY_NAME = "Other";
const char *const DEFAULT_CATEGORY_DISPLAY_NAME_RU = "Другое";

const char *const EDITION_NAME_KEY_NAME = "name";
const char *const EDITION_OBJECT_TYPE_KEY_NAME = "type";
const char *const EDITION_TYPE_KEY_VALUE = "edition";
const char *const EDITION_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const EDITION_LICENSE_KEY_NAME = "license";
const char *const EDITION_ARCHES_KEY_NAME = "arches";
const char *const EDITION_DE_KEY_NAME = "desktop_environment";
const char *const EDITION_KFLAVOURS_KEY_NAME = "kflavours";
const char *const EDITION_LANGUAGES_KEY_NAME = "languages";
const char *const EDITION_DEFAULT_LANUAGE = "en";
const char *const EDITION_SECTIONS_KEY_NAME = "sections";
const char *const EDITION_TAGS_KEY_NAME = "tags";

const char *const SECTION_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const SECTION_COMPONENTS_KEY_NAME = "components";
const char *const SECTION_DEFAULT_LANUAGE = "en";

const char *const DEFAULT_SECTION_NAME = "__other";
const int DEFAULT_SECTION_WEIGHT = 99;

const char *const CURRENT_EDITION_INTERFACE_NAME = "org.altlinux.alterator.current_edition1";
const char *const CURRENT_EDITION_INFO_METHOD_NAME = "Info";

const char *const BATCH_COMPONENTS_INERFACE_NAME = "org.altlinux.alterator.batch_components1";
const char *const BATCH_COMPONENT_CATEGORIES_INERFACE_NAME = "org.altlinux.alterator.batch_component_categories1";

const char *const SYSTEMINFO_PATH = "/org/altlinux/alterator/systeminfo";
const char *const SYSTEMINFO_INTERFACE_NAME = "org.altlinux.alterator.systeminfo1";
const char *const SYSTEMINFO_ARCH_METHOD_NAME = "GetArch";
const char *const SYSTEMINFO_KERNEL_METHOD_NAME = "GetKernel";
const char *const SYSTEMINFO_LOCALE_METHOD_NAME = "GetLocale";
const char *const SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_NAME = "ListDesktopEnvironments";

#endif // CONSTANTS_H
