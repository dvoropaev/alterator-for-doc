#pragma once

#include <QRadioButton>

#include "../../Editor.h"

namespace Ui { class EditorTitle; }

class EditorTitle : public QWidget
{
    Q_OBJECT

public:
    explicit EditorTitle(QWidget *parent = nullptr);
    ~EditorTitle();
    void setEditor(Editor* e);
    QRadioButton* radio();

private slots:
    void onChecked(bool);

private:
    Ui::EditorTitle *ui;
    Editor* m_editor{nullptr};

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent* event) override;
};

