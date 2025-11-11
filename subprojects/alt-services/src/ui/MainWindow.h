#pragma once

#include <QMainWindow>
#include <QSortFilterProxyModel>

class Controller;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addWidget(int index, QWidget* w);

protected:
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

public slots:
    void onRefreshStart();
    void onRefreshEnd();
    void onStatusChanged();

private slots:
    void on_treeView_activated(const QModelIndex &index);
    void on_searchBar_textChanged(const QString &arg1);
    void on_actionImport_triggered();
    void on_actionRefreshCurrent_triggered();
    void on_actionStart_triggered();
    void on_actionStop_triggered();
    void onContextActionTriggered();

private:
    class Private;
    Private* d;
};
