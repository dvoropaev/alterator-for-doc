#pragma once

#include "Controller.h"
#include "DBusProxy.h"
#include "ui/ActionWizard.h"
#include <QActionGroup>

#include "data/models/ServiceModel.h"

class Controller::Private {
public:
    Private(Controller* self, QWidget* window = nullptr);

    std::unique_ptr<Service> buildService(const QString& path, const QString& data) noexcept;

    DBusProxy  m_datasource;
    ServiceModel    m_model;
    PtrVector<Service> m_services;

    QActionGroup m_group;
    QAction m_compact;
    QAction m_detailed;

    ActionWizard    m_wizard;
};
