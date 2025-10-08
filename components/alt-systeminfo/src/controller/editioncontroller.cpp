#include "editioncontroller.h"

#include "interface/currenteditioninterface.h"
#include "interface/editioninterface.h"
#include "interface/managerinterface.h"

#include "utility/constants.h"

#include <QDebug>
#include <QLocale>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.h>

namespace alt
{
EditionController::EditionController()
    : m_editionsModel(nullptr)
    , m_selectedEditionId("")
{}

EditionController::~EditionController() = default;

void EditionController::buildModel(QStandardItemModel *model)
{
    if (!model)
    {
        return;
    }

    model->clear();

    const auto &objects = ManagerInterface::getObjects(ALTERATOR_EDITION1_INTERFACE_NAME);
    for (auto &object : objects)
    {
        auto reply = EditionInterface::info(object);
        if (!reply.has_value())
        {
            qWarning() << QString("%1: Edition not found").arg(object);
            continue;
        }

        auto parseResult = toml::parse(reply.value().toStdString());
        if (parseResult.failed())
        {
            qDebug() << "Failed to parse" << std::string(parseResult.error().description()).c_str();
            continue;
        }

        const auto &data = parseResult.table();
        auto name = data[NAME_KEY_NAME].value<std::string>();
        if (!name.has_value())
        {
            qDebug() << QString("Failed to find '%1'").arg(NAME_KEY_NAME);
            continue;
        }
        auto displayNameTable = data[DISPLAY_NAME_KEY_NAME].as_table();
        QString displayName;
        for (const auto &[language, value] : *displayNameTable)
        {
            if (language.str() == QLocale::system().name().split("_")[0].toStdString())
            {
                displayName = QString::fromStdString(value.value<std::string>().value());
            }
        }
        if (displayName.isEmpty())
        {
            if (displayNameTable->contains("en"))
            {
                displayName = displayNameTable->at("en").as_string()->get().c_str();
            }
            else
            {
                qWarning() << QString("Default language key 'en' does not exists in edition '%1'").arg(name->c_str());
            }
        }

        auto item = new QStandardItem();
        item->setData(QString::fromStdString(name.value()), Qt::UserRole);
        item->setData(object, Qt::UserRole + 1);
        item->setText(
            (QString::fromStdString(name.value()) == getCurrentEditionId() ? QString("• %1") : QString("   %1"))
                .arg(displayName));

        model->appendRow(item);
    }

    m_editionsModel = model;
}

int EditionController::setEdition(const QString &editionId)
{
    if (editionId == getCurrentEditionId())
    {
        return -2;
    }

    return CurrentEditionInterface::set(editionId);
}

QString EditionController::getLicense(const QString &editionId)
{
    QString path;
    for (size_t i = 0; i < m_editionsModel->rowCount(); ++i)
    {
        auto index = m_editionsModel->index(i, 0);
        if (index.data(Qt::UserRole).toString() == editionId)
        {
            path = index.data(Qt::UserRole + 1).toString();
        }
    }

    if (path.isEmpty())
    {
        return {};
    }

    const auto &license = EditionInterface::license(path);
    if (!license.has_value())
    {
        return tr("Not found");
    }

    return license.value();
}

QString EditionController::getDescription(const QString &editionId)
{
    QString path;
    for (size_t i = 0; i < m_editionsModel->rowCount(); ++i)
    {
        auto index = m_editionsModel->index(i, 0);
        if (index.data(Qt::UserRole).toString() == editionId)
        {
            path = index.data(Qt::UserRole + 1).toString();
        }
    }

    if (path.isEmpty())
    {
        return {};
    }

    const auto &description = EditionInterface::description(path);
    if (!description.has_value())
    {
        return tr("Not found");
    }

    return description.value();
}

QString EditionController::getCurrentEditionId()
{
    auto reply = CurrentEditionInterface::info();
    if (!reply.has_value())
    {
        qWarning() << "Edition not found";
        return {};
    }

    auto parseResult = toml::parse(reply.value().toStdString());
    if (parseResult.failed())
    {
        qDebug() << "Failed to parse" << std::string(parseResult.error().description()).c_str();
        return {};
    }

    const auto &data = parseResult.table();
    auto name = data[NAME_KEY_NAME].value<std::string>();
    if (!name.has_value())
    {
        qDebug() << QString("Failed to find '%1'").arg(NAME_KEY_NAME);
        return {};
    }

    return QString::fromStdString(name.value());
}

QString EditionController::getStateSelectedEditionId()
{
    return m_selectedEditionId;
}

void EditionController::setStateSelectedEditionId(const QString &editionId)
{
    m_selectedEditionId = editionId;
}

} // namespace alt
