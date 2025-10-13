#include "dialog.h"

#include "ui_dialog.h"

namespace alt
{
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::Dialog>())
{
    m_ui->setupUi(this);
}

Dialog::~Dialog() = default;
} // namespace alt
