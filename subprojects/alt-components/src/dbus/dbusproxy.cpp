#include "dbus/dbusproxy.h"

#include <nlohmann/json.hpp>

#include <QRegularExpression>
#include <QSet>
#include <QTimeZone>

#include "application.h"
#include "constants.h"

namespace alt
{
std::optional<QDate> DBusProxy::getDateResult(const QString &methodName)
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, APT1_PATH, APT1_INTERFACE_NAME, connection);
    auto dbusResult = DBusProxy::callDBus(&iface, methodName);
    if (!dbusResult.has_value())
    {
        return std::nullopt;
    }
    auto result = getResultAndError<QString>(dbusResult.value(), methodName);
    if (!result.has_value())
    {
        return std::nullopt;
    }

    QDateTime lastUpdateDateTime = QDateTime::fromString(result.value(), "yyyy-MM-dd HH:mm:ss 'UTC'");
    if (!lastUpdateDateTime.isValid())
    {
        return std::nullopt;
    }
    return lastUpdateDateTime.toLocalTime().date();
}

std::optional<QString> DBusProxy::checkSuccess(const QVariantList &args, const QString &methodName)
{
    const auto &supposedExitCode = args.last();
    if (!supposedExitCode.canConvert<int>())
    {
        auto message = QString("Return type of \"%1\" doesn't match expected signature.").arg(methodName);
        qWarning() << message;
        emit errorOccured(QtMsgType::QtWarningMsg, message);
        return message;
    }
    auto exitCode = supposedExitCode.toInt();

    if (exitCode != 0)
    {
        auto message = QString("Error from backend method \"%1\": exit code: %2").arg(methodName).arg(exitCode);
        qWarning().noquote() << message;
        emit errorOccured(QtMsgType::QtWarningMsg, message);
        return message;
    }
    return std::nullopt;
}

template<typename T>
std::optional<T> DBusProxy::getResultAndError(const QVariantList &args, const QString &methodName)
{
    auto wrongSigWriteoff = [this, methodName]() {
        auto warning = QString("Return type of \"%1\" doesn't match expected signature.").arg(methodName);
        emit errorOccured(QtMsgType::QtInfoMsg, warning);
        return std::nullopt;
    };

    if (args.size() != 3)
    {
        return wrongSigWriteoff();
    }
    const auto &error = args[1];
    const auto &result = args[0];
    if (!error.canConvert<QStringList>() || !result.canConvert<QStringList>())
    {
        return wrongSigWriteoff();
    }

    if (checkSuccess(args, methodName).has_value())
    {
        auto message = QObject::tr("Backend error: \"%1\"").arg(args[1].toString());
        qWarning().noquote() << message;
        emit errorOccured(QtMsgType::QtInfoMsg, message);
        return std::nullopt;
    }

    return args[0].value<T>();
}

void DBusProxy::resetManagerLocale()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME,
                         ALTERATOR_MANAGER_PATH,
                         ALTERATOR_MANAGER_INTERFACE_NAME,
                         connection);
    const auto locale = Application::getLocale().name() + ".UTF-8";
    this->callDBus(&iface, ALTERATOR_MANAGER_SET_ENV_METHOD_NAME, "LC_ALL", locale);
}

QString DBusProxy::getCurrentEditionInfo()
{
    QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                         QString(GLOBAL_PATH),
                         QString(CURRENT_EDITION_INTERFACE_NAME),
                         connection};
    auto dbusReply = callDBus(&iface, CURRENT_EDITION_INFO_METHOD_NAME);
    if (!dbusReply.has_value())
    {
        return "";
    }
    return dbusReply.value()[0].toString();
}

std::optional<QStringList> DBusProxy::getBatchComponentsInfo()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, GLOBAL_PATH, BATCH_COMPONENTS_INERFACE_NAME, connection);
    return getResult<QStringList>(callDBus(&iface, COMPONENT_INFO_METHOD_NAME), COMPONENT_INFO_METHOD_NAME);
}

