#include "CompactForm.h"

#include "data/models/ParameterModel.h"
#include "ui/ObjectInfoDelegate.h"


#include "ui/CustomTreeView.h"

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>

#include <QTimer>

#include <QHeaderView>

class EditModel : public ParameterModel {
    Q_OBJECT
    friend class EditDelegate;
    const Action& m_action;
public:
    inline EditModel(const Action& action)
        : m_action{action}
    {
        setScope(Parameter::ValueScope::Edit);
    }

    Parameter::Contexts contexts{};

    int getRow(Property::Value* value) {
        if ( value->parent() )
            return value->parent()->indexOf(value);

        return indexOf((Parameter*)value->property());
    }

    void remove(Property::Value* value) {
        auto row = getRow(value);
        QModelIndex parent = createIndex(row, 0, value).parent();
        beginRemoveRows( parent, row, row );
        value->parent()->removeChild(row);
        endRemoveRows();
    }

    void validate(const QModelIndex& index) {
        if ( index.isValid() ) {
            emit dataChanged(index.siblingAtColumn(0), index.siblingAtColumn(0), {Qt::DecorationRole});
            validate(index.parent());
        }
    }

    QVariant data(const QModelIndex& index, int role) const override {
        auto* value = ParameterModel::indexToValue(index);

        if ( role == Qt::DecorationRole && index.column() == 0 )
            return value->isInvalid(m_action.options.force) ? QIcon::fromTheme("dialog-warning") : QVariant{};

        if ( role == Qt::CheckStateRole ) {

            auto* property = value->property();
            auto* parameter = dynamic_cast<Parameter*>( property );
            bool required = property->isConstant() ||
                            property->isRequired() ||
                            ( parameter && parameter->required() & contexts ) ||
                            ( value->parent() && (
                                  value->parent()->property()->valueType() == Property::Type::Enum ||
                                  value->parent()->property()->valueType() == Property::Type::Array
                                )
                            );

            if ( required )
                return {};

            if ( index.column() == 0 )
                return value->isEnabled() ? Qt::Checked : Qt::Unchecked;

            if ( value->isEnabled() )
                return {};
        }

        return ParameterModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        auto* value = ParameterModel::indexToValue(index);
        auto flags = ParameterModel::flags(index);

        auto* property = value->property();
        auto* parameter = dynamic_cast<Parameter*>( property );
        bool required = property->isConstant() ||
                        property->isRequired() ||
                        ( parameter && parameter->required() & contexts ) ||
                        ( value->parent() && (
                              value->parent()->property()->valueType() == Property::Type::Enum ||
                              value->parent()->property()->valueType() == Property::Type::Array
                            )
                        );

        auto parent = index.parent();

        flags.setFlag(Qt::ItemIsUserCheckable, index.column() == 0 && !required &&
                      ( !parent.isValid() ||
                        !parent.flags().testFlag(Qt::ItemIsUserCheckable) ||
                        parent.data(Qt::CheckStateRole).toInt() != Qt::Unchecked ) );
        return flags;
    }

    bool setData(const QModelIndex& idx, const QVariant& value, int role) override {
        if ( idx.column() == 0 ) {
            auto* propertyValue = (Property::Value*)idx.internalPointer();

            if ( auto parent = propertyValue->parent() )
                if ( !parent->isEnabled() )
                    return false;

            propertyValue->setEnabled(!propertyValue->isEnabled());
            emit dataChanged(idx, idx.siblingAtColumn(1), {Qt::CheckStateRole, Qt::ForegroundRole});
            if ( int count = rowCount(idx) )
                emit dataChanged(index(0,0,idx), index(count-1, 1, idx), {Qt::CheckStateRole, Qt::ForegroundRole});

            return true;
        }
        return false;
    }

public slots:
    void onChange(const QModelIndex& index) { emit dataChanged(index,index); }

protected:
    int m_context;
};

#include "editors/Editor.h"

#include "ui/ObjectInfoDelegate.h"
#include <QStyledItemDelegate>

class EditDelegate : public ObjectInfoDelegate {
    Q_OBJECT
private:
    EditModel& m_model;
    const BaseForm* m_form;
public:
signals:
    void changed() const;
public:
    EditDelegate(EditModel& model, const BaseForm* form)
        : m_model{model}
        , m_form{form}
    {}

