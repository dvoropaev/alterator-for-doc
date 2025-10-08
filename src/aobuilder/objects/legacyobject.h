#ifndef AOB_LEGACY_OBJECT_H
#define AOB_LEGACY_OBJECT_H

#include "object.h"

namespace ao_builder
{
class LegacyObject final : public Object
{
public:
    LegacyObject() = default;
    ~LegacyObject() override = default;

public:
    QString m_x_Alterator_URI{};
    QString m_x_Alterator_Help{};
    QString m_x_Alterator_UI{};
    QString m_x_Alterator_Internal_Name{};
    bool m_isTerminal{false};
};

} // namespace ao_builder
#endif // AOB_LEGACY_OBJECT_H
