#ifndef ADTTEST_H
#define ADTTEST_H

#include <QMap>
#include <QObject>
#include <QString>

class ADTTest : public QObject
{
    Q_OBJECT
    friend class ADTToolBuilder;

public:
    enum BusType
    {
        None,
        System,
        Session
    };

    ADTTest();
    ~ADTTest() = default;

    QString id() const;
    QString testId() const;
    QString toolId() const;
    QString displayName() const;
    QString icon() const;
    QString comment() const;
    QString stringStdout() const;
    QString stringStderr() const;
    QString log() const;
    QMap<QString, QString> nameLocaleStorage() const;
    QMap<QString, QString> descriptionLocaleStorage() const;
    BusType bus() const;

    void setLocale(QString locale);

    void clearLogs();

    int exitCode() const;
    void setExitCode(int newExitCode);

    void appendToStdout(QString text);
    QString getStdout();
    void appendToStderr(QString text);
    QString getStderr();

public slots:
    void getStdout(QString out);
    void getStderr(QString err);

signals:
    void getStdoutText(QString toolId, QString testId, QString line);
    void getStderrText(QString toolId, QString testId, QString line);

private:
    QString m_id;
    QString m_testId; //without suffix
    QString m_toolId;
    BusType m_bus;
    QString m_icon;
    QString m_displayName;
    QString m_comment;
    int m_exitCode;

    QString m_stringStdout;
    QString m_stringStderr;
    QString m_log{};

    QMap<QString, QString> m_displayNameLocaleStorage;
    QMap<QString, QString> m_commentLocaleStorage;
};

#endif // ADTTESTIMPL_H
