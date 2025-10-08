#pragma once

#include <QMainWindow>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Controller;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setController(Controller* controller);

private slots:
    void on_listView_clicked(const QModelIndex &index);

    void on_searchBar_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QSortFilterProxyModel m_filter_model;
};
