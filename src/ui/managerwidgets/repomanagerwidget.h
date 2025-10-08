#ifndef REPOCONTROLLERWIDGET_H
#define REPOCONTROLLERWIDGET_H

#include "basemanagerwidget.h"
#include "controllers/repocontroller.h"

#include <QPushButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class RepoManagerWidget : public BaseManagerWidget
{
    Q_OBJECT

public:
    explicit RepoManagerWidget(QWidget *parent = nullptr);
    ~RepoManagerWidget();

public:
    void connect(RepoController *controller);

signals:
    void requestRemoveRepos(const QModelIndexList &indexes);

private:
    void setButtonsEnabled(bool isEnable) override;
    void retranslateButtons();

private slots:
    void changeEvent(QEvent *event) override;
    void onSectionCountChanged(int oldCount, int newCount);
    void onRemoveRepoRequested();

private:
    RepoManagerWidget(const RepoManagerWidget &)            = delete;
    RepoManagerWidget(RepoManagerWidget &&)                 = delete;
    RepoManagerWidget &operator=(const RepoManagerWidget &) = delete;
    RepoManagerWidget &operator=(RepoManagerWidget &&)      = delete;

private:
    std::unique_ptr<QPushButton> m_addPushButton;
    std::unique_ptr<QPushButton> m_removePushButton;
    std::unique_ptr<QShortcut> m_addShortcut;
    std::unique_ptr<QShortcut> m_rmShortcut;
};

#endif // REPOCONTROLLERWIDGET_H
