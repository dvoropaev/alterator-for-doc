#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include "datasource/datasourceinterface.h"
#include "model/packagessortfilterproxymodel.h"

#include <QFutureWatcher>
#include <QStandardItemModel>
#include <QtConcurrent/QtConcurrent>

class MainWindow;
class MainController;

class BaseController : public QObject
{
    Q_OBJECT
public:
    BaseController(DataSourceInterface *dataSource, MainController *mc);
    virtual ~BaseController();

public:
    virtual void retranslate(const QLocale &locale);
    virtual void updateModel();
    PackagesFilterProxyModel *getModel();
    void selected();
    void deSelected();
    void setWindow(std::shared_ptr<MainWindow> window);

signals:
    void progressChanged(int state, QString message = {});
    void wait(bool start, QString message = {});
    void requestToUpdateData();
    void showError(int, const QStringList &);
    void openWaitDialog();
    void appendWaitMessage(const QString &message);
    void closeWaitDialog();
    void showLoadingIndicator(bool visible);

protected:
    DataSourceInterface *getDataSource();
    virtual void updateModelHeaderData();

private:
    BaseController(const BaseController &)            = delete;
    BaseController(BaseController &&)                 = delete;
    BaseController &operator=(const BaseController &) = delete;
    BaseController &operator=(BaseController &&)      = delete;

protected:
    std::shared_ptr<MainWindow> m_window;
    DataSourceInterface *m_dataSource;
    std::unique_ptr<QStandardItemModel> m_model;
    std::unique_ptr<PackagesFilterProxyModel> m_proxyModel;
};

#endif // BASECONTROLLER_H
