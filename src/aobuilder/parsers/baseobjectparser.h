#ifndef AOB_BASEOBJECTPARSER_H
#define AOB_BASEOBJECTPARSER_H

#include "objectparserinterface.h"

namespace ao_builder
{
class BaseObjectParser : public ObjectParserInterface
{
public:
    BaseObjectParser() = default;
    ~BaseObjectParser() override = default;

public:
    const Nodes &getSections() override;
    bool parse(const QString &data) override;

    QString getValue(const QString &key, const QString &section = {}) override;

private:
    Nodes m_nodes{};
};

} // namespace ao_builder

#endif // BASEOBJECTPARSER_H
