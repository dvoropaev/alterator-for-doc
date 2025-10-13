#ifndef RPMFILESDIALOG_H
#define RPMFILESDIALOG_H

#include <QDialog>
#include <QStringListModel>

namespace Ui
{
class RpmFilesDialog;
}

class RpmFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RpmFilesDialog(QWidget *parent = nullptr);
    ~RpmFilesDialog();

public:
    void setModel(QStringListModel *model);

private:
    RpmFilesDialog(const RpmFilesDialog &)            = delete;
    RpmFilesDialog(RpmFilesDialog &&)                 = delete;
    RpmFilesDialog &operator=(const RpmFilesDialog &) = delete;
    RpmFilesDialog &operator=(RpmFilesDialog &&)      = delete;

private:
    std::unique_ptr<Ui::RpmFilesDialog> m_ui;
};

#endif // RPMFILESDIALOG_H
