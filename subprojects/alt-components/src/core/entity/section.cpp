#include "section.h"
#include "constants.h"

namespace alt
{
unsigned Section::weight_count = 0;

Section::Section(const toml::table &data)
    : Section(data, weight_count += 10)
{}

Section::Section(const toml::table &data, unsigned sort_weight)
    : Object(data)
    , sort_weight(sort_weight)
{
    if (const toml::array *components = data[entity::SECTION_COMPONENTS_KEY_NAME].as_array())
    {
        components->for_each(
            [this](const toml::node &component) { this->components.insert(*component.value<std::string>()); });
    }
}
} // namespace alt
