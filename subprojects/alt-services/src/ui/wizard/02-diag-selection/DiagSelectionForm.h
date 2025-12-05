#pragma once

#include <QWidget>
#include "data/Service.h"
#include "data/Action.h"

class DiagSelectionForm : public QWidget
{
    Q_OBJECT

public:
    explicit DiagSelectionForm(QWidget *parent = nullptr);
    ~DiagSelectionForm();

    void setService(Service* service, Action::TestSet& tests);
    void setMode(DiagTool::Test::Mode mode);

    QAbstractItemModel& model();

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

