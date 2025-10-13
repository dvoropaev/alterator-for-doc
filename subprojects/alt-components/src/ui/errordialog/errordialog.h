#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>

namespace alt::Ui
{
class ErrorDialog;
}

namespace alt
{
class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();

public slots:
    void showError(int code, const QString &log);

private:
    ErrorDialog(const ErrorDialog &) = delete;
    ErrorDialog(ErrorDialog &&) = delete;
    ErrorDialog &operator=(const ErrorDialog &) = delete;
    ErrorDialog &operator=(ErrorDialog &&) = delete;

private:
    Ui::ErrorDialog *ui;
};
} // namespace alt
#endif // ERRORDIALOG_H
