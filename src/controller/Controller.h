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
    Controller(QObject* parent = nullptr, QWidget* window = nullptr);
    ~Controller();

    Service* getByIndex(const QModelIndex&);
    Service* findByName(const QString& name);
    void selectByPath(const QString& path);
    void importParameters(const ServicesApp::ParsedParameters& parameters);

    QAbstractItemModel* model();
    void addTableActions(QMenu*);

    int  status(Service* service, QByteArray& data);

    void prepareAction(Parameter::Context ctx, Service* service);
    bool call(Service* service, Parameter::Context ctx);

    void start(Service* service);
    void stop(Service* service);

    bool diag(Service* service, bool post);

    bool findConflict(Service* deployService, Resource** deployResource, Service** other, Resource** conflicting);

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
