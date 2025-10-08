#ifndef KEYPROVIDER_H
#define KEYPROVIDER_H

#include <cassert>
#include <initializer_list>

namespace ao_builder
{
/* list of base keys */
enum KEY
{
    TYPE,
    NAME,
    GENERIC_NAME,
    COMMENT,
    TRY_EXEC,
    ICON,
    KEYWORDS,
    EXEC,
    MIMETYPE,
    CATEGORIES,
    WEIGHT,

    ENUM_SIZE
};

struct KeyProvider
{
    static const KeyProvider iniKeys;
    static const KeyProvider tomlKeys;

    template<KEY k>
    inline constexpr const char *key() const
    {
        static_assert(k >= 0 && k < ENUM_SIZE, "invalid key");
        return data[k];
    }

    KeyProvider() = delete;
    KeyProvider(const KeyProvider &) = delete;
    KeyProvider(KeyProvider &&) = delete;
    KeyProvider &operator=(const KeyProvider &) = delete;
    KeyProvider &operator=(KeyProvider &&) = delete;

private:
    const char *data[ENUM_SIZE]{};
    explicit constexpr KeyProvider(std::initializer_list<const char *> il) noexcept;
};

} // namespace ao_builder

#endif // KEYPROVIDER_H
