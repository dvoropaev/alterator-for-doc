#ifndef RPMMANAGERWIDGET_H
#define RPMMANAGERWIDGET_H

#include "basemanagerwidget.h"
#include "controllers/rpmcontroller.h"
#include "filterwidget.h"

#include <QPushButton>
#include <QStandardItemModel>

#include <memory>

class RpmManagerWidget : public BaseManagerWidget
{
    Q_OBJECT

public:
    explicit RpmManagerWidget(QWidget *parent = nullptr);
    ~RpmManagerWidget();

public:
    void connect(RpmController *controller);

signals:
    void requestFilesForPackages(const QModelIndexList &indexes);
    void requestInfoForPackages(const QModelIndexList &indexes);
    void requestRemovePackages(const QModelIndexList &indexes);

private:
    void setButtonsEnabled(bool isEnable) override;
    void retranslateChildWidgets();

private slots:
    void changeEvent(QEvent *event) override;
    void onSectionCountChanged(int oldCount, int newCount);
    void onRemovePackagesRequested();
    void onInfoAboutPackagesRequested();
    void onFilesOfPackagesRequested();
    void onPackagesListChanged();
    void onFilterGroupChanged(const QString &text);
    void onFilterArchChanged(const QString &text);

private:
    RpmManagerWidget(const RpmManagerWidget &)            = delete;
    RpmManagerWidget(RpmManagerWidget &&)                 = delete;
    RpmManagerWidget &operator=(const RpmManagerWidget &) = delete;
    RpmManagerWidget &operator=(RpmManagerWidget &&)      = delete;

private:
    RpmController *m_controller;
    std::unique_ptr<QPushButton> m_installPushButton;
    std::unique_ptr<QPushButton> m_removePushButton;
    std::unique_ptr<QPushButton> m_filesPushButton;
    std::unique_ptr<QPushButton> m_infoPushButton;

    std::unique_ptr<FilterWidget> m_groupFilterWidget;
    std::unique_ptr<FilterWidget> m_archFilterWidget;
};

#endif // RPMMANAGERWIDGET_H
