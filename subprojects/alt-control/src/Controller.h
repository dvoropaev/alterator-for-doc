#pragma once

#include <QAbstractItemModel>

class Controller : public QObject
{
    Q_OBJECT

public:
    Controller();
    ~Controller();

    static const auto ValueListRole = Qt::ItemDataRole::UserRole+1;
    QAbstractItemModel* facilitiesModel();

    bool setValue(const QString& facility, const QString& value, QString& error);

signals:
    void beginRefresh();
    void   endRefresh();

public slots:
    void refresh();

private:
    class Private;
    Private* d;
};

