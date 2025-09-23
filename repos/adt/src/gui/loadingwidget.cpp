/***********************************************************************************************************************
**
** Copyright (C) 2024 BaseALT Ltd. <org@basealt.ru>
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

#include "loadingwidget.h"

#include <QPainter>

LoadingWidget::LoadingWidget(QWidget* parent)
    : QWidget(parent)
    , m_angle(0)
    , m_timerId(-1)
    , m_delay(60)
    , m_color(Qt::black)
{
    QColor themeOS = this->palette().window().color();
    if (themeOS.red() <= 100 || themeOS.green() <= 100 || themeOS.blue() <= 100)
        m_color = Qt::white;

    startAnimation();
}

void LoadingWidget::startAnimation()
{
    m_angle = 0;

    if (m_timerId == -1)
        m_timerId = startTimer(m_delay);
}

void LoadingWidget::timerEvent(QTimerEvent* event)
{
    m_angle = (m_angle + 30) % 360;

    update();
}

void LoadingWidget::paintEvent(QPaintEvent* event)
{
    int width = qMin(this->width(), this->height());

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int outerRadius = (width - 1) * 0.5;
    int innerRadius = (width - 1) * 0.5 * 0.38;

    int capsuleHeight = outerRadius - innerRadius;
    int capsuleWidth = (width > 32) ? capsuleHeight * .23 : capsuleHeight * .35;
    int capsuleRadius = capsuleWidth / 2;

    for (int i = 0; i < 12; i++) {
        QColor color = m_color;
        color.setAlphaF(1.0f - (i / 12.0f));
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.save();
        p.translate(rect().center());
        p.rotate(m_angle - i * 30.0f);
        p.drawRoundedRect(-capsuleWidth * 0.5, -(innerRadius + capsuleHeight), capsuleWidth,
                          capsuleHeight, capsuleRadius, capsuleRadius);
        p.restore();
    }
}