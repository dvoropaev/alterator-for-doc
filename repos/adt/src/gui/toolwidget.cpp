#include "toolwidget.h"
#include "ui_toolwidget.h"

#include <model/tooltabviewdelegate.h>

#define TEST_ICON_X_SIZE 16
#define TEST_ICON_Y_SIZE 16
#define YES_ICON "emblem-default"
#define NO_ICON "dialog-error"
#define UNDEFINED_ICON "dialog-question"

ToolWidget::ToolWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToolWidget)
{
    ui->setupUi(this);
    QIcon questionIcon = QIcon::fromTheme(UNDEFINED_ICON);
}

ToolWidget::~ToolWidget()
{
    delete ui;
}

void ToolWidget::setTestAvailability(bool systemTests, bool sessionTests)
{
    QIcon yesIcon = QIcon::fromTheme(YES_ICON);
    QIcon noIcon  = QIcon::fromTheme(NO_ICON);

    if (systemTests)
        ui->systemTestsCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->systemTestsCheckBox->setCheckState(Qt::CheckState::Unchecked);

    if (sessionTests)
        ui->sessionTestsCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->sessionTestsCheckBox->setCheckState(Qt::CheckState::Unchecked);
}

void ToolWidget::setVarsModel(ADTTool *tool, std::unique_ptr<QStandardItemModel> model)
{
    ui->varsTableView->setModel(model.release());

    ui->varsTableView->setColumnHidden(1, true);
    ui->varsTableView->setColumnHidden(3, true);
    ui->varsTableView->setColumnHidden(4, true);
    ui->varsTableView->setColumnHidden(5, true);

    ToolTabViewDelegate *delegate = new ToolTabViewDelegate(tool);
    ui->varsTableView->setItemDelegate(delegate);

    for (int i = 0; i < ui->varsTableView->model()->rowCount(); ++i)
        ui->varsTableView->openPersistentEditor(ui->varsTableView->model()->index(i, 2));

    ui->varsTableView->updateColumnsSizes();
}

void ToolWidget::setDescription(const QString &description)
{
    ui->descriptionTextEdit->setText(description);
}
