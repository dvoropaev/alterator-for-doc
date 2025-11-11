#include "ServiceWidget.h"
#include "ui_ServiceWidget.h"

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
    Private(Service* s, QWidget* parent)
        : m_service{s}
        , m_resource_model{s->resources()}
    {
        m_resource_model.setScope(Parameter::ValueScope::Default);

        std::vector<Parameter*> status_filtered;
        for ( const auto& parameter : s->parameters() )
            if ( !parameter->isConstant() && parameter->contexts().testFlag(Parameter::Context::Status) )
                status_filtered.push_back(parameter.get());
        m_parameter_model.setItems(status_filtered);
    }

    Ui::ServiceWidget ui;
    Service* m_service{};
    ParameterModel m_parameter_model;
    ResourceModel m_resource_model;
};

ServiceWidget::ServiceWidget(Service* s, QWidget *parent)
    : QWidget(parent)
    , d{ new Private{s, parent} }
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

    d->ui.parametersView->setModel( &d->m_parameter_model );
    d->ui.parametersView->header()->resizeSection(0, 300);

    d->ui.resourcesView->setModel( &d->m_resource_model );
    for ( int r = 0; r < d->m_resource_model.rowCount(); ++r )
        d->ui.resourcesView->setFirstColumnSpanned(r, {}, true);
    d->ui.resourcesView->expandAll();
    d->ui.resourcesView->header()->resizeSection(0, 300);
    d->ui.resourcesView->setColumnNeverDetailed(1, true);

    connect(d->ui.resourcesView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* parameter = data.value<Parameter*>() ) {
            d->ui.tabWidget->setCurrentIndex(0);
            int row = d->m_parameter_model.indexOf(parameter);
            d->ui.parametersView->highlight(d->m_parameter_model.index(row, 0));
        }
    });

    connect(d->ui.parametersView, &QAbstractItemView::clicked, this, [this](const QModelIndex& i){
        auto data = i.data(Qt::UserRole);
        if ( data.isNull() ) return;
        if ( auto* resource = data.value<Resource*>() ) {
            d->ui.tabWidget->setCurrentIndex(1);
            d->ui.resourcesView->highlight(d->m_resource_model.indexOf(resource));
        }
    });

    onStatusChanged();
}

ServiceWidget::~ServiceWidget() { delete d; }

void ServiceWidget::on_comboBox_activated(int index)
{
    d->m_parameter_model.setScope((Parameter::ValueScope)index);
}

void ServiceWidget::onStatusChanged()
{
    d->m_parameter_model.refresh();

    d->ui. deployedCheckBox -> setChecked( d->m_service->isDeployed() );
    d->ui.  startedCheckBox -> setVisible( d->m_service->isDeployed() );
    d->ui.  startedCheckBox -> setChecked( d->m_service->isStarted()  );


    on_comboBox_activated(d->ui.comboBox->currentIndex());
}

