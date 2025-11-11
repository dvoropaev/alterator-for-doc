#pragma once

#include <QWidget>
#include "data/Facilty.h"
#include <QRadioButton>

class StateForm : public QWidget
{
    Q_OBJECT

public:
    explicit StateForm(Facility::State* state, QWidget *parent = nullptr);
    ~StateForm();
    QRadioButton* radio();
    Facility::State* state();
    QVariant value();
    bool isChanged();

signals:
    void changed();

private:
    class Private;
    Private* d;
};
