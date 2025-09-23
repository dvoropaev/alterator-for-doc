/***********************************************************************************************************************
**
** Copyright (C) 2023 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "statuscommonwidget.h"
#include "ui_statuscommonwidget.h"

#include <QStyle>

StatusCommonWidget::StatusCommonWidget(TestWidgetInterface *widget, QString test, QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::StatusCommonWidget)
    , m_widget(widget)
    , m_test(test)
    , m_labelText()
    , m_backColorWidget()
    , m_mutex()
    , m_status(WidgetStatus::idle)
{
    m_ui->setupUi(this);

    QIcon icon = QIcon::fromTheme("preferences-other");

    m_labelText = m_widget->getDisplayName(m_test);

    m_ui->testIconLabel->setPixmap(icon.pixmap(m_ui->runPushButton->iconSize()));
    m_ui->testNameLabel->setText(m_widget->getDisplayName(m_test));
}

StatusCommonWidget::~StatusCommonWidget()
{
    delete m_ui;
}

QString StatusCommonWidget::getToolId()
{
    return m_widget->getTool();
}

QString StatusCommonWidget::getTestId()
{
    return m_test;
}

void StatusCommonWidget::setText(QString text)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_labelText = text;
    m_ui->testNameLabel->setText(text);
}

void StatusCommonWidget::setIcon(QIcon &icon)
{
    m_ui->testIconLabel->setPixmap(icon.pixmap(m_ui->runPushButton->iconSize()));
}

void StatusCommonWidget::setEnabledRunButton(bool isEnabled)
{
    m_ui->runPushButton->setEnabled(isEnabled);
}

void StatusCommonWidget::setWidgetStatus(WidgetStatus status)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    QIcon icon;
    QString text;
    switch (status)
    {
    case WidgetStatus::idle:
        m_status          = status;
        icon              = QIcon::fromTheme("preferences-other");
        text              = m_widget->getDisplayName(m_test);
        m_backColorWidget = TestIdleColor();
        break;
    case WidgetStatus::ready:
        m_status          = status;
        icon              = QIcon::fromTheme("preferences-other");
        text              = m_widget->getDisplayName(m_test);
        m_backColorWidget = TestReadyColor();
        break;
    case WidgetStatus::running:
        m_status          = status;
        icon              = style()->standardIcon(QStyle::SP_BrowserReload);
        text              = QString(tr("Running:")) + QString(" ") + m_widget->getDisplayName(m_test);
        m_backColorWidget = TestRunningColor();
        break;
    case WidgetStatus::finishedOk:
        m_status = status;
        icon     = style()->standardIcon(QStyle::SP_DialogApplyButton);
        text     = m_widget->getDisplayName(m_test);
        ;
        m_backColorWidget = TestOkColor();
        break;
    case WidgetStatus::finishedFailed:
        m_status          = status;
        icon              = style()->standardIcon(QStyle::SP_DialogCloseButton);
        text              = m_widget->getDisplayName(m_test);
        m_backColorWidget = TestFailedColor();
        break;
    }
    setColorWidget(0);
    setIcon(icon);
    setText(text.trimmed());

    setNameTestLength(this->width());
}

WidgetStatus StatusCommonWidget::getWidgetStatus()
{
    return m_status;
}

QString StatusCommonWidget::getLabelText()
{
    return m_labelText;
}

void StatusCommonWidget::setColorWidget(int deltaColor)
{
    QPalette pal = QPalette();
    QColor color(m_backColorWidget.red,
                 m_backColorWidget.green,
                 m_backColorWidget.blue,
                 m_backColorWidget.alpha + deltaColor);
    pal.setColor(QPalette::Window, color);
    setPalette(pal);
    setAutoFillBackground(true);
}

void StatusCommonWidget::setNameTestLength(int size, int verticalScrollBarWidth)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    QSize labelSize = m_ui->testNameLabel->size();

    int leftMargin = 0, rightMargin = 0, topMargin = 0, bottomMargin = 0;
    m_ui->gridLayout->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    int widgetsWidth = m_ui->runPushButton->width() + m_ui->logsPushButton->width() + m_ui->testIconLabel->width();
    int labelWidth   = size - (widgetsWidth + leftMargin + rightMargin + verticalScrollBarWidth + 30);

    m_ui->testNameLabel->resize(labelWidth, labelSize.height());

    QFontMetrics metrics(m_ui->testNameLabel->font());
    m_ui->testNameLabel->setText(metrics.elidedText(m_labelText, Qt::ElideRight, labelWidth));
}

void StatusCommonWidget::showEvent(QShowEvent *event)
{
    setNameTestLength(this->width());
}

void StatusCommonWidget::on_runPushButton_clicked()
{
    emit runButtonCLicked(this);
}

void StatusCommonWidget::on_logsPushButton_clicked()
{
    emit logsButtonClicked(this);
}

void StatusCommonWidget::enterEvent(QEnterEvent *event)
{
    setColorWidget(50);
}

void StatusCommonWidget::leaveEvent(QEvent *event)
{
    setColorWidget(0);
}

void StatusCommonWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    on_runPushButton_clicked();
}