std::optional<QSet<QString>> DBusProxy::getBatchComponentsStatus()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, GLOBAL_PATH, BATCH_COMPONENTS_INERFACE_NAME, connection);
    auto dbusResult = getResult<QString>(callDBus(&iface, COMPONENT_STATUS_METHOD_NAME), COMPONENT_STATUS_METHOD_NAME);
    if (!dbusResult.has_value())
    {
        return std::nullopt;
    }

    const auto notInstalledPackages = dbusResult.value().split('\n');
    return QSet(notInstalledPackages.begin(), notInstalledPackages.end());
}

QString DBusProxy::getComponentDescription(const QString &path)
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, path, COMPONENT1_INTERFACE_NAME, connection);
    auto reply = getResult<QString>(callDBus(&iface, COMPONENT_DESCRIPTION_METHOD_NAME),
                                    COMPONENT_DESCRIPTION_METHOD_NAME);
    return {reply.value_or("")};
}

QString DBusProxy::getCategoryDescription(const QString &category)
{
    // NOTE(sheriffkorov): idk when such the feature will be available
    // QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME, GLOBAL_PATH, COMPONENT_CATEGORIES_INTERFACE_NAME, connection);
    // auto reply = getResult<QString>(callDBus(&iface, COMPONENT_DESCRIPTION_METHOD_NAME, category),
    //                                 COMPONENT_DESCRIPTION_METHOD_NAME);
    //
    // return {reply.value_or("")};
    std::ignore = category;
    return {};
}

std::optional<QStringList> DBusProxy::getBatchCategoriesInfo()
{
    QDBusInterface iface(ALTERATOR_MANAGER_SERVICE_NAME,
                         GLOBAL_PATH,
                         BATCH_COMPONENT_CATEGORIES_INERFACE_NAME,
                         connection);
    return getResult<QStringList>(callDBus(&iface, COMPONENT_INFO_METHOD_NAME), COMPONENT_INFO_METHOD_NAME);
}

QString DBusProxy::getCurrentArch()
{
    static std::optional<QString> arch = std::nullopt;

    if (!arch.has_value())
    {
        QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                             QString(SYSTEMINFO_PATH),
                             QString(SYSTEMINFO_INTERFACE_NAME),
                             connection};

        auto dbusReply = callDBus(&iface, SYSTEMINFO_ARCH_METHOD_NAME);
        if (!dbusReply.has_value())
        {
            qWarning() << "Could not get current arch";
            arch = "";
            return arch.value();
        }

        arch = dbusReply.value()[0].toString();
    }

    return arch.value();
}

QString DBusProxy::getCurrentKernelFlavour()
{
    static std::optional<QString> kflavour = std::nullopt;

    const auto *const fallback = "6.12";

    if (!kflavour.has_value())
    {
        QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                             QString(SYSTEMINFO_PATH),
                             QString(SYSTEMINFO_INTERFACE_NAME),
                             connection};

        auto dbusReply = callDBus(&iface, SYSTEMINFO_KERNEL_METHOD_NAME);
        if (!dbusReply.has_value())
        {
            qWarning() << "Could not get current kflavour";
            kflavour = fallback;
            return kflavour.value();
        }

        const auto kernelVersion = dbusReply.value()[0].toString();
        QRegularExpression extractKFlavour("[^-]*-(.*)-[^-]*");
        const auto match = extractKFlavour.match(kernelVersion).captured(1);
        if (match.isEmpty())
        {
            qWarning() << "Empty or no kflavour, will fall back to" << fallback << "as kflavour";
            kflavour = fallback;
            return kflavour.value();
        }
        kflavour = match;
    }

    return kflavour.value();
}

