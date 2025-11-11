#pragma once

#include <QDialog>

namespace Ui {
class AboutDialog;
}


class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    AboutDialog(const AboutDialog &) = delete;
    AboutDialog(AboutDialog &&) = delete;
    AboutDialog &operator=(const AboutDialog &) = delete;
    AboutDialog &operator=(AboutDialog &&) = delete;

private:
    Ui::AboutDialog *ui;
};
