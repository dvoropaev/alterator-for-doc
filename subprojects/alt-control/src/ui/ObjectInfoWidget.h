#pragma once

#include <QWidget>
#include "data/TranslatableObject.h"

class ObjectInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectInfoWidget(QWidget *parent = nullptr);
    ~ObjectInfoWidget();
    void setObject(TranslatableObject* obj);

signals:
    void clicked();

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    class Private;
    Private* d{};
};

