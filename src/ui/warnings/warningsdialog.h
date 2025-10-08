#ifndef WARNINGSDIALOG_H
#define WARNINGSDIALOG_H

#include <QDialog>
#include <QItemSelectionModel>
#include <QStandardItemModel>

namespace alt::Ui
{
class WarningsDialog;
} // namespace alt::Ui

namespace alt
{
class WarningsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarningsDialog(QStandardItemModel *model, QWidget *parent = nullptr);
    ~WarningsDialog();

signals:
    void messagesRemoved(QItemSelectionModel *selection);

private:
    Ui::WarningsDialog *ui;
};
} // namespace alt

#endif // WARNINGSDIALOG_H
