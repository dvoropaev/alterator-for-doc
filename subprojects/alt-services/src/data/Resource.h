#pragma once

#include "Parameter.h"

/*
 *  Resource may consist of multiple properties
 */
class Resource : public TranslatableObject
{
public:
    enum class Type {
        Path,
        File,
        Unit,
        Port
    };

    enum Protocol {
        TCP = 1,
        UDP = 2,
        BOTH = TCP | UDP,
        NONE = 0
    };

    inline Resource(const QString& name, const Locales& locales,
             Type type,
             const QVariant& defaultValue, Parameter* override = nullptr)
        : TranslatableObject{name, locales}
        , m_type{type}
        , m_value{defaultValue}
        , m_override{override}
    {}


    inline Resource(const QString& name, const Locales& locales,
             bool tcp, bool udp,
             const QVariant& defaultValue, Parameter* override = nullptr)
        : TranslatableObject{name, locales}
        , m_type{Type::Port}
        , m_value{defaultValue}
        , m_override{override}
    {
        if ( tcp ) m_port_protocol |= TCP;
        if ( udp ) m_port_protocol |= UDP;
    }

    inline Type     type()  const { return m_type;  }

    inline QVariant value(Parameter::ValueScope scope) const {
        return m_override && m_override->value(scope)->isEnabled()
                   ? m_override->value(scope)->get()
                   : m_value;
    }

    inline int portProtocol(){ return m_port_protocol; }

    bool conflicts(const Resource* toDeploy) const;
    inline Parameter* override() const {return m_override;}

    inline Service* service() const {return m_service;}

protected:
    Type m_type;
    QVariant m_value;
    Parameter* m_override;
    int m_port_protocol{NONE};
    Service* m_service{};
    friend class Service;
};
Q_DECLARE_METATYPE(Resource*)

using ResourcePtr  = std::unique_ptr<Resource>;
