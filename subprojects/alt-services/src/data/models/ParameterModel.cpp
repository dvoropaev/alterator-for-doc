#include "ParameterModel.h"

#include "ResourceModel.h"
#include <QApplication>
#include <QIcon>
#include <QFont>
#include <QPalette>

#include <range/v3/algorithm.hpp>

void ParameterModel::setItems(const std::vector<Parameter*>& items) {
    beginResetModel();
    m_items = items;
    endResetModel();
}

void ParameterModel::setScope(Parameter::ValueScope scope)
{
    beginResetModel();
    m_scope = scope;
    endResetModel();
}


int ParameterModel::indexOf(const Parameter* param) const {
    auto match = ranges::find(m_items, param);
    return match == m_items.cend() ? -1 : std::distance(m_items.cbegin(), match);
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

    return index( indexOf( static_cast<Parameter*>(value->property()) ), 0 );
}

Property::Value* ParameterModel::indexToValue(const QModelIndex& index) const
{
    if ( checkIndex(index, CheckIndexOption::IndexIsValid) )
        return (Property::Value*)index.internalPointer();
    return nullptr;
}


QModelIndex ParameterModel::index(int row, int column, const QModelIndex& parent) const {
    if ( column < 0 || column >= columnCount(parent) || row < 0 || row >= rowCount(parent) )
        return {};

    if ( parent.isValid() ) {
        auto value = static_cast<Property::Value*>(parent.internalPointer());

        if ( value->property()->valueType() == Property::Type::Enum ) {

            auto currenVariant = ranges::find_if(value->children(), &Property::Value::isEnabled);

            if ( currenVariant != value->children().cend() ) {

                if ( currenVariant->get()->children().size() <= row )
                    return {};

                int i = row;
                for ( const auto& v : currenVariant->get()->children() ) {
                    if ( currenVariant->get()->children()[i]->property()->isConstant() )
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
    if ( auto* value = static_cast<Property::Value*>(child.internalPointer()) ) {

        if ( Property::Value* parent = value->parent() ) {
            if ( Property::Value* gramps = parent->parent() ) {
                if ( gramps->property()->valueType() == Property::Type::Enum )
                    return indexOf(gramps);
                else
                    return createIndex(gramps->indexOf(parent), 0, parent);
            }

            return createIndex( indexOf(static_cast<Parameter*>(parent->property())), 0, parent);
        }
    }

    return {};
}

int ParameterModel::rowCount(const QModelIndex& parent) const {
    if ( parent.isValid() ) {
        auto value = static_cast<Property::Value*>(parent.internalPointer());

        if ( value->property()->valueType() == Property::Type::Enum ) {

            auto currentVariant = ranges::find_if(value->children(), &Property::Value::isEnabled);

            if ( currentVariant != value->children().cend() )
                return ranges::count_if(currentVariant->get()->children(),
                                        std::not_fn(&Property::isConstant),
                                        &Property::Value::property);

            return 0;
        }
        return value->children().size();
    }

    return m_items.size();
}

int ParameterModel::columnCount(const QModelIndex& parent) const {
    return 2;
}

QVariant ParameterModel::data(const QModelIndex& index, int role) const
{
    auto* value = indexToValue(index);
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
        break;

        case Qt::ToolTipRole:
            return property->comment();
        break;

        case Qt::DecorationRole:
            if ( auto parameter = dynamic_cast<Parameter*>(property) ) {
                return parameter->resource()
                    ? ResourceModel::resourceIcon(parameter->resource()->type())
                    : QVariant{};
            }
        break;

        case Qt::UserRole:
            if ( !index.parent().isValid() )
                return QVariant::fromValue( static_cast<Parameter*>(value->property())->resource() );
        break;
    }
    else if ( index.column() == 1 ) switch (role) {
        case Qt::DisplayRole:
            if ( property->valueType() == Property::Type::Enum ) {
                auto currentVariant = ranges::find_if(value->children(), &Property::Value::isEnabled);
                if ( currentVariant != value->children().cend() )
                    return currentVariant->get()->property()->displayName();
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
        break;

        case Qt::ToolTipRole:
            if ( property->valueType() == Property::Type::Enum ) {
                auto currentVariant = ranges::find_if(value->children(), &Property::Value::isEnabled);
                if ( currentVariant != value->children().cend() )
                    return currentVariant->get()->property()->comment();
            }
            return {};
        break;

        case Qt::FontRole:
        {
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

inline Property::Value* ParameterModel::getValue(Parameter* p) const { return p->value(m_scope); }

