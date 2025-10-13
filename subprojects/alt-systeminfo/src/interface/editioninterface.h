#ifndef EDITIONINTERFACE_H
#define EDITIONINTERFACE_H

#include <QStringList>

#include <optional>

namespace alt
{
class EditionInterface
{
public:
    static std::optional<QByteArray> license(const QString& object);
    static std::optional<QByteArray> description(const QString& object);
    static std::optional<QByteArray> info(const QString& object);

public:
    EditionInterface() = delete;

public:
    EditionInterface(const EditionInterface &) = delete;
    EditionInterface(EditionInterface &&) = delete;
    EditionInterface &operator=(const EditionInterface &) = delete;
    EditionInterface &operator=(EditionInterface &&) = delete;

private:
    static std::optional<QByteArray> call(const QString& object, const QString &methodName);
};
} // namespace alt

#endif // EDITIONINTERFACE_H