    void createRemoveButton(Property::Value* value, QWidget* container) const {
        auto removeBtn = new QPushButton{container};
        removeBtn->setFocusPolicy(Qt::StrongFocus);
        removeBtn->setIcon(QIcon::fromTheme("list-remove"));
        removeBtn->setToolTip(tr("Remove"));
        if ( removeBtn->icon().isNull() )
            removeBtn->setText(tr("Remove"));
        connect(removeBtn, &QPushButton::clicked, [this, value]{ m_model.remove(value); emit changed(); });
        removeBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        if ( container->layout()->count() == 0 )
            container->layout()->addItem(new QSpacerItem{0,0, QSizePolicy::Expanding});

        container->layout()->addWidget(removeBtn);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        auto* value = m_model.indexToValue(index);

        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout{};
        layout->setContentsMargins(QMargins{1,1,1,1});
        container->setLayout(layout);
        container->setAutoFillBackground(true);

        if ( Editor* editor = ::createEditor(*m_form, value, parent, Parameter::Context::Deploy, true).release() ) {
            connect(editor, &Editor::changed, this, &EditDelegate::changed);

            editor->fill();
            editor->setParent(container);
            layout->addWidget(editor->widget());

            connect(editor, &Editor::changed, [this, value]{m_model.validate(m_model.indexOf(value));});

            if ( value->parent() && value->parent()->property()->valueType() == Property::Type::Array )
                createRemoveButton(value, container);

            if ( value->property()->valueType() == Property::Type::Enum ) {
                auto remove = std::make_shared<bool>(false);

                connect(editor, &Editor::aboutToChange, [this, value, remove] {
                    auto index = m_model.indexOf(value);

                    if ( int count = m_model.rowCount(index) ) {
                        m_model.beginRemoveRows(index, 0, count-1);
                        *remove.get() = true;
                    }
                });

                connect(editor, &Editor::changed, [this, value, remove] {

                    auto index = m_model.indexOf(value);

                    auto it = std::find_if(value->children().begin(), value->children().end(),
                                           [](auto& child){return child->isEnabled();});

                    if ( it != value->children().end() ) {

                        int newCount = std::count_if(it->get()->children().cbegin(), it->get()->children().cend(),
                                                      [](const auto& val){return !val->property()->isConstant();});


                        it->get()->setEnabled(false);

                        if ( *remove )
                            m_model.endRemoveRows();

                        *remove = false;

                        if ( newCount )
                            m_model.beginInsertRows(index, 0, newCount-1);

                        it->get()->setEnabled(true);

                        if ( newCount )
                            m_model.endInsertRows();

                    }
                });


            } else if ( value->property()->valueType() == Property::Type::Array ) {
                connect(editor, &Editor::aboutToChange, [=] {
                    auto row = value->children().size();

                    auto index = m_model.indexOf(value);
                    m_model.beginInsertRows(index, row, row);
                });
                connect(editor, &Editor::changed, [=] { m_model.endInsertRows(); });

                connect(&m_model, &QAbstractItemModel::rowsRemoved, editor, [=](const QModelIndex& parent, int first, int last){
                    if ( parent.internalPointer() == index.internalPointer() )
                        editor->fill();
                });
            }

            return container;
        } else if ( value->parent() && value->parent()->property()->valueType() == Property::Type::Array ) {
            createRemoveButton(value, container);
        }

        return container;
    }

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QSize s = QStyledItemDelegate::sizeHint(option, index);
        s.setHeight(40);
        return s;
    }
};


class TabNavigatedTreeView : public CustomTreeView {
private:
    bool removePending{false};

public:
    TabNavigatedTreeView()
        : CustomTreeView{}
    {
        setTabKeyNavigation(true);
    }

    void setModel(EditModel* m)
    {
        connect(m, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this]{removePending = true; });
        connect(m, &QAbstractItemModel::rowsRemoved,          this, [this]{removePending = false;});
        CustomTreeView::setModel(m);
    }

protected:
    bool focusNextPrevChild(bool next) override
    {
        if ( removePending )
            return true;

        QModelIndex current = currentIndex();
        if ( !current.isValid() )
            current = model()->index(0, 0);

        int currentRow = current.row();
        int currentCol = current.column();


        if ( isPersistentEditorOpen(current) )
        {
            // if current index has an editor, make sure we'll walk through its children
            if ( auto* widget = indexWidget(current) )
            {
                auto children = widget->findChildren<QWidget*>(Qt::FindChildOption::FindDirectChildrenOnly);
                QList<QWidget*>::Iterator it = std::find_if(children.begin(), children.end(), std::mem_fn(&QWidget::hasFocus));

                int i = std::distance(children.begin(), it);
                i += (next ? 1 : -1);

                for ( ; i < children.size() && i >= 0 ; i += (next ? 1 : -1) )
                {
                    if ( children[i]->focusPolicy() & Qt::TabFocus )
                    {
                        children[i]->setFocus(Qt::TabFocusReason);
                        return true;
                    }
                }
            }
        }

        QModelIndex nextIndex;

        // horizontal
        nextIndex = isFirstColumnSpanned(currentRow, current.parent())
            ? QModelIndex{}
            : current.siblingAtColumn(currentCol + (next? 1 : -1));

        // vertical
        if (!nextIndex.isValid())
            nextIndex = next
                ? indexBelow(current).siblingAtColumn(0)
                : indexAbove(current).siblingAtColumn(model()->columnCount()-1);

        if ( nextIndex.isValid() )
        {
            // NOTE: this is needed to take focus from editor, so current cell will actually be highlighted
            setFocus(Qt::TabFocusReason);

            setCurrentIndex(nextIndex);
            if ( isPersistentEditorOpen(nextIndex) )
                if ( auto* widget = indexWidget(nextIndex) )
                {
                    auto children = widget->findChildren<QWidget*>();
                    for ( auto* child : children )
                    {
                        child->setFocus(Qt::FocusReason::TabFocusReason);
                        if ( child->focusPolicy() & Qt::TabFocus )
                        {
                            break;
                        }
                    }
                }

            return true;
        }
        return QTreeView::focusNextPrevChild(next);
    }
};

