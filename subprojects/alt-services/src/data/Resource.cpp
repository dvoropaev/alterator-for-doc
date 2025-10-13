#include "Resource.h"

bool Resource::conflicts(const Resource* toDeploy) const {
    if ( m_type == toDeploy->m_type ) {
        return currentValue() == toDeploy->editValue()
                && ( m_type != Type::Port || m_port_protocol & toDeploy->m_port_protocol);
    }
    return false;
}
