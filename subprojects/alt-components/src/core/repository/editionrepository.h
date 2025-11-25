#ifndef EDITION_REPOSITORY_H
#define EDITION_REPOSITORY_H

#include "entity/edition.h"

#include "interface/ilogger.h"
#include <functional>
#include <map>
#include <optional>
#include <string>

namespace alt
{
class DBusManager;
class EditionRepository
{
public:
    std::optional<std::reference_wrapper<Edition>> current();
    std::optional<std::reference_wrapper<Edition>> get(const std::string &name);
    std::map<std::string, Edition> &getAll();
    void update();
    void setLogger(const std::shared_ptr<ILogger> &logger);

public:
    EditionRepository(const EditionRepository &) = delete;
    EditionRepository(EditionRepository &&) = delete;
    EditionRepository &operator=(const EditionRepository &) = delete;
    EditionRepository &operator=(EditionRepository &&) = delete;
    explicit EditionRepository(const std::shared_ptr<DBusManager> &source);
    ~EditionRepository();

private:
    class Private;
    std::unique_ptr<Private> p;
};
} // namespace alt

#endif // EDITION_REPOSITORY_H
