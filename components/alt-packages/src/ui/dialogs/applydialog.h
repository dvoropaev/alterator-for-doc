#ifndef APPLYDIALOG_H
#define APPLYDIALOG_H

#include <set>
#include <QDialog>
#include <QLabel>
#include <QListWidget>

namespace Ui
{
class ApplyDialog;
} // namespace Ui

class ApplyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyDialog(const QStringList &installPackages,
                         const QStringList &removePackages,
                         const QStringList &extraRemovePackages,
                         bool safeMode,
                         QWidget *parent = nullptr);
    ~ApplyDialog() override;

private:
    ApplyDialog(const ApplyDialog &)            = delete;
    ApplyDialog(ApplyDialog &&)                 = delete;
    ApplyDialog &operator=(const ApplyDialog &) = delete;
    ApplyDialog &operator=(ApplyDialog &&)      = delete;

private:
    void initPackageViews(const QStringList &installPackages,
                          const QStringList &removePackages,
                          const QStringList &extraRemovePackages);

private:
    Ui::ApplyDialog *m_ui;
    bool safeMode;
};

#endif // APPLYDIALOG_H
