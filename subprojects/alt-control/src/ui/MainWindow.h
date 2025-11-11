#pragma once

#include <QMainWindow>

class Controller;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setController(Controller* controller);

private slots:
    void on_facilityView_clicked(const QModelIndex &index);
    void on_applyButton_clicked();

private:
    class Private;
    Private* d;
};
