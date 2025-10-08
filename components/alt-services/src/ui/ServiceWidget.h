#pragma once

#include <QWidget>
#include "data/Service.h"

class Controller;

class ServiceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceWidget(Controller* c, Service* s, QWidget *parent = nullptr);
    ~ServiceWidget();

private slots:
    void on_comboBox_activated(int index);

public slots:
    void updateStatus();

private:
    class Private;
    Private* d;
};

