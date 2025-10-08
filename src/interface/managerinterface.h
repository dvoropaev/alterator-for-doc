#ifndef MANAGERINTERFACE_H
#define MANAGERINTERFACE_H

#include <QStringList>

#include <optional>

namespace alt
{
class ManagerInterface
{
public:
    static void setEnvValue(const QString &env, const QString &value);
    static void unsetEnvValue(const QString &env);
    static std::optional<QString> getEnvValue(const QString &env);
    static QStringList getObjects(const QString &interface);

public:
    ManagerInterface() = delete;

public:
    ManagerInterface(const ManagerInterface &) = delete;
    ManagerInterface(ManagerInterface &&) = delete;
    ManagerInterface &operator=(const ManagerInterface &) = delete;
    ManagerInterface &operator=(ManagerInterface &&) = delete;

private:
};
} // namespace alt

#endif // MANAGERINTERFACE_H
