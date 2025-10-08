#ifndef STOPDIALOG_H
#define STOPDIALOG_H

#include "loadingwidget.h"

#include <QDialog>

namespace Ui
{
class StopDialog;
}

class StopDialog : public QDialog
{
    Q_OBJECT

public:
    StopDialog(QWidget *parent = nullptr);
    ~StopDialog();

    QSize getSize();

private:
    Ui::StopDialog *ui{};

private:
    StopDialog(const StopDialog &)            = delete;
    StopDialog(StopDialog &&)                 = delete;
    StopDialog &operator=(const StopDialog &) = delete;
    StopDialog &operator=(StopDialog &&)      = delete;
};

#endif // STOPDIALOG_H
