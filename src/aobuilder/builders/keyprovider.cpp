#include "keyprovider.h"

namespace ao_builder
{

constexpr KeyProvider::KeyProvider(std::initializer_list<const char *> il) noexcept
{
    assert(il.size() == KEY::ENUM_SIZE);
    int i = 0;
    for (auto src = il.begin(); src != il.end(); src++)
        data[i++] = *src;
}

constexpr const KeyProvider KeyProvider::iniKeys{
    "Type",
    "Name",
    "GenericName",
    "Comment",
    "TryExec",
    "Icon",
    "Keywords",
    "Exec",
    "MimeType",
    "Categories",
    "Weight",
};

constexpr const KeyProvider KeyProvider::tomlKeys{
    "type",
    "name",
    "generic_name",
    "comment",
    "tryexec",
    "icon",
    "keywords",
    "exec",
    "mimetype",
    "categories",
    "weight",
};

} // namespace ao_builder
