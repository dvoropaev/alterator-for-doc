#include "databuilder.h"

#include "interface/aptinterface.h"
#include "interface/currenteditioninterface.h"
#include "interface/managerinterface.h"
#include "interface/systeminfointerface.h"

#include "utility/constants.h"
#include "utility/debug.h"

#define TOML_EXCEPTIONS 0
#include <algorithm>
#include <toml++/toml.h>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimeZone>

namespace alt
{
struct ErrorMessage
{
    static QString notFound() { return QObject::tr("Not found"); }
    static QString lastUpdateError() { return QObject::tr("Update the system to receive information"); }
};

struct MemoryUnit
{
    static QString bytes() { return QObject::tr("B"); }

    static QString kilabytes() { return QObject::tr("KB"); }
    static QString megabytes() { return QObject::tr("MB"); }
    static QString gigabytes() { return QObject::tr("GB"); }
    static QString terabytes() { return QObject::tr("TB"); }
    static QString petabytes() { return QObject::tr("PB"); }

    static QString kibibytes() { return QObject::tr("KiB"); }
    static QString mebibytes() { return QObject::tr("MiB"); }
    static QString gibibytes() { return QObject::tr("GiB"); }
    static QString tebibytes() { return QObject::tr("TiB"); }
    static QString pebibytes() { return QObject::tr("PiB"); }
};

struct FrequencyUnit
{
    static QString herz() { return QObject::tr("Hz"); }
    static QString kilaherz() { return QObject::tr("KHz"); }
    static QString megaherz() { return QObject::tr("MHz"); }
    static QString gigaherz() { return QObject::tr("GHz"); }
};

void DataBuilder::rebuildProduct()
{
    rebuildProductDisplayName();
    rebuildBranch();
    rebuildEdition();
    rebuildProductLicense();
    rebuildProductLogo();
}

void DataBuilder::rebuildEdition()
{
    m_product.setEdition(std::nullopt);
    auto edition = Edition();

    auto reply = CurrentEditionInterface::info();
    if (!reply.has_value())
    {
        return;
    }

    auto parseResult = toml::parse(reply.value().toStdString());
    if (parseResult.failed())
    {
        qDebug() << "Failed to parse" << std::string(parseResult.error().description()).c_str();
        return;
    }

    // Name
    auto name = parseResult.table()[NAME_KEY_NAME].value<std::string>();
    if (!name.has_value())
    {
        qDebug() << QString("Failed to find '%1'").arg(NAME_KEY_NAME);
        return;
    }
    edition.m_name = name->c_str();

    // Display Name
    auto *displayNameTable = parseResult.table()[DISPLAY_NAME_KEY_NAME].as_table();
    if (!displayNameTable)
    {
        qDebug() << QString("Failed to find '%1'").arg(DISPLAY_NAME_KEY_NAME);
        edition.m_displayName = edition.m_name;
    }
    else
    {
        auto systemLanguage = QLocale::system().name().split("_")[0];
        for (const auto &[language, value] : *displayNameTable)
        {
            if (language.str() == systemLanguage.toStdString())
            {
                auto displayName = value.value<std::string>();
                if (!displayName.has_value())
                {
                    qDebug() << QString("'%1' is not string").arg(DISPLAY_NAME_KEY_NAME);
                    break;
                }

                edition.m_displayName = QString::fromStdString(*displayName);
                break;
            }
        }
    }

    if (!edition.m_displayName.has_value())
    {
        auto defaultDisplayName = (*displayNameTable)[DEFAULT_LANGUAGE].value<std::string>();
        if (defaultDisplayName.has_value())
        {
            edition.m_displayName = defaultDisplayName->c_str();
        }
    }

    // Desktop Environment
    auto de = parseResult.table()[DESKTOP_ENVIRONMENT_KEY_NAME].value<std::string>();
    if (de.has_value())
    {
        edition.m_desktopEnvironment = QString(de->c_str());
    }

    // License
    reply = CurrentEditionInterface::license();
    if (!reply.has_value())
    {
        qWarning() << "Edition license not found";
    }
    else
    {
        edition.m_license = reply.value();
    }

    // Logo
    //
    //

    m_product.setEdition(edition);
}

void DataBuilder::rebuildProductDisplayName()
{
    auto reply = SystemInfoInterface::getOperationSystemName();
    if (reply.has_value())
    {
        auto displayName = reply.value().join(" ");
        displayName.replace("ALT", QObject::tr("ALT"))
            .replace("Server", QObject::tr("Server"))
            .replace("Workstation", QObject::tr("Workstation"))
            .replace("Virtualization", QObject::tr("Virtualization"))
            .replace("SP", QObject::tr("SP"))
            .replace("Education", QObject::tr("Education"))
            .replace("Regular", QObject::tr("Regular"))
            .replace("Starterkit", QObject::tr("Starterkit"));

        m_product.setDisplayName(displayName);
    }
    else
    {
        m_product.setDisplayName(QObject::tr("ALT Linux"));
    }
}

void DataBuilder::rebuildBranch()
{
    auto reply = SystemInfoInterface::getBranch();
    if (reply.has_value())
    {
        auto name = reply.value().join(" ");
        auto displayName = name;
        displayName = displayName.replace("sisyphus", QObject::tr("Repository \"Sisyphus\""))
                          .replace("_e2k", " (e2k)")
                          .replace("_riscv64", " (riscv64)")
                          .replace("_loongarch64", " (loongarch64)")
                          .replace("p11", QObject::tr("Eleventh Platform P11 (Salvia)"))
                          .replace("p10", QObject::tr("Tenth Platform P10 (Aronia)"));

        Branch branch{name, displayName};
        m_product.setBranch(branch);
    }
    else
    {
        QString name = "sisyphus";
        m_product.setBranch(Branch{name, QObject::tr("Repository \"Sisyphus\"")});
    }
}

void DataBuilder::rebuildProductLicense()
{
    if (!m_product.edition().has_value() || !m_product.edition()->m_license.has_value())
    {
        auto reply = SystemInfoInterface::getLicense();
        if (reply.has_value())
        {
            m_product.setLicense(reply.value());
        }
    }
}

void DataBuilder::rebuildProductLogo()
{
    std::vector<QString> basealtDistros = {QObject::tr("Server"),
                                           QObject::tr("Workstation"),
                                           QObject::tr("Virtualization"),
                                           QObject::tr("SP"),
                                           QObject::tr("Education"),
                                           QObject::tr("Simply Linux"),
                                           QObject::tr("Starterkit")};

    auto logo = QPixmap();
    auto size = QSize(160, 160);
    if (m_product.edition().has_value() && !m_product.edition()->m_logo.has_value()
        || m_product.displayName().has_value()
               && std::any_of(basealtDistros.begin(),
                              basealtDistros.end(),
                              [name = m_product.displayName()](const QString &distro) {
                                  return name->contains(distro);
                              }))
    {
        m_product.m_isBasealt = true;
        logo = QPixmap(":/logo/assets/basealt.png");
    }
    else
    {
        m_product.m_isBasealt = false;
        logo = QPixmap(":/logo/assets/altlinux.png");
    }
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    logo.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation).save(&buffer, "PNG");
    m_product.setLogo(bArray);
}

