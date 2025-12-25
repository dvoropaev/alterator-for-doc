#pragma once

#include <QObject>

class RichToolTipEventFilter : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    bool eventFilter(QObject* watched, QEvent* event) override;
};
