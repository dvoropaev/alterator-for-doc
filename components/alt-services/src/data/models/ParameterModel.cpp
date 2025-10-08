#include "ParameterModel.h"

#include "ResourceModel.h"
#include <QApplication>
#include <QIcon>
#include <QFont>
#include <QPalette>

ParameterModel::ParameterModel(const PtrVector<Parameter>& data)
{
    for ( auto& param : data )
        if ( !param->isConstant() && ( param->contexts() & Parameter::Context::Status ) )
            m_items.push_back(param.get());
}

void ParameterModel::setItems(const std::vector<Parameter*>& items) {
    beginResetModel();
    m_items = items;
    endResetModel();
}


int ParameterModel::indexOf(const Parameter* param) const {
    auto it = std::find(m_items.cbegin(), m_items.cend(), param);
    return it == m_items.cend() ? -1 : std::distance(m_items.cbegin(), it);
}

QModelIndex ParameterModel::indexOf(const Property::Value* value) const
{
    if ( auto* parent = value->parent() ) {
        if ( auto* gramps = parent->parent() ) {
            if ( gramps->property()->valueType() == Property::Type::Enum )
                return index(parent->indexOf(value), 0, indexOf(gramps) );
        }

        return index(parent->indexOf(value), 0, indexOf(parent) );
    }

    return index( indexOf( (Parameter*)value->property() ), 0 );
}


QModelIndex ParameterModel::index(int row, int column, const QModelIndex& parent) const {
    if ( parent.isValid() ) {
        auto value = (Property::Value*)parent.internalPointer();

        if ( row >= rowCount(parent) )
            return {};

        if ( value->property()->valueType() == Property::Type::Enum ) {

            auto it = std::find_if(value->children().cbegin(),
                                   value->children().cend(),
                                   [](const auto& child){ return child->isEnabled(); });

            if ( it != value->children().cend() ) {

                if ( it->get()->children().size() <= row )
                    return {};

                int i = row;
                for ( const auto& v : it->get()->children() ) {
                    if ( it->get()->children()[i]->property()->isConstant() )
                        continue;

                    if ( i )
                        --i;
                    else
                        return createIndex(row, column, v.get());;
                }

            }

            return {};
        }
        try {
            return createIndex(row, column, value->children().at(row).get());
        } catch (...){ return {}; }
    }

    auto parameter = m_items.at(row);
    return createIndex(row, column, getValue(parameter) );
}

QModelIndex ParameterModel::parent(const QModelIndex& child) const {
    if ( auto value = (Property::Value*)child.internalPointer() ) {

        if ( auto* parent = value->parent() ) {
            if ( auto* gramps = parent->parent() ) {
                if ( gramps->property()->valueType() == Property::Type::Enum )
                    return indexOf(gramps);
                else
                    return createIndex(gramps->indexOf(parent), 0, parent);
            }

            return createIndex( indexOf((Parameter*)parent->property()), 0, parent);
        }
    }

    return {};
}

int ParameterModel::rowCount(const QModelIndex& parent) const {
    if ( parent.isValid() ) {
        auto value = (Property::Value*)parent.internalPointer();

        if ( value->property()->valueType() == Property::Type::Enum ) {

            auto it = std::find_if(value->children().cbegin(),
                                   value->children().cend(),
                                   [](const auto& child){ return child->isEnabled(); });

            if ( it != value->children().cend() )
                return std::count_if(it->get()->children().cbegin(), it->get()->children().cend(),
                                     [](const auto& val){return !val->property()->isConstant();});

            return 0;
        }
        return value->children().size();
    }

    return m_items.size();
}

int ParameterModel::columnCount(const QModelIndex& parent) const {
    if ( !parent.isValid() || parent.column() == 0 ) return 2;
    return 0;
}

QVariant ParameterModel::data(const QModelIndex& index, int role) const
{
    auto* value = (Property::Value*)index.internalPointer();
    auto* property = value->property();
    auto  type = property->valueType();

    if ( role == Qt::ForegroundRole ) {
        bool enabled = (!value->parent() || value->parent()->isEnabled()) && value->isEnabled();
        return QApplication::palette().brush(enabled ? QPalette::ColorGroup::Current : QPalette::ColorGroup::Disabled,
                                             QPalette::ColorRole::Text);
    }

    if ( index.column() == 0 ) switch (role) {
        case Qt::DisplayRole:
            return value->displayName(true);

        case Qt::ToolTipRole:
            return property->comment();

        case Qt::DecorationRole:
            if ( auto parameter = dynamic_cast<Parameter*>(property) ) {
                return parameter->resource()
                    ? ResourceModel::resourceIcon(parameter->resource()->type())
                    : QVariant{};
            }

        case Qt::UserRole:
            if ( !index.parent().isValid() )
                return QVariant::fromValue( ((Parameter*)value->property())->resource() );
    }
    else if ( index.column() == 1 ) switch (role) {
        case Qt::DisplayRole:
            if ( property->valueType() == Property::Type::Enum ) {
                auto it = std::find_if(value->children().cbegin(),
                                       value->children().cend(),
                                       [](const auto& child){ return child->isEnabled(); });
                if ( it != value->children().cend() )
                    return it->get()->property()->displayName();
            }

            if ( type == Property::Type::Bool )
                return value->get().toBool() ? tr("Yes") : tr("No");

            if ( type == Property::Type::Array )
                return tr("%n items", nullptr, value->children().size());

            if ( type == Property::Type::Composite && value->children().size() )
                return {};

            if ( value->get().isNull() )
                return tr("(not specified)");

            return value->property()->isPassword() ? tr("(password hidden)") : value->get();

        case Qt::ToolTipRole:
            if ( property->valueType() == Property::Type::Enum ) {
                auto it = std::find_if(value->children().cbegin(),
                                       value->children().cend(),
                                       [](const auto& child){ return child->isEnabled(); });
                if ( it != value->children().cend() )
                    return it->get()->property()->comment();
            }
            return {};

        case Qt::FontRole: {
            QFont f;
            f.setItalic( property->isPassword() ||
                         type == Property::Type::Array ||
                         ( property->valueType() != Property::Type::Enum && value->get().isNull() ) );
            return f;
        }

    }

    return {};
}

QVariant ParameterModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch ( section ) {
            case 0: return tr("Name");
            case 1: return tr("Value");
        }
    }

    return {};
}

