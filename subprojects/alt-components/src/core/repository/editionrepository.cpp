#include "editionrepository.h"

#include "dbus/dbusmanager.h"

#include <functional>

namespace alt
{
class EditionRepository::Private
{
public:
    std::map<std::string, Edition> editions{};
    std::shared_ptr<ILogger> logger;
    std::shared_ptr<DBusManager> source;
};

EditionRepository::EditionRepository(const std::shared_ptr<DBusManager> &source)
    : p(std::make_unique<Private>())
{
    p->source = source;
}

EditionRepository::~EditionRepository() = default;

std::optional<std::reference_wrapper<Edition>> EditionRepository::get(const std::string &name)
{
    auto &editions = p->editions;
    auto result = editions.find(name);
    if (result == editions.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

std::optional<std::reference_wrapper<Edition>> EditionRepository::current()
{
    auto &editions = p->editions;
    auto result = std::find_if(editions.begin(), editions.end(), [](const std::pair<std::string, Edition> &pair) {
        return pair.second.installed;
    });
    if (result == editions.end())
    {
        return std::nullopt;
    }

    return std::make_optional(std::ref(result->second));
}

void EditionRepository::setLogger(const std::shared_ptr<ILogger> &logger)
{
    p->logger = logger;
}

void EditionRepository::update()
{
    p->editions.clear();
    const auto entry = p->source->getCurrentEdition();
    if (!entry || entry->empty())
    {
        return;
    }
    const auto parse_result = toml::parse(entry.value());
    if (parse_result.failed())
    {
        if (p->logger)
        {
            p->logger->write(ILogger::Level::Warning,
                             "Failed to parse edition:" + std::string(parse_result.error().description()));
        }
    }
    auto edition = Edition(parse_result.table(), true);
    p->editions.emplace(edition.name, std::move(edition));
}
} // namespace alt
