#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "Controller.h"
#include "ControlForm.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setController(Controller* controller)
{
    connect(controller, &Controller::beginRefresh, this, [this]{
        ui->progressBar->show();
        ui->centralwidget->setEnabled(false);
        setCursor(QCursor(Qt::WaitCursor));
    });

    connect(controller, &Controller::endRefresh, this, [this]{
        ui->progressBar->hide();
        ui->centralwidget->setEnabled(true);
        unsetCursor();
    });

    auto model = controller->facilitiesModel();
    m_filter_model.setSourceModel(model);
    ui->listView->setModel(&m_filter_model);

    connect(model, &QAbstractItemModel::modelReset, this, [this]{
        ui->stackedWidget->setCurrentIndex(0);
        for ( int i = 1; i < ui->stackedWidget->count(); ++i )
            ui->stackedWidget->removeWidget(ui->stackedWidget->widget(i));
    });

    connect(model, &QAbstractItemModel::rowsInserted, this,
    [=](QModelIndex, int first, int last){
        for ( int i = first; i <= last; ++i ) {
            ui->stackedWidget->insertWidget(i+1, new ControlForm{
                controller,
                model->index(i,0).data().toString(),
                model->index(i,1).data().toString(),
                model->index(i,2).data().toString(),
                model->index(i,2).data(Controller::ValueListRole).toMap(),
                this
            });
        }
    });
}


void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    ui->stackedWidget->setCurrentIndex( m_filter_model.mapToSource(index).row() +1);
}


void MainWindow::on_searchBar_textChanged(const QString &arg1)
{
    m_filter_model.setFilterFixedString(arg1);
}

