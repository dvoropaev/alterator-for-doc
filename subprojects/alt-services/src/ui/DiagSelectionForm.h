#pragma once

#include <QWidget>

#include "data/Service.h"
class Controller;

class DiagSelectionForm : public QWidget
{
    Q_OBJECT

public:
    explicit DiagSelectionForm(QWidget *parent = nullptr);
    ~DiagSelectionForm();

    void setController(Controller* c);
    void setService(Service* service);
    void setMode(DiagTool::Test::Mode mode);

    QAbstractItemModel* model();

    bool anySelected();

signals:
    void selectionChanged();

private slots:
    void on_selectAllButton_clicked();
    void on_clearButton_clicked();

private:
    void onDataChanged();

    class Private;
    Private* d;
};

