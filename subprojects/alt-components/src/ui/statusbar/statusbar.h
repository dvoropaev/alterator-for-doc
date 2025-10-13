#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>

namespace alt::Ui
{
class StatusBar;
} // namespace alt::Ui

namespace alt
{
class StatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget *parent = nullptr);
    ~StatusBar() override;

public:
    StatusBar(const StatusBar &) = delete;
    StatusBar(StatusBar &&) = delete;
    StatusBar &operator=(const StatusBar &) = delete;
    StatusBar &operator=(StatusBar &&) = delete;

public:
    void setProgressBarMaximum(int maximum);
    void incrementProgressBarValue();
    void resetProgressBarValue();
    void onStarted(const QString &text);
    void onProgress(const QString &text);
    void onDone(const QString &text);

protected:
    Ui::StatusBar *ui;
};

} // namespace alt

#endif // STATUSBAR_H
