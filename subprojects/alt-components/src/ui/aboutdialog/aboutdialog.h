#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>

namespace alt::Ui
{
class AboutDialog;
} // namespace alt::Ui

namespace alt
{
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent);

    ~AboutDialog();

private:
    AboutDialog(const AboutDialog &) = delete;
    AboutDialog(AboutDialog &&) = delete;
    AboutDialog &operator=(const AboutDialog &) = delete;
    AboutDialog &operator=(AboutDialog &&) = delete;

private:
    Ui::AboutDialog *ui;
};

} // namespace alt

#endif // ABOUT_DIALOG_H
