#ifndef APPLYSTATUSBAR_H
#define APPLYSTATUSBAR_H

#include "statusbar.h"
#include <QFrame>
#include <QLabel>

namespace alt
{
class ApplyStatusBar : public StatusBar
{
    Q_OBJECT
public:
    explicit ApplyStatusBar(QWidget *parent = nullptr);
    ~ApplyStatusBar() override;

public:
    ApplyStatusBar(const ApplyStatusBar &) = delete;
    ApplyStatusBar(ApplyStatusBar &&) = delete;
    ApplyStatusBar &operator=(const ApplyStatusBar &) = delete;
    ApplyStatusBar &operator=(ApplyStatusBar &&) = delete;

public:
    void setError(bool value);
    void setWarning(bool value);

private:
    std::unique_ptr<QLabel> iconLabel;
};

} // namespace alt

#endif // APPLYSTATUSBAR_H
