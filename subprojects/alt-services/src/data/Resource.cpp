#include "Resource.h"

bool Resource::conflicts(const Resource* toDeploy) const {
    if ( toDeploy == this ) return false;
    if ( m_type == toDeploy->m_type ) {
        return value(service() == toDeploy->service()
                         ? Parameter::ValueScope::Edit
                         : Parameter::ValueScope::Current)
                   == toDeploy->value(Parameter::ValueScope::Edit)
                && ( m_type != Type::Port || m_port_protocol & toDeploy->m_port_protocol);
    }
    return false;
}