#include "ui/ParameterSearcher.h"
class ParameterViewModelAdaptor : public ParameterSearcher
{
    Q_OBJECT
    CustomTreeView& m_view;
public:
    ParameterViewModelAdaptor(ParameterModel& model, CustomTreeView& view)
        : ParameterSearcher{&model}
        , m_view{view}
    {}

public slots:
    void prev() override
    { m_view.highlight(ParameterSearcher::prevIndex()); }

    void next() override
    { m_view.highlight(ParameterSearcher::nextIndex()); }
};
#include "CompactForm.moc"

class CompactForm::Private {
public:
    Private(const CompactForm* self, const Action& action)
        : m_model{action}
        , m_delegate{m_model, self}
        , m_searchAdapter{m_model, m_view}
    {}

    void openPersistentEditor(const QModelIndex& parent = {})
    {
        if ( parent.isValid() && parent.column() != 1 )
        {
            qCritical() << "Interanal error:"
                        << __PRETTY_FUNCTION__
                        << "called with index at incorrect column";
            return;
        }

        if ( parent.isValid() && parent.column() == 1 && !m_view.isRowHidden(parent.row(), parent.parent()) ) {
            auto checked = parent.siblingAtColumn(0).data(Qt::CheckStateRole);
            if ( checked.isValid() && checked.toInt() == Qt::Unchecked )
                return;
            m_view.openPersistentEditor(parent);
        }

        for ( int i = 0; i < m_model.rowCount(parent); ++i )
        {
            auto child = m_model.index(i,1, parent);
            if ( child.isValid() )
                openPersistentEditor(child);
        }
    }

    void closePersistentEditor(const QModelIndex& parent = {})
    {
        if ( parent.isValid() )
            m_view.closePersistentEditor(parent);


        for ( int i = 0; i < m_model.rowCount(parent); ++i )
            closePersistentEditor(m_model.index(i,1, parent));
    }


    TabNavigatedTreeView m_view;
    EditModel m_model;
    EditDelegate m_delegate;
    ParameterViewModelAdaptor m_searchAdapter;
};

CompactForm::CompactForm(const Action& action, QWidget* parent)
    : BaseForm{action, parent}
    , d{ new Private{this, action} }
{
    connect(&d->m_delegate, &EditDelegate::changed, this, &BaseForm::changed);

    setMinimumWidth(500);
    setLayout(new QVBoxLayout{});
    layout()->setContentsMargins({});
    layout()->addWidget(&d->m_view);
    layout()->addItem(new QSpacerItem{0,0});

    d->m_view.setSortingEnabled(false);

    d->m_view.setModel(&d->m_model);
    d->m_view.setItemDelegateForColumn(1, &d->m_delegate);

    connect(&d->m_view, &QTreeView::collapsed, this, [this](const auto& index){
        d->closePersistentEditor(index.siblingAtColumn(1));
    });
    connect(&d->m_view, &QTreeView::expanded,  this, [this](const auto& index){
        d-> openPersistentEditor(index.siblingAtColumn(1));
    });

    connect(&d->m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex& parent, int first, int last){
        for ( int i = first; i <= last; ++i )
            d->closePersistentEditor(d->m_model.index(i,1, parent));
    });
    connect(&d->m_model, &QAbstractItemModel::rowsInserted, this, [=](const QModelIndex& parent, int first, int last){
        for ( int i = first; i <= last; ++i )
            d->openPersistentEditor(d->m_model.index(i,1, parent));
    });

    connect(&d->m_model, &QAbstractItemModel::dataChanged, [this](const QModelIndex& tl, const QModelIndex& br, const QList<int>& roles){
        if ( roles.contains(Qt::CheckStateRole) ) {
            for ( int row = tl.row(); row <= br.row(); ++row ) {
                auto state = tl.data(Qt::CheckStateRole);
                auto index = tl.siblingAtRow(row).siblingAtColumn(1);
                if ( state.isValid() )
                {
                    if ( state.toInt() == Qt::Unchecked )
                        d->closePersistentEditor(index);
                    else
                        d->openPersistentEditor(index);

                    d->m_model.validate(index);
                }
            }
            emit changed();
        }
    });
}

CompactForm::~CompactForm() {delete d;}

void CompactForm::ensureVisible(const Parameter::Value* value)
{
    auto index = d->m_model.indexOf(value);
    d->m_view.expand(index);
    d->m_view.scrollTo(index);
    d->m_view.highlight(index);
}

SearchAdapter* CompactForm::searchAdapter() { return &d->m_searchAdapter; }

void CompactForm::setParametersImpl(Parameter::Contexts contexts)
{
    d->m_model.contexts = contexts;
    d->m_model.setItems(m_parameters);

    d->openPersistentEditor();
    d->m_view.header()->resizeSection(0, 300);
}
