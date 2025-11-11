#pragma once

#include <KMessageWidget>

class Editor;

class ValidationIndicator : public KMessageWidget
{
    Q_OBJECT
public:
    ValidationIndicator(QWidget* parent = nullptr);

    void setEditor(Editor* e);

private slots:
    void validate();

private:
    Editor* m_editor{nullptr};
};
