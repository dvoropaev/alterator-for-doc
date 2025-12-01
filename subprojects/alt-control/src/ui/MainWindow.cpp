#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "controller/Controller.h"
#include "app/ControlApp.h"

#include <QActionGroup>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>

#include "AboutDialog.h"
#include "ObjectInfoDelegate.h"
#include "StateForm.h"

#include <QSortFilterProxyModel>
class FilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public slots:
    void setSecondColumnVisible(bool how) {
        beginFilterChange();
        m_show_second_column = how;
        invalidateColumnsFilter();
    }

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override {
        return source_column != 1 || m_show_second_column;
    }

private:
    bool m_show_second_column{false};
};
#include "MainWindow.moc"


class MainWindow::Private {
public:
    Ui::MainWindow ui;
    AboutDialog m_about;
    FilterProxyModel m_filter_model{};
    FacilityModel* m_model{};
    Controller* m_controller{};

    QButtonGroup m_group{};
    std::map<QRadioButton*, StateForm*> m_states;
    Facility* m_facility{};
    bool m_pending{false};
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d{new Private{}}
{
    d->ui.setupUi(this);

    d->ui.searchBar->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), QLineEdit::LeadingPosition)
        ->setDisabled(true);

    {
        d->ui.menuView->insertSeparator(d->ui.actionCompact)->setText(tr("Table style"));
        auto group = new QActionGroup{this};
        group->addAction(d->ui.actionCompact);
        group->addAction(d->ui.actionVerbose);
        group->setExclusive(true);

        ( qApp->settings().tableDetailed()
            ? d->ui.actionVerbose
            : d->ui.actionCompact ) -> setChecked(true);

        connect(d->ui.actionVerbose, &QAction::toggled, &qApp->settings(), &AppSettings::set_tableDetailed);

        /*
         * NOTE: every control1 backend may have different policies,
         * so for this feature to work, we'll need somehow determine if
         * password is not required to get current facility's state;
         * For the same reason current batch backend does not provide
         * a corresponding method, so some kind of scroll-driven
         * lazy loading needs to be implemented.
         */
        d->ui.actionDisplay_current_state_in_second_column->setChecked(false /*qApp->settings().tableShowCurrentState()*/);
        //connect(d->ui.actionDisplay_current_state_in_second_column, &QAction::toggled, &qApp->settings(), &AppSettings::set_tableShowCurrentState);
        //connect(d->ui.actionDisplay_current_state_in_second_column, &QAction::toggled, &d->m_filter_model, &FilterProxyModel::setSecondColumnVisible);

        connect(d->ui.actionAbout, &QAction::triggered, &d->m_about, &QDialog::open);
    }

    connect(d->ui.searchBar, &QLineEdit::textChanged, &d->m_filter_model, &FilterProxyModel::setFilterFixedString);

    d->ui.facilityView->setUniformRowHeights(false);
    d->ui.facilityView->setItemDelegateForColumn(0, new ObjectInfoDelegate{this});
    d->ui.facilityView->setItemDelegateForColumn(1, new ObjectInfoDelegate{this});
    d->ui.facilityView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    connect(d->ui.facilityView->header(), &QHeaderView::sectionResized, [this](int index, int oldSize, int newSize){
        if ( auto delegate = qobject_cast<ObjectInfoDelegate*>(d->ui.facilityView->itemDelegateForColumn(index)) )
            delegate->setColumnSize(newSize);
    });

    d->ui.progressBar->hide();
    d->ui.facilityView->setModel(&d->m_filter_model);
}

MainWindow::~MainWindow() { delete d; }

void MainWindow::setController(Controller* controller)
{
    d->m_controller = controller;

    connect(controller, &Controller::beginRefresh, this, [this]{
        d->ui.progressBar->show();
        d->ui.centralwidget->setEnabled(false);
        setCursor(QCursor(Qt::WaitCursor));
    });

    connect(controller, &Controller::endRefresh, this, [this]{
        d->ui.progressBar->hide();
        d->ui.centralwidget->setEnabled(true);
        unsetCursor();
    });

    d->m_model = controller->facilitiesModel();
    d->m_filter_model.setSourceModel(d->m_model);

    connect(d->m_model, &QAbstractItemModel::modelReset, this, [this]{
        d->ui.stackedWidget->setCurrentIndex(0);
    });
}

