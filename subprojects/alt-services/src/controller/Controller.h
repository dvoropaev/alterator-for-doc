#pragma once

#include <QObject>
#include <QMenu>
#include "app/ServicesApp.h"
#include "data/Service.h"
#include "data/Action.h"


class QAbstractItemModel;

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller();
    ~Controller();

    Service* getByIndex(const QModelIndex&);
    Service* findByName(const QString& name);
    void selectByPath(const QString& path);

    QAbstractItemModel* model();
    QList<QAction*> tableActions();

    bool call(const Action& action);
    bool updateStatus(Service* service);

    bool start(Service* service);
    bool stop(Service* service);

    /*
     * For given resource, returns another (or same) resource,
     * which has same type and value and owned by some deployed service,
     * or nullptr if such resource is not owned.
     */
    Resource* findOwner(const Resource* resource);

    static const QString& actionName(Parameter::Context context);
    static const QIcon& actionIcon(Parameter::Context context);

signals:
    void beginRefresh();
    void endRefresh();
    void select(int);
    void actionBegin(const QString&);
    void actionEnd(bool);
    void stdout(const QString&);
    void stderr(const QString&);

public slots:
    void refresh();

private:
    bool diag(Service* service, DiagTool::Test::Mode mode, const Action::TestSet& tests);

    class Private;
    Private* d;
};
