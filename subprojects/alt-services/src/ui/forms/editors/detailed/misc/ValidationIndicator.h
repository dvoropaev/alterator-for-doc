#pragma once

#include <QLabel>

class Editor;

namespace Ui { class ValidationIndicator; }

class ValidationIndicator : public QWidget
{
    Q_OBJECT
public:
    ValidationIndicator(QWidget* parent);
    ~ValidationIndicator();

    void setEditor(Editor* e);

private slots:
    void validate();

private:
    Editor* m_editor{nullptr};
    Ui::ValidationIndicator* ui;
};
