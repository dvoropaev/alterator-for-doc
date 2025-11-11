#pragma once

#include <QObject>
#include <QMenu>
#include "app/ServicesApp.h"
#include "data/Service.h"


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


    bool call(Service* service, Parameter::Context ctx);
    bool updateStatus(Service* service);

    void start(Service* service);
    void stop(Service* service);

    bool diag(Service* service, bool post);

    bool findConflict(Service* deployService, Resource* deployResource, Service** other, Resource** conflicting);

    static const QString& actionName(Parameter::Context context);
    static const QIcon& actionIcon(Parameter::Context context);

signals:
    void beginRefresh();
    void endRefresh();
    void select(int);
    void stdout(const QString&);
    void stderr(const QString&);

public slots:
    void refresh();

private:
    class Private;
    Private* d;
};
