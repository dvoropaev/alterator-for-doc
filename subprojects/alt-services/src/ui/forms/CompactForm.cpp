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
public:
    using ParameterModel::ParameterModel;

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
        auto* value = (Property::Value*)index.internalPointer();

        if ( role == Qt::DecorationRole && index.column() == 0 )
            return value->isInvalid() ? QIcon::fromTheme("dialog-warning") : QVariant{};

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
        auto* value = (Property::Value*)index.internalPointer();
        auto flags = ParameterModel::flags(index);
        flags.setFlag(Qt::ItemIsEnabled, (!value->parent() || value->parent()->isEnabled()) && value->isEnabled());

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
                      ( !parent.isValid() || parent.flags().testFlag(Qt::ItemIsEnabled) ) );
        return flags;
    }

    // bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    //     if ( index.column() == 0 && role == Qt::CheckStateRole ) {
    //         auto* propertyValue = (Property::Value*)index.internalPointer();
    //         propertyValue->setEnabled(value.toBool());
    //         emit dataChanged(index, index, {role});
    //     }
    //     return false;
    // }

    void toggle(const QModelIndex& idx) {
        if ( idx.column() == 0 ) {
            auto* propertyValue = (Property::Value*)idx.internalPointer();

            if ( auto parent = propertyValue->parent() )
                if ( !parent->isEnabled() )
                    return;

            propertyValue->setEnabled(!propertyValue->isEnabled());
            emit dataChanged(idx, idx, {Qt::CheckStateRole});
            if ( int count = propertyValue->children().size() )
                emit dataChanged(index(0,0,idx), index(count-1, 0, idx), {Qt::CheckStateRole});
        }
    }

public slots:
    void onChange(const QModelIndex& index) { emit dataChanged(index,index); }

protected:
    Property::Value* getValue(Parameter* p) const override { return p->editValue(); }
    int m_context;
};

#include "editors/Editor.h"

#include "ui/ObjectInfoDelegate.h"
#include <QStyledItemDelegate>

class EditDelegate : public ObjectInfoDelegate {
    Q_OBJECT
private:
    EditModel& m_model;
public:
signals:
    void changed() const;
public:
    EditDelegate(EditModel& model) : m_model{model} {}

    void createRemoveButton(Property::Value* value, QWidget* container) const {
        auto removeBtn = new QPushButton{container};
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
        auto* value = (Property::Value*)index.internalPointer();

        auto* container = new QWidget(parent);
        auto* layout = new QHBoxLayout{};
        layout->setContentsMargins(QMargins{1,1,1,1});
        container->setLayout(layout);
        container->setAutoFillBackground(true);

        if ( Editor* editor = ::createEditor(value, parent, Parameter::Context::Deploy, true).release() ) {
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

                        bool newCount = std::count_if(it->get()->children().cbegin(), it->get()->children().cend(),
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
#include "CompactForm.moc"



class CompactForm::Private {
public:
    Private()
        : m_model{}
        , m_delegate{m_model}
    {}

    void openPersistentEditor(const QModelIndex& parent = {})
    {
        if ( parent.isValid() && parent.column() == 1 && !m_view.isRowHidden(parent.row(), parent.parent()) ) {
            auto checked = parent.siblingAtColumn(0).data(Qt::CheckStateRole);
            if ( !parent.flags().testFlag(Qt::ItemIsEnabled) )
                return;
            m_view.openPersistentEditor(parent);
        }

        for ( int i = 0; i < m_model.rowCount(parent); ++i )
            openPersistentEditor(m_model.index(i,1, parent));
    }

    void closePersistentEditor(const QModelIndex& parent = {})
    {
        if ( parent.isValid() )
            m_view.closePersistentEditor(parent);


        for ( int i = 0; i < m_model.rowCount(parent); ++i )
            closePersistentEditor(m_model.index(i,1, parent));
    }


    CustomTreeView m_view;
    EditModel m_model;
    EditDelegate m_delegate;
};

CompactForm::CompactForm(QWidget* parent)
    : BaseForm{parent}
    , d{ new Private{} }
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

    connect(&d->m_view, &QAbstractItemView::clicked, this, [this](const QModelIndex& index){
        if ( index.column() == 0 ) {
            auto checked = index.data(Qt::CheckStateRole);
            if ( checked.isValid() )
                //d->m_model.setData(index, !checked.toBool(), Qt::CheckStateRole);

                /*
                 * QAbstractItemView::clicked is emitted even on checkboxes
                 * so we do not use setData here to avoid calling it twice
                 */
                d->m_model.toggle(index);
        }
    });

    connect(&d->m_view, &QTreeView::collapsed, this, [this](const auto& index){
        d->closePersistentEditor(index);
    });
    connect(&d->m_view, &QTreeView::expanded,  this, [this](const auto& index){
        d-> openPersistentEditor(index);
    });

    connect(&d->m_model, &QAbstractItemModel::rowsInserted, [this]{
        d->closePersistentEditor();
        d-> openPersistentEditor();
    });
    connect(&d->m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [=]{
        d->closePersistentEditor();
    });
    connect(&d->m_model, &QAbstractItemModel::rowsRemoved, this, [=]{
        d-> openPersistentEditor();
    });

    connect(&d->m_model, &QAbstractItemModel::dataChanged, [this](const QModelIndex& tl, const QModelIndex& br, const QList<int>& roles){
        if ( roles.contains(Qt::CheckStateRole) ) {
            for ( int row = tl.row(); row <= br.row(); ++row ) {
                auto index = tl.siblingAtRow(row).siblingAtColumn(1);
                if ( index.flags().testFlag(Qt::ItemIsEnabled) )
                    d->openPersistentEditor(index);
                else
                    d->closePersistentEditor(index);

                d->m_model.validate(index);
            }
            emit changed();
        }
    });

}

CompactForm::~CompactForm() {delete d;}

void CompactForm::ensureVisible(const Parameter::Value::ValidationInfo* invalid, int level)
{   
    auto index = d->m_model.indexOf(invalid->value);
    d->m_view.expand(index);
    d->m_view.scrollTo(index);

    if ( level && invalid->childInfo )
        ensureVisible(invalid->childInfo.get(), level - 1);
    else
        d->m_view.highlight(index);
}

void CompactForm::setParametersImpl(Parameter::Contexts contexts)
{
    d->m_model.contexts = contexts;
    d->m_model.setItems(m_parameters);

    d->openPersistentEditor();
    d->m_view.header()->resizeSection(0, 300);
}
