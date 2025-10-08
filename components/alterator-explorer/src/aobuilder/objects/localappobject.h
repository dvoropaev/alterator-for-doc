#ifndef AOB_LOCAL_APPLICATION_H
#define AOB_LOCAL_APPLICATION_H

#include "object.h"

namespace ao_builder
{
class LocalAppObject : public Object
{
public:
    LocalAppObject() = default;
    ~LocalAppObject() override = default;

    void setLocale(const QString &locale) override;

public:
    QString m_genericName{};            // Desktop Entry Section
    QString m_displayComment{};         // Desktop Entry Section
    QString m_try_Exec{};               // Desktop Entry Section
    QString m_desktopExec{};            // Desktop Entry Section
    QString m_displayKeywords{};        // Desktop Entry Section
    std::vector<QString> m_mimeTypes{}; // Desktop Entry Sections
    bool isTerminal{false};

    QString m_type{};                    // Alterator Entry Section
    QString m_exec{};                    // Alterator Entry Section
    std::vector<QString> m_interfaces{}; // Alterator Entry Section

    QMap<QString, QString> m_genericNameLocaleStorage{}; // Translations for m_genericName
    QMap<QString, QString> m_commentLocaleStorage{};     // Translations for m_displayComment
    QMap<QString, QString> m_keywordsLocaleStorage{};    // Translations for m_displayKeywords
};
} // namespace ao_builder

#endif // AOB_LOCAL_APPLICATION_H
