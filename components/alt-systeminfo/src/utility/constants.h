#ifndef CONSTANTS_H
#define CONSTANTS_H

const char *const ALTERATOR_SERVICE_NAME = "org.altlinux.alterator";
const char *const ALTERATOR_OBJECT_PATH = "/org/altlinux/alterator";
const char *const ALTERATOR_MANAGER_INTERFACE_NAME = "org.altlinux.alterator.manager";
const char *const ALTERATOR_SYSTEMINFO_OBJECT_PATH = "/org/altlinux/alterator/systeminfo";
const char *const ALTERATOR_APT_OBJECT_PATH = "/org/altlinux/alterator/apt";
const char *const ALTERATOR_GLOBAL_OBJECT_PATH = "/org/altlinux/alterator/global";
const char *const ALTERATOR_SYSTEMINFO1_INTERFACE_NAME = "org.altlinux.alterator.systeminfo1";
const char *const ALTERATOR_APT1_INTERFACE_NAME = "org.altlinux.alterator.apt1";
const char *const ALTERATOR_CURRENT_EDITION1_INTERFACE_NAME = "org.altlinux.alterator.current_edition1";
const char *const ALTERATOR_EDITION1_INTERFACE_NAME = "org.altlinux.alterator.edition1";

const char *const ALTERATOR_MANAGER_GET_ENV_VALUE_METHOD_NAME = "GetEnvValue";
const char *const ALTERATOR_MANAGER_SET_ENV_VALUE_METHOD_NAME = "SetEnvValue";
const char *const ALTERATOR_MANAGER_UNSET_ENV_VALUE_METHOD_NAME = "UnsetEnvValue";
const char *const ALTERATOR_MANAGER_GET_OBJECTS_METHOD_NAME = "GetObjects";

const char *const ALTERATOR_SYSTEMINFO1_GET_HOST_NAME_METHOD_NAME = "GetHostName";
const char *const ALTERATOR_SYSTEMINFO1_GET_OPERATION_SYSTEM_NAME_METHOD_NAME = "GetOperationSystemName";
const char *const ALTERATOR_SYSTEMINFO1_GET_LICENSE_METHOD_NAME = "GetLicense";
const char *const ALTERATOR_SYSTEMINFO1_GET_RELEASE_NOTES_METHOD_NAME = "GetReleaseNotes";
const char *const ALTERATOR_SYSTEMINFO1_GET_ARCH_METHOD_NAME = "GetArch";
const char *const ALTERATOR_SYSTEMINFO1_GET_BRANCH_METHOD_NAME = "GetBranch";
const char *const ALTERATOR_SYSTEMINFO1_GET_KERNEL_METHOD_NAME = "GetKernel";
const char *const ALTERATOR_SYSTEMINFO1_GET_CPU_METHOD_NAME = "GetCPU";
const char *const ALTERATOR_SYSTEMINFO1_GET_GPU_METHOD_NAME = "GetGPU";
const char *const ALTERATOR_SYSTEMINFO1_GET_MOTHERBOARD_METHOD_NAME = "GetMotherboard";
const char *const ALTERATOR_SYSTEMINFO1_GET_MEMORY_METHOD_NAME = "GetMemory";
const char *const ALTERATOR_SYSTEMINFO1_GET_DRIVE_METHOD_NAME = "GetDrive";
const char *const ALTERATOR_SYSTEMINFO1_GET_MONITOR_METHOD_NAME = "GetMonitor";

const char *const ALTERATOR_APT1_LAST_DIST_UPGRADE_METHOD_NAME = "LastDistUpgrade";

const char *const ALTERATOR_CURRENT_EDITION1_LICENSE_METHOD_NAME = "License";
const char *const ALTERATOR_CURRENT_EDITION1_INFO_METHOD_NAME = "Info";
const char *const ALTERATOR_CURRENT_EDITION1_DESCRIPTION_METHOD_NAME = "Description";
const char *const ALTERATOR_CURRENT_EDITION1_GET_METHOD_NAME = "Get";
const char *const ALTERATOR_CURRENT_EDITION1_SET_METHOD_NAME = "Set";

const char *const ALTERATOR_EDITION1_INFO_METHOD_NAME = "Info";
const char *const ALTERATOR_EDITION1_LICENSE_METHOD_NAME = "License";
const char *const ALTERATOR_EDITION1_DESCRIPTION_METHOD_NAME = "Description";

const char *const DISPLAY_NAME_KEY_NAME = "display_name";
const char *const NAME_KEY_NAME = "name";
const char *const DESKTOP_ENVIRONMENT_KEY_NAME = "name";

const char *const DEFAULT_LANGUAGE = "en";

#endif // CONSTANTS_H
