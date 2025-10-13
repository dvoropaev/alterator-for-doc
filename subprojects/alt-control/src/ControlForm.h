#pragma once

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class ControlForm;
}

class Controller;
class ControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit ControlForm(Controller* controller, const QString& name, const QString& summary, const QString& value,
                         const QVariantMap& values, QWidget *parent = nullptr);
    ~ControlForm();

private slots:
    void onButtonClicked(QAbstractButton*);

private:
    Ui::ControlForm *ui;
    QMap<QAbstractButton*, QString> m_values;
    QButtonGroup m_group;
    Controller* m_controller;
    QAbstractButton* current{nullptr};
};

