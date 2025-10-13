#ifndef DYNAMICTRANSLATOR_H
#define DYNAMICTRANSLATOR_H

#include <QLocale>
#include <QObject>
#include <QTranslator>

#include <map>

class DynamicTranslator : public QObject
{
public:
    struct Translators
    {
        QTranslator *m_appTranslator;
        QTranslator *m_qtTranslator;
    };

public:
    void retranslate(const QLocale &locale = QLocale::system());
    void insert(QLocale::Language key, std::unique_ptr<Translators> value);

public:
    DynamicTranslator();
    ~DynamicTranslator() override;

public:
    DynamicTranslator(const DynamicTranslator &)            = delete;
    DynamicTranslator(DynamicTranslator &&)                 = delete;
    DynamicTranslator &operator=(const DynamicTranslator &) = delete;
    DynamicTranslator &operator=(DynamicTranslator &&)      = delete;

private:
    std::map<QLocale::Language, std::unique_ptr<Translators>> m_translators;
    Translators *m_currentTranslators;
};

#endif // DYNAMICTRANSLATOR_H
