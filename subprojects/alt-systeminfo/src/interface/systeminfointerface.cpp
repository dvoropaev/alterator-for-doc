#include "systeminfointerface.h"

#include "utility/constants.h"

namespace alt
{
std::optional<QStringList> SystemInfoInterface::getHostName()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_HOST_NAME_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getOperationSystemName()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_OPERATION_SYSTEM_NAME_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getBranch()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_BRANCH_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getKernel()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_KERNEL_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getCpu()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_CPU_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getArch()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_ARCH_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getGpu()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_GPU_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getMemory()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_MEMORY_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getDrive()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_DRIVE_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getMotherboard()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_MOTHERBOARD_METHOD_NAME);
}

std::optional<QStringList> SystemInfoInterface::getMonitor()
{
    return call<QStringList>(ALTERATOR_SYSTEMINFO1_GET_MONITOR_METHOD_NAME);
}

std::optional<QByteArray> SystemInfoInterface::getLicense()
{
    return call<QByteArray>(ALTERATOR_SYSTEMINFO1_GET_LICENSE_METHOD_NAME);
}

std::optional<QByteArray> SystemInfoInterface::getReleaseNotes()
{
    return call<QByteArray>(ALTERATOR_SYSTEMINFO1_GET_RELEASE_NOTES_METHOD_NAME);
}
} // namespace alt
