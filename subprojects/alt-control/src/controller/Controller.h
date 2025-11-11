#pragma once

#include "data/FacilityModel.h"

class Controller : public QObject
{
    Q_OBJECT

public:
    Controller();
    ~Controller();

    FacilityModel* facilitiesModel();
    bool getValue(Facility* facility, QString& error);
    bool setValue(Facility* facility, const QString& value, QString& error);
    bool getStates(Facility* facility, QString& error);

signals:
    void beginRefresh();
    void   endRefresh();

public slots:
    void refresh();

private:
    class Private;
    Private* d;
};

