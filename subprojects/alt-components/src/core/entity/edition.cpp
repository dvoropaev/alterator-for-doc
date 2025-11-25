#include "edition.h"
#include "builder.h"
#include "constants.h"
#include "object.h"

namespace alt
{
Edition::Edition(const toml::table &data, bool installed)
    : Object(data)
    , license(buildField<std::string>(data, entity::EDITION_LICENSE_KEY_NAME).value_or(""))
    , desktopEnvironment(buildField<std::string>(data, entity::EDITION_DE_KEY_NAME).value_or(""))
    , installed(installed)
{
    const auto arches = *data[entity::EDITION_ARCHES_KEY_NAME].as_array();
    if (const toml::array *arches = data[entity::EDITION_ARCHES_KEY_NAME].as_array())
    {
        arches->for_each([this](const toml::node &arch) { this->arches.push_back(*arch.value<std::string>()); });
    }

    if (const toml::table *kflavours = data[entity::EDITION_KFLAVOURS_KEY_NAME].as_table())
    {
        this->defaultKflavour = *kflavours->get("default")->value<std::string>();
    }

    if (const toml::table *languages = data[entity::EDITION_LANGUAGES_KEY_NAME].as_table())
    {
        this->defaultLanguage = *languages->get("default")->value<std::string>();
    }

    if (const toml::table *sections = data[entity::EDITION_SECTIONS_KEY_NAME].as_table())
    {
        for (const auto &[sectionName, sectionKeys] : *sections)
        {
            toml::table table = *sectionKeys.as_table();
            table.insert(entity::COMPONENT_NAME_KEY_NAME, sectionName.str().data());
            this->sections.emplace_back(table);
        }
    }

    if (const toml::table *tags = data[entity::EDITION_TAGS_KEY_NAME].as_table())
    {
        for (const auto &[tagName, tagKeys] : *tags)
        {
            toml::table table = *tagKeys.as_table();
            table.insert(entity::COMPONENT_NAME_KEY_NAME, tagName.str().data());
            this->tags.emplace_back(table);
        }
    }
};
} // namespace alt
