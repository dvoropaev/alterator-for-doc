#pragma once

#include <QWidget>
#include "data/Service.h"

class Controller;

class ServiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceWidget(QWidget *parent = nullptr);
    ~ServiceWidget();

    void setService(Service* s);
    void clear();

private slots:
    void on_comboBox_activated(int index);

public slots:
    void onStatusChanged();

private:
    class Private;
    Private* d;
};

