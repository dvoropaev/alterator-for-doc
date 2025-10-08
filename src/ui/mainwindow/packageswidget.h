#ifndef PACKAGESWIDGET_H
#define PACKAGESWIDGET_H

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QShortcut>
#include <QWidget>

namespace Ui
{
class PackagesWidget;
}

class AptController;
class RpmController;
class RepoController;

class PackagesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PackagesWidget(QWidget *parent = nullptr);
    ~PackagesWidget();

public:
    void setActive(const QString &objectPath);
    void connect(AptController *controller);
    void connect(RpmController *controller);
    void connect(RepoController *controller);

public slots:
    void onManagerChanged(int index);

private slots:
    void changeEvent(QEvent *event) override;

signals:
    void aptSelected();

private:
    PackagesWidget(const PackagesWidget &)            = delete;
    PackagesWidget(PackagesWidget &&)                 = delete;
    PackagesWidget &operator=(const PackagesWidget &) = delete;
    PackagesWidget &operator=(PackagesWidget &&)      = delete;

private:
    std::unique_ptr<Ui::PackagesWidget> m_ui;
};

#endif // PACKAGESWIDGET_H