void MainWindow::on_facilityView_clicked(const QModelIndex &index)
{
    if ( auto* facility = d->m_model->facility(d->m_filter_model.mapToSource(index)) ) {

        if ( d->m_facility == facility )
            return;

        if ( d->m_pending &&
                QMessageBox::warning(this,
                    tr("Warning"),
                    tr("You have pending changes that were not applied! \nForget these changes and continue?"),
                    QMessageBox::Button::Yes | QMessageBox::Button::Cancel
                ) != QMessageBox::Button::Yes
            )
        {
            // restore selection
            d->ui.facilityView->selectionModel()->select(
                d->m_filter_model.mapFromSource(d->m_model->indexOf(d->m_facility)),
                QItemSelectionModel::ClearAndSelect
            );
            return;
        }

        d->m_pending = false;
        d->ui.applyButton->setEnabled(false);
        d->m_states.clear();
        d->m_facility = facility;
        d->ui.stackedWidget->setCurrentIndex(1);
        d->ui.facilityInfo->setObject(d->m_facility);

        auto* layout = d->ui.scrollAreaWidgetContents->layout();

        while ( QLayoutItem *child = layout->takeAt(0) ) {
            if ( child->widget() )
                delete child->widget();
            delete child;
        }


        QString error;
        QMessageBox::StandardButton decision = QMessageBox::StandardButton::Yes;

        do {
            decision = d->m_controller->getStates(d->m_facility, error)
                ? QMessageBox::StandardButton::Yes
                : QMessageBox::critical( this, tr("Error"),
                    tr("Failed to get available options for this facility")
                        .append(":\n").append(error),
                    QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Retry
                );
        }
        while ( decision == QMessageBox::StandardButton::Retry );

        if ( decision != QMessageBox::StandardButton::Abort )
            do {
                decision = d->m_controller->getValue(d->m_facility, error)
                    ? QMessageBox::StandardButton::Yes
                    : QMessageBox::critical(this, tr("Error"),
                        tr("Failed to get current state")
                            .append(":\n").append(error),
                        QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Retry
                    );
            }
            while(decision == QMessageBox::StandardButton::Retry);

        if ( d->m_facility->states().empty() ) {
            auto* label = new QLabel{
                tr("This facility cannot be controlled from GUI: a list of possible configuraton states is not available.")
            };
            label->setWordWrap(true);
            label->setAlignment(Qt::AlignHCenter);
            layout->addWidget(label);
        }
        else for ( const auto& state : d->m_facility->states() ) {
            auto* stateForm = new StateForm{state.get(), this};
            layout->addWidget(stateForm);
            if ( d->m_facility->currentState() == state.get() )
                stateForm->radio()->setChecked(true);
            d->m_group.addButton(stateForm->radio());
            d->m_states[stateForm->radio()] = stateForm;
            connect(stateForm, &StateForm::changed, [this,stateForm]{
                d->m_pending = stateForm->isChanged();
                d->ui.applyButton->setEnabled(d->m_pending);
            });
        }

        layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));

        connect(&d->m_group, &QButtonGroup::buttonToggled, this, [this](QAbstractButton* b)
        {
            auto* stateForm = d->m_states.at(static_cast<QRadioButton*>(b));
            d->m_pending = stateForm->state() != d->m_facility->currentState() || stateForm->isChanged();
            d->ui.applyButton->setEnabled(d->m_pending);
        });


    } else {
        qCritical() << "failed to get selected facility";
        d->ui.stackedWidget->setCurrentIndex(0);
    }

}

void MainWindow::on_applyButton_clicked()
{
    auto* stateForm = d->m_states.at(static_cast<QRadioButton*>(d->m_group.checkedButton()));

    QString error;
    if ( d->m_controller->setValue(d->m_facility, stateForm->value().toString(), error) ) {
        d->m_pending = false;
        d->ui.applyButton->setDisabled(true);
    } else {
        QMessageBox::critical(
            this, tr("Error"),
            tr("Failed to apply new settings")
                .append(":\n").append(error)
        );
    }
}
