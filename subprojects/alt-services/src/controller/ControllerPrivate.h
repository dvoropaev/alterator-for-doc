#pragma once

#include "Controller.h"
#include "DBusProxy.h"
#include <QActionGroup>

#include "data/models/ServiceModel.h"

class Controller::Private {
public:
    Private(Controller* self);

    std::unique_ptr<Service> buildService(const QString& path, const QString& data) noexcept;

    DBusProxy m_datasource;
    ServiceModel m_model;
    PtrVector<Service> m_services;

    QActionGroup m_table_mode_group;
    QList<QAction*> m_table_actions;

    Controller::DiagErrorHandler* m_diag_error_handler{};
};
