#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include "model/adttool.h"
#include "model/treemodel.h"

#include <memory>
#include <vector>
#include <QObject>

#include <model/builder/dbusconfig.h>

class BaseController : public QObject
{
public:
    Q_OBJECT
public:
    BaseController(std::shared_ptr<TreeModel> model);
    virtual ~BaseController() = default;

    //CommandLine functions
    virtual int listTools()                                               = 0;
    virtual int listToolTests(const QString &toolId)                      = 0;
    virtual int runAllTests(const QString &toolId)                        = 0;
    virtual int runTest(const QString &toolId, const QString &test)       = 0;
    virtual int getToolReport(const QString &toolId, const QString &file) = 0;
    virtual int runApp()                                                  = 0;

    virtual void onDetailsDialogShow(const QString &toolId, const QString &testId) = 0;

    //Tool
    virtual void setToolFilter(const QString &filter);
    virtual QStringList getFilteredTests(const QString &toolId);
    virtual QStringList getAllTests(const QString &toolId);
    virtual QString getToolFilter(const QString &toolId);
    virtual QString getToolDisplayName(const QString &toolId);
    virtual QString getToolComment(const QString &toolId);
    virtual QString getReportSuffix(const QString &toolId, ADTTool::BusType bus);
    virtual int getToolReports(QByteArray &array, ADTTool::BusType bus);
    virtual QString getAllTestLogs(const QString &toolId);
    virtual void resetToolTestsLogs(const QString &toolId, const QStringList &testsList);
    virtual bool hasSystemTests(const QString &toolId);
    virtual bool hasSessionTests(const QString &toolId);
    ADTTool *getTool(const QString &toolId);
    std::unique_ptr<QStandardItemModel> getVarsModel(const QString &toolId);

    QString getCurrentTool();
    bool setCurrentTool(const QString &toolId);

    //Test
    virtual QString getTestDisplayName(const QString &toolId, const QString &testId);
    virtual int getTestExitCode(const QString &toolId, const QString &testId);
    ADTTest *getTest(const QString &toolId, const QString &testId);

    QString getCurrentTest();

    bool setCurrentTest(const QString &toolId, const QString &testId);

public slots:
    virtual void on_serviceUnregistered() = 0;
    virtual void on_serviceRegistered()   = 0;
    virtual void on_serviceOwnerChanged() = 0;

    virtual void getStdoutFromTest(QString toolId, QString testId, QString text) = 0;
    virtual void getStderrFromTest(QString toolId, QString testId, QString text) = 0;

protected:
    virtual void onAllTasksBegin()    = 0;
    virtual void onAllTasksFinished() = 0;

    virtual void onBeginTask(QString toolId, QString testId)  = 0;
    virtual void onFinishTask(QString toolId, QString testId) = 0;

private:
    std::shared_ptr<TreeModel> m_model;
    QDBusConnection m_systemConn;
    QDBusConnection m_sessionConn;
    QString m_currentTool;
    QString m_currentTest;
    DBusConfig m_dbusConfig;
};

#endif // BASE_CONTROLLER_H
