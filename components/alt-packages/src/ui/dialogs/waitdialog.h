#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>

namespace Ui
{
class WaitDialog;
}

class WaitDialog : public QDialog
{
    Q_OBJECT

public:
    WaitDialog(QWidget *parent = nullptr);
    ~WaitDialog();

public:
    void appendText(QString text);
    void clearText();
    bool isEmpty();

public slots:
    void setProgressBarVisible(bool visible);

private:
    WaitDialog(const WaitDialog &)            = delete;
    WaitDialog(WaitDialog &&)                 = delete;
    WaitDialog &operator=(const WaitDialog &) = delete;
    WaitDialog &operator=(WaitDialog &&)      = delete;

private:
    std::unique_ptr<Ui::WaitDialog> m_ui;
};

#endif // WAITDIALOG_H
