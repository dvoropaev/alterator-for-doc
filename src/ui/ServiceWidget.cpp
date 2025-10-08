#include "ServiceWidget.h"
#include "ui_ServiceWidget.h"

#include "controller/Controller.h"

#include "ObjectInfoDelegate.h"

#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include <QMessageBox>
#include <QDebug>
#include <QTimer>

#include <QJsonObject>
#include <QJsonDocument>

#include <QPainter>

#include "data/models/ParameterModel.h"
#include "data/models/ResourceModel.h"

class ServiceWidget::Private {
public:
    Private(Controller* c, Service* s, QWidget* parent)
        : m_controller{c}
        , m_service{s}
    {}

    Ui::ServiceWidget ui;
    Controller* m_controller;
    Service* m_service;
};

ServiceWidget::ServiceWidget(Controller* c, Service* s, QWidget *parent)
    : QWidget(parent)
    , d{ new Private{c,s, parent} }
{
    d->ui.setupUi(this);


    {
        d->ui. tabWidget->setTabIcon(0, QIcon::fromTheme("dialog-information"));
        d->ui. tabWidget->setTabIcon(1, QIcon::fromTheme("changes-prevent"));
        d->ui. tabWidget->setTabIcon(2, QIcon::fromTheme("system-run"));
    }


    { // make disabled checkboxes use same font color as enabled ones
        auto palette = d->ui.deployedCheckBox->palette();
        palette.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::WindowText,
           palette.color(QPalette::ColorGroup::Active,   QPalette::ColorRole::WindowText)
        );
        d->ui.deployedCheckBox->setPalette(palette);
        d->ui. startedCheckBox->setPalette(palette);
    }

    d->ui.objectInfoWidget->setObject(d->m_service);

    d->ui.parametersView->setModel( d->m_service->parameterModel() );
    d->ui.parametersView->header()->resizeSection(0, 300);

    d->ui.resourcesView->setModel( d->m_service->resourceModel() );
    for ( int r = 0; r < d->m_service->resourceModel()->rowCount(); ++r )
        d->ui.resourcesView->setFirstColumnSpanned(r, {}, true);
    d->ui.resourcesView->expandAll();
    d->ui.resourcesView->header()->resizeSection(0, 300);
    d->ui.resourcesView->setItemDelegateForColumn(1, new QStyledItemDelegate{this});

    connect(d->ui.resourcesView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* parameter = data.value<Parameter*>() ) {
            d->ui.tabWidget->setCurrentIndex(0);
            int row = d->m_service->parameterModel()->indexOf(parameter);
            d->ui.parametersView->highlight(d->m_service->parameterModel()->index(row, 0));
        }
    });

    connect(d->ui.parametersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* resource = data.value<Resource*>() ) {
            d->ui.tabWidget->setCurrentIndex(1);
            d->ui.resourcesView->highlight(d->m_service->resourceModel()->indexOf(resource));
        }
    });

    updateStatus();
}

ServiceWidget::~ServiceWidget() { delete d; }

void ServiceWidget::on_comboBox_activated(int index)
{
    d->m_service->showDefault(index == 0);
}


void ServiceWidget::updateStatus()
{
    QByteArray data;
    auto status = d->m_controller->status(d->m_service, data);

    if ( status < 0 ) {
        QMessageBox::critical(this, tr("Error requesting status"), data);
        return;
    }

    d->m_service->setStatus(status, data);

    d->ui. deployedCheckBox -> setChecked( d->m_service->isDeployed() );

    d->ui.  startedCheckBox -> setVisible( d->m_service->isDeployed() );
    d->ui.  startedCheckBox -> setChecked( d->m_service->isStarted()  );


    on_comboBox_activated(d->ui.comboBox->currentIndex());
}

