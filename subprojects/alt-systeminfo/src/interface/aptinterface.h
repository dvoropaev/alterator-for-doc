#ifndef APTINTERFACE_H
#define APTINTERFACE_H

#include <QStringList>

#include <optional>

namespace alt
{
class AptInterface
{
public:
    static std::optional<QStringList> lastDistUpgrade();

public:
    AptInterface() = delete;

public:
    AptInterface(const AptInterface &) = delete;
    AptInterface(AptInterface &&) = delete;
    AptInterface &operator=(const AptInterface &) = delete;
    AptInterface &operator=(AptInterface &&) = delete;

private:
    static std::optional<QStringList> call(const QString &methodName);
};
} // namespace alt

#endif // APTINTERFACE_H
