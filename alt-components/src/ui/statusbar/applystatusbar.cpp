#include "applystatusbar.h"

#include "ui_statusbar.h"

#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QStyle>

namespace alt
{
ApplyStatusBar::ApplyStatusBar(QWidget *parent)
    : StatusBar(parent)
    , iconLabel(std::make_unique<QLabel>())
{
    ui->horizontalLayout->insertWidget(0, iconLabel.get());
    setWarning(false);
}

ApplyStatusBar::~ApplyStatusBar() = default;

void ApplyStatusBar::setWarning(bool value)
{
    QIcon icon = QIcon::fromTheme("dialog-warning-symbolic", QIcon(":warning"));
    iconLabel->setPixmap(icon.pixmap(24, 24).scaled(24, 24));
    iconLabel->setVisible(value);
}

void ApplyStatusBar::setError(bool value)
{
    QIcon icon = QIcon::fromTheme("dialog-error-symbolic", QIcon(":error"));
    iconLabel->setPixmap(icon.pixmap(24, 24).scaled(24, 24));
    iconLabel->setVisible(value);
}
} // namespace alt
