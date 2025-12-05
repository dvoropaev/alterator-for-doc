#include "Resource.h"
#include "Service.h"

inline auto getResourceValue(const Resource* r)
{
    return r->value(r->service()->isDeployed()
                        ? Parameter::ValueScope::Edit
                        : Parameter::ValueScope::Current );
}

bool Resource::intersects(const Resource* other) const
{
    return m_type == other->m_type &&
           ::getResourceValue(this) == ::getResourceValue(other) &&
           ( m_type != Type::Port || m_port_protocol & other->m_port_protocol );
}
