#include "ArrayEditor.h"
#include "ui_ArrayEditor.h"

#include <QAbstractListModel>

class ChildModel : public QAbstractListModel {
public:
    ChildModel(Property::Value* value)
        : m_data{value->children()}
    {};

    QModelIndex indexOf( const Property::Value* value ) {
        int row = 0;
        for ( const auto& val : m_data ) {
            if ( val.get() == value )
                return index(row,0);
            ++row;
        }

        return {};
    }

    int rowCount(const QModelIndex &parent = {}) const override { return m_data.size(); };
    QVariant data(const QModelIndex &index, int role) const override {
        if ( role == Qt::DisplayRole ) try {
            return m_data.at(index.row())->get();
        } catch (std::out_of_range&){return {};}
        return {};
    };

    void onAdd() {
        beginInsertRows({}, m_data.size(), m_data.size());
        endInsertRows();
    }

    void onRemove(int row){
        beginRemoveRows({}, row, row);
        endRemoveRows();
    }

private:
    const std::vector<ValuePtr>& m_data;
};

class ArrayEditor::Private {
public:
    Private(Property::Value* value, QWidget* parent, Parameter::Contexts contexts)
        : m_editor{createEditor(value->property()->prototype()->defaultValue(), parent, contexts, false, true)}
        , ui{new Ui::ArrayEditor}
        , m_model{value}
    {}

    ~Private(){delete ui;}

    ChildModel m_model;
    EditorPtr m_editor;
    Ui::ArrayEditor *ui;
};


ArrayEditor::ArrayEditor(Property::Value* value, QWidget *parent, Parameter::Contexts contexts)
    : DetailedEditor{value}
    , d{new Private{m_value, parent, contexts}}
{
    m_widget = new QWidget{parent};
    d->ui->setupUi(m_widget);

    d->ui->listView->setModel(&d->m_model);
    d->ui->listView->setModelColumn(0);
    d->ui->removeButton->setEnabled(false);

    connect(d->ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]{
        d->ui->removeButton->setEnabled( d->ui->listView->selectionModel()->hasSelection() );
    });

    connect(d->ui->removeButton, &QAbstractButton::clicked, this, [=]{
        auto row = d->ui->listView->currentIndex().row();
        m_value->removeChild(row);
        d->m_model.onRemove(row);

        checkSize();
    });

    connect(d->ui->addButton, &QAbstractButton::clicked, this, [=]{
        m_value->addChild(d->m_editor->value()->clone());
        d->m_model.onAdd();
        checkSize();
    });

    connect(d->m_editor.get(), &Editor::changed, this, &ArrayEditor::checkSize);
    checkSize();

    d->ui->content->layout()->addWidget(d->m_editor->widget());
    d->ui->indicator->setEditor(d->m_editor.get());
}

ArrayEditor::~ArrayEditor() { delete d; }

QWidget* ArrayEditor::makeVisible(const Property::Value::ValidationInfo* info, int level)
{
    if ( level != 1 )
        qWarning() << "unexpected validation level for ArrayEditor";

    if ( auto* childInfo = info->childInfo.get() ) {
        auto index = d->m_model.indexOf(childInfo->value);
        if ( index.isValid() )
            d->ui->listView->scrollTo(index);
        else
            qWarning() << "child not found";

        return d->ui->listView;
    } else
        qWarning() << "child info not found";

    return m_widget;
}

bool ArrayEditor::checkSize(){
    auto rc = m_value->children().size();
    auto [min,max] = m_value->property()->allowed().toSize();
    bool oor = min != max && rc < min || rc > max;

    d->ui->addButton->setEnabled( rc < max && !d->m_editor->value()->isInvalid() );

    emit changed();
    return oor;
}
