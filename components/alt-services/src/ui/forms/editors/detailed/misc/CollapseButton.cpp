#include "CollapseButton.h"

CollapseButton::CollapseButton(QWidget *parent) :
    QPushButton(parent)
{
    static const QIcon expanded  = QIcon::fromTheme("pan-down", QIcon::fromTheme("go-down"));
    static const QIcon collapsed = QIcon::fromTheme("pan-end",  QIcon::fromTheme("go-next"));

    setIcon(collapsed);
    setFlat(true);
    setCheckable(true);
    setText(tr("Expand"));
    connect( this, &QAbstractButton::toggled, this, [this](bool checked){
        setIcon(checked ? expanded : collapsed);
        setText(checked ? tr("Collapse") : tr("Expand"));
    });
}
