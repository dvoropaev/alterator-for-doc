#ifndef TRANSACTION_TAB_WIDGET_H
#define TRANSACTION_TAB_WIDGET_H

#include "service/transaction.h"
#include <QWidget>

namespace alt::Ui
{
class TransactionTabWidget;
} // namespace alt::Ui

namespace alt
{
class TransactionTabWidget : public QWidget
{
    Q_OBJECT

public:
    enum Stage
    {
        Request,
        Resolve,
    };

public:
    explicit TransactionTabWidget(QWidget *parent = nullptr);
    ~TransactionTabWidget() override;

public:
    void setTransaction(const Transaction &transaction);
    void setStage(Stage stage);
    void clear();

private:
    void setComponentList(const Transaction &transaction);
    void setPackageList(const Transaction &transaction);

private:
    std::unique_ptr<Ui::TransactionTabWidget> ui;
    Stage stage;
};
} // namespace alt
#endif // TRANSACTION_TAB_WIDGET_H
