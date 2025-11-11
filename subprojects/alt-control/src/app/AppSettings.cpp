#include "AppSettings.h"
#include <QSettings>

class AppSettings::Private {
public:
    QSettings m_settings;
};

AppSettings::AppSettings(QObject *parent)
    : QObject{parent}
    , d{ new Private }
{}

AppSettings::~AppSettings() { delete d; }

#define PROPERTY(type, name, key, defaultVal) \
type AppSettings::name(){ return d->m_settings.value(key, defaultVal).value<type>(); } \
    void AppSettings::set_##name(const type & new_##name){ d->m_settings.setValue(key, new_##name); emit name##Changed(); }

PROPERTY(bool, tableDetailed,         "view/table/detailed", true)
PROPERTY(bool, tableShowCurrentState, "view/table/show_second_column", false)
