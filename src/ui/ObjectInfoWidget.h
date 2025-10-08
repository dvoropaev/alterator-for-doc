#pragma once

#include <QWidget>

#include "data/TranslatableObject.h"

namespace Ui {
class ObjectInfoWidget;
}

class ObjectInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectInfoWidget(QWidget *parent = nullptr);
    ~ObjectInfoWidget();
    void setObject(TranslatableObject* obj);

private:
    Ui::ObjectInfoWidget *ui;
};

