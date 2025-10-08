#ifndef CURRENTEDITIONINTERFACE_H
#define CURRENTEDITIONINTERFACE_H

#include <QStringList>

#include <optional>

namespace alt
{
class CurrentEditionInterface
{
public:
    static std::optional<QByteArray> license();
    static std::optional<QByteArray> info();
    static int set(const QString &editionId);
    static std::optional<QStringList> get();

public:
    CurrentEditionInterface() = delete;

public:
    CurrentEditionInterface(const CurrentEditionInterface &) = delete;
    CurrentEditionInterface(CurrentEditionInterface &&) = delete;
    CurrentEditionInterface &operator=(const CurrentEditionInterface &) = delete;
    CurrentEditionInterface &operator=(CurrentEditionInterface &&) = delete;

private:
    static std::optional<QByteArray> call(const QString &methodName);
};
} // namespace alt

#endif // CURRENTEDITIONINTERFACE_H
