#ifndef APTMANAGERWIDGET_H
#define APTMANAGERWIDGET_H

#include "basemanagerwidget.h"
#include "controllers/aptcontroller.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class AptManagerWidget : public BaseManagerWidget
{
    Q_OBJECT

public:
    explicit AptManagerWidget(QWidget *parent = nullptr);
    ~AptManagerWidget();

public:
    void connect(AptController *controller);

    void setProgress(int status);
    void hideProgress();

signals:
    void requestApplyChanges(const QModelIndexList &indexes);

private:
    void setButtonsEnabled(bool isEnable) override;
    void retranslateButtons();

private slots:
    void changeEvent(QEvent *event) override;
    void onItemClicked(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onApplyButtonClicked();
    void onResetButtonClicked();

private:
    AptManagerWidget(const AptManagerWidget &)            = delete;
    AptManagerWidget(AptManagerWidget &&)                 = delete;
    AptManagerWidget &operator=(const AptManagerWidget &) = delete;
    AptManagerWidget &operator=(AptManagerWidget &&)      = delete;

private:
    std::unique_ptr<QPushButton> m_applyPushButton;
    std::unique_ptr<QPushButton> m_resetPushButton;
    std::unique_ptr<QPushButton> m_updatePushButton;
    std::unique_ptr<QPushButton> m_distUpgradePushButton;
};

#endif // APTMANAGERWIDGET_H
