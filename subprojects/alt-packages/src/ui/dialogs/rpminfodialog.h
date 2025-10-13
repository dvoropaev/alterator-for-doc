#ifndef RPMINFODIALOG_H
#define RPMINFODIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
class RpmInfoDialog;
}

class RpmInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RpmInfoDialog(QWidget *parent = nullptr);
    ~RpmInfoDialog();

public:
    void setModel(QStandardItemModel *model);

private:
    RpmInfoDialog(const RpmInfoDialog &)            = delete;
    RpmInfoDialog(RpmInfoDialog &&)                 = delete;
    RpmInfoDialog &operator=(const RpmInfoDialog &) = delete;
    RpmInfoDialog &operator=(RpmInfoDialog &&)      = delete;

private:
    std::unique_ptr<Ui::RpmInfoDialog> m_ui;
};

#endif // RPMINFODIALOG_H