std::unique_ptr<QStandardItemModel> DataBuilder::buildPropertyModel(PropertyModelType type)
{
    auto model = std::make_unique<QStandardItemModel>();

    for (auto &&propertyRow : buildProperties(type))
    {
        model->appendRow(propertyRow);
    }

    return model;
}

QString DataBuilder::buildUsefulSources()
{
    auto notes = SystemInfoInterface::getReleaseNotes();
    if (!notes.has_value())
    {
        return QObject::tr("No information available");
    }

    return notes.value();
}

QList<QList<QStandardItem *>> DataBuilder::buildProperties(PropertyModelType type)
{
    auto properties = QList<QList<QStandardItem *>>();

    const QHash<PropertyModelType, QList<QList<QStandardItem *> (*)()>> methods = {
        {
            PropertyModelType::PropertyModelTypeOperationSystem,
            {
                &DataBuilder::buildPropertyOsName,
                &DataBuilder::buildPropertyEdition,
                &DataBuilder::buildPropertyBranch,
                &DataBuilder::buildPropertyArch,
                &DataBuilder::buildPropertyUpdatedOn,
                &DataBuilder::buildPropertyKernel,
            },
        },
        {
            PropertyModelType::PropertyModelTypeHardware,
            {
                &DataBuilder::buildPropertyProcessor,
                &DataBuilder::buildPropertyArch,
                &DataBuilder::buildPropertyGraphics,
                &DataBuilder::buildPropertyMemory,
                &DataBuilder::buildPropertyDrive,
                &DataBuilder::buildPropertyMotherboard,
                &DataBuilder::buildPropertyDisplays,
            },
        },
    };

    for (auto buildProperty : methods[type])
    {
        auto property = buildProperty();
        const auto &propertyNameItem = property[0];
        const auto &propertyValueItem = property[1];
        const auto &propertyName = propertyNameItem->data(Qt::DisplayRole).toString();
        const auto &propertyValue = propertyValueItem->data(Qt::DisplayRole).toString();
        bool isRequiredForDisplay = propertyNameItem->data(Qt::UserRole).toBool();
        if (propertyValue == ErrorMessage::notFound() && !isRequiredForDisplay)
        {
            qWarning() << QString("%1 %2").arg(propertyName).arg(propertyValue);
            IF_DEBUG(properties << property);
        }
        else
        {
            properties << property;
        }
    }

    return properties;
}

QString DataBuilder::buildHostName()
{
    auto reply = SystemInfoInterface::getHostName();
    if (!reply.has_value())
    {
        qWarning() << "Hostname not found";
        return ErrorMessage::notFound();
    }

    return reply.value().join(" ");
}

