#ifndef OBJECT_H
#define OBJECT_H

#define TOML_EXCEPTIONS 0
#include <map>
#include <string>
#include <toml++/toml.h>

namespace alt
{
class Object
{
public:
    std::string_view displayName(const std::string &lang) const;
    std::string_view comment(const std::string &lang) const;

public:
    Object() = default;
    Object(const toml::table &data);
    Object(const Object &) = default;
    Object(Object &&) = default;
    Object &operator=(const Object &object) = default;
    Object &operator=(Object &&object) = default;

    virtual ~Object() = 0;

public:
    const std::string name{};
    std::string type{};
    std::string category{}; // TODO(cherniginma): make it optional
    std::string dbusPath{};
    std::string icon{};

    std::map<std::string, std::string> displayNameStorage{};
    std::map<std::string, std::string> descriptionStorage{};
    std::map<std::string, std::string> commentStorage{};

    bool isDraft = false;

protected:
    static std::string_view localizedString(const std::map<std::string, std::string> &storage, const std::string &lang);
};
} // namespace alt

#endif // OBJECT_H
