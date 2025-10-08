#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class AboutDialog;
} // namespace Ui
QT_END_NAMESPACE

class AboutDialogPrivate;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    void setVersion(const QString &version);

public:
    AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    AboutDialog(const AboutDialog &)            = delete;
    AboutDialog(AboutDialog &&)                 = delete;
    AboutDialog &operator=(const AboutDialog &) = delete;
    AboutDialog &operator=(AboutDialog &&)      = delete;

private:
    std::unique_ptr<Ui::AboutDialog> m_ui;
};

#endif // ABOUTDIALOG_H