QList<QStandardItem *> DataBuilder::buildPropertyOsName()
{
    auto items = QList<QStandardItem *>() << buildPropertyName(QObject::tr("Name"));
    if (!m_product.displayName().has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(m_product.displayName().value(), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyEdition()
{
    bool isEditable = !ManagerInterface::getObjects(ALTERATOR_EDITION1_INTERFACE_NAME).isEmpty();
    bool isSetted = m_product.edition().has_value();

    auto items = QList<QStandardItem *>() << buildPropertyName(QObject::tr("Edition"), isSetted || isEditable);

    if (!m_product.edition().has_value())
    {
        return items << buildPropertyValue(ErrorMessage::notFound(), isEditable);
    }

    return items << buildPropertyValue(m_product.edition().has_value()
                                           ? m_product.edition().value().m_displayName.value()
                                           : ErrorMessage::notFound(),
                                       isEditable);
}

QList<QStandardItem *> DataBuilder::buildPropertyBranch()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Branch"));

    if (!m_product.m_branch.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(m_product.m_branch.value().m_displayName, false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyUpdatedOn()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Updated on"));

    auto reply = AptInterface::lastDistUpgrade();
    if (!reply.has_value() || reply.value().size() < 1)
    {
        qWarning() << "Last update not found: try to update the system to receive information";

        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        QDateTime lastUpdateDateTime = QDateTime::fromString(reply.value().first(), "yyyy-MM-dd HH:mm:ss 'UTC'");
        if (!lastUpdateDateTime.isValid())
        {
            items << buildPropertyValue(ErrorMessage::notFound(), false);
        }
        else
        {
            items << buildPropertyValue(QLocale::system().toString(lastUpdateDateTime.toLocalTime(),
                                                                   QLocale::FormatType::NarrowFormat),
                                        false);
        }
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyKernel()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Kernel"));

    auto reply = SystemInfoInterface::getKernel();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(reply.value().join(" "), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyProcessor()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Processor"));

    auto reply = SystemInfoInterface::getCpu();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        quint64 volumeInKHz = reply.value()[2].toULongLong();
        auto frequency = reply.value()[0] + " (" + reply.value()[1] + ") " + " @ "
                         + QString::number(volumeInKHz / 1'000) + " " + FrequencyUnit::megaherz();

        items << buildPropertyValue(std::move(frequency), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyArch()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Arch"));

    auto reply = SystemInfoInterface::getArch();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(reply.value().join(" "), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyGraphics()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Graphics"));

    auto reply = SystemInfoInterface::getGpu();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(reply.value().join(" "), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyMemory()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Memory"));

    auto reply = SystemInfoInterface::getMemory();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        qlonglong volumeInMB = reply.value()[0].toULongLong() / 1000 / 1000;

        auto volume = volumeInMB < 1000 ? QString::number(volumeInMB) + " " + MemoryUnit::megabytes()
                                        : QString::number(volumeInMB / 1000) + " " + MemoryUnit::gigabytes();

        items << buildPropertyValue(std::move(volume), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyDrive()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Drive"));

    auto reply = SystemInfoInterface::getDrive();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        qlonglong volumeInMB = reply.value()[0].toULongLong() / 1000 / 1000;

        auto volume = volumeInMB < 1000 ? QString::number(volumeInMB) + " " + MemoryUnit::megabytes()
                                        : QString::number(volumeInMB / 1000) + " " + MemoryUnit::gigabytes();

        items << buildPropertyValue(std::move(volume), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyMotherboard()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Motherboard"));

    auto reply = SystemInfoInterface::getMotherboard();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(reply.value().join(" "), false);
    }

    return items;
}

QList<QStandardItem *> DataBuilder::buildPropertyDisplays()
{
    auto items = QList<QStandardItem *>();
    items << buildPropertyName(QObject::tr("Displays"));

    auto reply = SystemInfoInterface::getMonitor();
    if (!reply.has_value())
    {
        items << buildPropertyValue(ErrorMessage::notFound(), false);
    }
    else
    {
        items << buildPropertyValue(reply.value().join("; "), false);
    }

    return items;
}

QStandardItem *DataBuilder::buildPropertyName(const QString &name, bool isRequiredForDisplay)
{
    auto propertyName = std::make_unique<QStandardItem>();
    QFont font = propertyName->font();
    font.setBold(true);
    propertyName->setFont(font);
    propertyName->setData(isRequiredForDisplay, Qt::UserRole);
    propertyName->setData(QString("%1:").arg(name), Qt::DisplayRole);

    return propertyName.release();
}

QStandardItem *DataBuilder::buildPropertyValue(const QString &value, bool isEditable)
{
    auto propertyName = std::make_unique<QStandardItem>();
    propertyName->setData(isEditable, Qt::UserRole);
    propertyName->setData(value, Qt::DisplayRole);

    return propertyName.release();
}

const Product &DataBuilder::product()
{
    return m_product;
}
} // namespace alt
