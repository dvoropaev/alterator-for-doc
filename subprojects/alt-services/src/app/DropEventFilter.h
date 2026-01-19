#pragma once

#include <QObject>

/*
 * Allows .json file drag'n'drop
 */
class DropEventFilter : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    bool eventFilter(QObject* watched, QEvent* event) override;
};
Q_GLOBAL_STATIC(DropEventFilter, dropEventFilter)
