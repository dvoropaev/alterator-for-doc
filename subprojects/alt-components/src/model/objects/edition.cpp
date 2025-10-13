#include "edition.h"
#include "constants.h"
#include "model/objects/object.h"
#include <QDebug>

namespace alt
{
Edition::Edition(const toml::table &data)
    : Object(data)
{
    this->license = QString::fromStdString(*data[EDITION_LICENSE_KEY_NAME].value<std::string>());
    this->desktopEnvironment = QString::fromStdString(*data[EDITION_DE_KEY_NAME].value<std::string>());

    const auto arches = *data[EDITION_ARCHES_KEY_NAME].as_array();
    if (const toml::array *arches = data[EDITION_ARCHES_KEY_NAME].as_array())
    {
        arches->for_each([this](const toml::node &arch) {
            this->arches.push_back(QString::fromStdString(*arch.value<std::string>()));
        });
    }

    if (const toml::table *kflavours = data[EDITION_KFLAVOURS_KEY_NAME].as_table())
    {
        this->defaultKflavour = QString::fromStdString(*kflavours->get("default")->value<std::string>());
    }

    if (const toml::table *languages = data[EDITION_LANGUAGES_KEY_NAME].as_table())
    {
        this->defaultLanguage = QString::fromStdString(*languages->get("default")->value<std::string>());
    }

    if (const toml::table *sections = data[EDITION_SECTIONS_KEY_NAME].as_table())
    {
        for (const auto &[sectionName, sectionKeys] : *sections)
        {
            this->sections.emplace_back(*sectionKeys.as_table(), QString(sectionName.str().data()));
        }
    }

    if (const toml::table *tags = data[EDITION_TAGS_KEY_NAME].as_table())
    {
        for (const auto &[tagName, tagKeys] : *tags)
        {
            this->tags.emplace_back(*tagKeys.as_table(), QString(tagName.str().data()));
        }
    }
};
} // namespace alt
