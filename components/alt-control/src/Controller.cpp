#include "Controller.h"

#include "DBusDataSource.h"
#include <QStandardItemModel>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class Controller::Private {
public:
    Private(){}
    DBusDataSource m_datasource;
    QStandardItemModel m_model;
};

Controller::Controller()
    : d { new Private{} }
{}


void Controller::refresh(){
    emit beginRefresh();
    d->m_model.clear();

    auto data = d->m_datasource.getFacilitiesJson();
    auto doc = QJsonDocument::fromJson(data);
    if ( doc.isArray() ) {
        auto array = doc.array();

        d->m_model.setColumnCount(3);

        int row = 0;
        for ( const auto& facility : array ) {
            auto obj = facility.toObject();

            auto values = obj["list"].toArray();
            auto helps  = obj["help"].toArray();

            QVariantMap valueHelpMap;
            for ( const auto& v : values ) {
                QString value = v.toString();
                QString help;

                for ( const auto& h : helps )
                    if ( h.toString().startsWith(value) ) {
                        help = h.toString();
                        break;
                    }

                valueHelpMap[value] = help;
            }

            QVariant v = QVariant::fromValue(valueHelpMap);

            auto items = QList{
                new QStandardItem{obj["name"   ].toString()},
                new QStandardItem{obj["summary"].toString()},
                new QStandardItem{obj["status" ].toString()}
            };

            items[2]->setData(v);

            d->m_model.insertRow(row++, items);
        }
    }

    emit endRefresh();
}

Controller::~Controller() { delete d; }

QAbstractItemModel* Controller::facilitiesModel() { return &d->m_model; }

bool Controller::setValue(const QString& facility, const QString& value, QString& error)
{
    emit beginRefresh();
    bool result = d->m_datasource.setValue(facility, value, error);
    emit endRefresh();
    return result;
}
