#include "section.h"
#include "constants.h"

#include <utility>

namespace alt
{
unsigned Section::weight_count = 0;

Section::Section(const toml::table &data, QString name)
    : Section(data, std::move(name), weight_count += 10)
{}

Section::Section(const toml::table &data, QString name, unsigned sort_weight)
    : Object(data)
    , sort_weight(sort_weight)
{
    this->name = std::move(name);

    this->components = QSet<QString>{};
    if (const toml::array *components = data[SECTION_COMPONENTS_KEY_NAME].as_array())
    {
        components->for_each([this](const toml::node &component) {
            this->components.insert(QString::fromStdString(*component.value<std::string>()));
        });
    }
}
} // namespace alt