QSet<QString> DBusProxy::getCurrentDesktopEnvironments()
{
    QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                         QString(SYSTEMINFO_PATH),
                         QString(SYSTEMINFO_INTERFACE_NAME),
                         connection};

    auto dbusReply = callDBus(&iface, SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_NAME);
    if (!dbusReply.has_value())
    {
        qWarning() << "Could not get current locale";
        return {};
    }

    const auto list = dbusReply.value()[0].toStringList();
    return {list.begin(), list.end()};
}

QString DBusProxy::getCurrentLanguage()
{
    QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                         QString(SYSTEMINFO_PATH),
                         QString(SYSTEMINFO_INTERFACE_NAME),
                         connection};

    auto dbusReply = callDBus(&iface, SYSTEMINFO_LOCALE_METHOD_NAME);
    if (!dbusReply.has_value())
    {
        qWarning() << "Could not get current locale";
        return "";
    }

    const auto locale = dbusReply.value()[0].toString();
    return locale.split('_').first();
}

const FilterOptions &DBusProxy::getFilterOptions()
{
    static std::optional<FilterOptions> options = std::nullopt;

    if (!options.has_value())
    {
        options = {
            .kflavour = getCurrentKernelFlavour(),
            .arch = getCurrentArch(),
            .desktops = getCurrentDesktopEnvironments(),
            .language = getCurrentLanguage(),
        };
    }

    return options.value();
}

PackageTransactionInfo DBusProxy::getActualPackageListInTransaction(std::set<QString> packagesOnInstallation,
                                                                    const std::set<QString> &packagesOnRemoval)
{
    QDBusInterface iface{QString(ALTERATOR_MANAGER_SERVICE_NAME),
                         QString(APT1_PATH),
                         QString(APT1_INTERFACE_NAME),
                         connection};

    std::for_each(packagesOnRemoval.begin(), packagesOnRemoval.end(), [&packagesOnInstallation](const QString &item) {
        packagesOnInstallation.insert(QString("%1-").arg(item));
    });

    QString packageList = std::accumulate(std::next(packagesOnInstallation.begin()),
                                          packagesOnInstallation.end(),
                                          *packagesOnInstallation.begin(),
                                          [](const QString &prev, const QString &next) {
                                              return QString("%1 %2").arg(prev).arg(next);
                                          });

    auto dbusReply = callDBus(&iface, APT1_CHECK_APPLY_METHOD_NAME, packageList);
    if (!dbusReply.has_value())
    {
        qWarning() << "Could not get package list on installation";
        return PackageTransactionInfo{.exitCode = -1,
                                      .onInstallation = {},
                                      .onRemoval = {},
                                      .onExtraRemoval = {},
                                      .error = QStringList() << "Could not get package lists"};
    }

    if (dbusReply->last().toBool())
    {
        return PackageTransactionInfo{.exitCode = dbusReply->last().toInt(),
                                      .onInstallation = {},
                                      .onRemoval = {},
                                      .onExtraRemoval = {},
                                      .error = dbusReply->at(1).toStringList()};
    }
    auto jsonArrayToQStringSet = [](const nlohmann::json &jsonArray) {
        std::set<QString> result;
        for (const auto &item : jsonArray)
        {
            result.insert(QString::fromStdString(item.get<std::string>()));
        }
        return result;
    };

    QString jsonString = dbusReply.value()[0].toString();
    nlohmann::json response = nlohmann::json::parse(jsonString.toStdString());
    auto listOnInstallation = jsonArrayToQStringSet(response["install_packages"]);
    auto listOnRemoval = jsonArrayToQStringSet(response["remove_packages"]);
    auto listOnExtraRemoval = jsonArrayToQStringSet(response["extra_remove_packages"]);

    return PackageTransactionInfo{.exitCode = 0,
                                  .onInstallation = std::set(listOnInstallation.begin(), listOnInstallation.end()),
                                  .onRemoval = std::set(listOnRemoval.begin(), listOnRemoval.end()),
                                  .onExtraRemoval = std::set(listOnExtraRemoval.begin(), listOnExtraRemoval.end()),
                                  .error = {}};
}
} // namespace alt
