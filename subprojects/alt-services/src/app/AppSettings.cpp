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

PROPERTY(bool, editorTableMode, "view/editor/table_mode", false)
PROPERTY(bool, tablesDetailed, "view/detailed_tables", false)
PROPERTY(bool, tablesDetailedMultiline, "view/detailed_tables_multiline", false)
PROPERTY(bool, lowerUnrequired, "view/editor/lower_nonrequired", false)
PROPERTY(QByteArray, mainWindowState, "view/mainwindow_state", {})
PROPERTY(Qt::ToolButtonStyle, toolButtonStyle, "view/toolbutton_style", Qt::ToolButtonStyle::ToolButtonTextBesideIcon)
