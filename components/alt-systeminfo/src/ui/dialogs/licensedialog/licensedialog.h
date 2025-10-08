#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>
#include <QShortcut>

#include <memory>

namespace alt
{
namespace Ui
{
class LicenseDialog;
} // namespace Ui

class LicenseDialog : public QDialog
{
    Q_OBJECT

public:
    LicenseDialog(QWidget *parent = nullptr);
    ~LicenseDialog();

public:
    void setLicenseText(const QString &text);

private:
    LicenseDialog(const LicenseDialog &) = delete;
    LicenseDialog(LicenseDialog &&) = delete;
    LicenseDialog &operator=(const LicenseDialog &) = delete;
    LicenseDialog &operator=(LicenseDialog &&) = delete;

private:
    std::unique_ptr<Ui::LicenseDialog> m_ui;
    QShortcut m_quitShortcut;
};
} // namespace alt
#endif // LICENSEDIALOG_H
