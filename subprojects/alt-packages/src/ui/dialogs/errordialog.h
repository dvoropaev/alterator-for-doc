#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ErrorDialog(const ErrorDialog &)            = delete;
    ErrorDialog(ErrorDialog &&)                 = delete;
    ErrorDialog &operator=(const ErrorDialog &) = delete;
    ErrorDialog &operator=(ErrorDialog &&)      = delete;
    ~ErrorDialog();

    void showError(int code, const QStringList& log);

private:
    Ui::ErrorDialog *ui;
};

#endif // ERRORDIALOG_H
