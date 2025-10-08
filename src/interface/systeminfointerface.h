#ifndef SYSTEMINFOINTERFACE_H
#define SYSTEMINFOINTERFACE_H

#include <QStringList>

#include <optional>

namespace alt
{
class SystemInfoInterface
{
public:
    static std::optional<QStringList> getHostName();
    static std::optional<QStringList> getOperationSystemName();
    static std::optional<QStringList> getBranch();
    static std::optional<QStringList> getKernel();
    static std::optional<QByteArray> getLicense();
    static std::optional<QByteArray> getReleaseNotes();

    static std::optional<QStringList> getCpu();
    static std::optional<QStringList> getArch();
    static std::optional<QStringList> getGpu();
    static std::optional<QStringList> getMemory();
    static std::optional<QStringList> getDrive();
    static std::optional<QStringList> getMotherboard();
    static std::optional<QStringList> getMonitor();

public:
    SystemInfoInterface() = delete;

public:
    SystemInfoInterface(const SystemInfoInterface &) = delete;
    SystemInfoInterface(SystemInfoInterface &&) = delete;
    SystemInfoInterface &operator=(const SystemInfoInterface &) = delete;
    SystemInfoInterface &operator=(SystemInfoInterface &&) = delete;

private:
    template<typename T>
    static std::optional<T> call(const QString &methodName);
};
} // namespace alt

#include "systeminfointerface.inl"

#endif // SYSTEMINFOINTERFACE_H
