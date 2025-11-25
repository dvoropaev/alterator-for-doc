#include "dbusmanager.h"
#include "constants.h"
#include "utility/scopeguard.h"

#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include <unordered_set>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDateTime>
#include <QRegularExpression>

namespace alt
{
class QtCallbackHelper : public QObject
{
    Q_OBJECT

public:
    Callback callback;

public:
    void setCallback(const Callback &callback) { this->callback = callback; }

public slots:
    void call(QString sig)
    {
        if (callback)
        {
            callback(sig.toUtf8().toStdString());
        }
    }

public:
    ~QtCallbackHelper() override = default;
};

#include "dbusmanager.moc"

class DBusManager::Private
{
public:
    Private();

public:
    static tl::expected<QDBusInterface *, Error> validateInterface(QDBusInterface *iface);
    static tl::expected<QDBusMessage, Error> checkError(const QDBusMessage &reply);
    template<class... Args>
    static tl::expected<QVariantList, Error> call(const std::string &expectedReplySignature,
                                                  QDBusInterface *interface,
                                                  const std::string &method,
                                                  Args... args);
    tl::expected<std::unordered_set<std::string>, Error> getAllHelper(const std::string &interface);
    tl::expected<std::string, Error> getCurrentArch();
    tl::expected<std::string, Error> getCurrentKernelFlavour();
    tl::expected<std::set<std::string>, Error> getCurrentDesktopEnvironments();
    tl::expected<std::string, Error> getCurrentLanguage();
    tl::expected<std::chrono::system_clock::time_point, Error> getDate(const std::string &method,
                                                                       const std::string &replySignature);

public:
    std::shared_ptr<ILogger> logger = nullptr;
    QDBusConnection connection = QDBusConnection::systemBus();
    QtCallbackHelper receiver;
};

void DBusManager::setLogger(const std::shared_ptr<ILogger> &logger)
{
    p->logger = logger;
}

std::string DBusManager::Error::text() const
{
    std::ostringstream errorTextBuilder;
    if (type == Type::InvalidInterface)
    {
        errorTextBuilder << "No interface \"" << interface << "\" on object \"" << object << "\" of service \""
                         << service << "\"";
    }
    else if (type == Type::Reply)
    {
        errorTextBuilder << "Error reply from service \"" << service << "\" with method \"" << interface << "."
                         << method << "\" on object \"" << object << "\": " << data.at(dbus::ERROR_REPLY_KEY_DATA);
    }
    else if (type == Type::InvalidSignature)
    {
        errorTextBuilder << "Return type of \"" << interface << "." << method
                         << "\" does not match expected signature.\n"
                         << "Expected: (" << data.at(dbus::ERROR_INVALID_SIGNATURE_KEY_EXPECTED) << ").\n"
                         << "Got: (" << data.at(dbus::ERROR_INVALID_SIGNATURE_KEY_GOT) << ").";
    }
    else if (type == Type::Server)
    {
        errorTextBuilder << "Backend error from method \"" << interface << "." << method << "\" on object \"" << object
                         << "\" of service \"" << service << "\":\n"
                         << (code ? ("Exit code: " + std::to_string(code.value()) + ".\n") : "")
                         << "Details: " << data.at(dbus::ERROR_SERVER_KEY_DATA);
    }
    return errorTextBuilder.str();
}

DBusManager::DBusManager()
    : p(std::make_unique<DBusManager::Private>())
{}

DBusManager::~DBusManager() = default;

tl::expected<void, DBusManager::Error> DBusManager::setLocale(const std::string &locale)
{
    using Result = decltype(setLocale(locale));
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::ALTERATOR_MANAGER_PATH,
                         dbus::ALTERATOR_MANAGER_INTERFACE_NAME,
                         p->connection);
    return p
        ->call(dbus::ALTERATOR_MANAGER_SET_ENV_METHOD_REPLY_SIGNATURE,
               &iface,
               dbus::ALTERATOR_MANAGER_SET_ENV_METHOD_NAME,
               "LC_ALL",
               QString::fromStdString(locale))
        .map([](const QVariantList &) -> void {})
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::string, DBusManager::Error> DBusManager::getCurrentEdition()
{
    using Result = decltype(getCurrentEdition());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::GLOBAL_PATH,
                         dbus::CURRENT_EDITION_INTERFACE_NAME,
                         p->connection);
    return p->call(dbus::CURRENT_EDITION_INFO_METHOD_REPLY_SIGNATURE, &iface, dbus::CURRENT_EDITION_INFO_METHOD_NAME)
        .and_then(+[](const QVariantList &args) -> Result {
            const auto code = args.last().toInt();
            if (code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::CURRENT_EDITION_INTERFACE_NAME,
                    .object = dbus::GLOBAL_PATH,
                    .method = dbus::CURRENT_EDITION_INFO_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            return args.first().toByteArray().toStdString();
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::string, DBusManager::Error> DBusManager::getComponentDescription(const std::string &path)
{
    using Result = decltype(getComponentDescription(path));
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         QString::fromStdString(path),
                         dbus::COMPONENT1_INTERFACE_NAME,
                         p->connection);
    return p->call(dbus::COMPONENT_DESCRIPTION_METHOD_REPLY_SIGNATURE, &iface, dbus::COMPONENT_DESCRIPTION_METHOD_NAME)
        .and_then([&path](const QVariantList &args) -> Result {
            const auto code = args.last().toInt();
            if (code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::COMPONENT1_INTERFACE_NAME,
                    .object = path,
                    .method = dbus::COMPONENT_DESCRIPTION_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            return args.first().toByteArray().toStdString();
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::string, DBusManager::Error> DBusManager::getCategoryDescription(const std::string &name)
{
    std::ignore = name;
    return "";
}

tl::expected<EntityDataResponse, DBusManager::Error> DBusManager::getAll()
{
    using Result = decltype(getAll());
    return p->getAllHelper(dbus::BATCH_COMPONENTS_INTERFACE_NAME)
        .and_then([this](std::unordered_set<std::string> &&components) -> Result {
            return p->getAllHelper(dbus::BATCH_COMPONENT_CATEGORIES_INERFACE_NAME)
                .and_then([&components](std::unordered_set<std::string> &&categories) -> Result {
                    return EntityDataResponse{
                        .components = std::move(components),
                        .categories = std::move(categories),
                    };
                });
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Critical, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::set<std::string>, DBusManager::Error> DBusManager::getInstalledPackages()
{
    using Result = decltype(getInstalledPackages());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME, dbus::RPM_PATH, dbus::RPM1_INTERFACE_NAME, p->connection);
    return p->call(dbus::RPM1_LIST_METHOD_REPLY_SIGNATURE, &iface, dbus::RPM1_LIST_METHOD_NAME)
        .and_then(+[](const QVariantList &args) -> Result {
            const auto code = args.last().toInt();
            if (code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::RPM1_INTERFACE_NAME,
                    .object = dbus::RPM_PATH,
                    .method = dbus::RPM1_LIST_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            const auto list = args.first().toStringList();
            std::set<std::string> packages;
            std::transform(list.begin(), list.end(), std::inserter(packages, packages.begin()), [](const QString &item) {
                return item.split(' ')[0].toUtf8().toStdString();
            });
            return packages;
        });
}

tl::expected<void, DBusManager::Error> DBusManager::updateSources(const Callback &callback)
{
    using Result = decltype(updateSources(callback));
    p->receiver.setCallback(callback);
    ScopeGuard guard([this] { p->receiver.setCallback(nullptr); });

    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME, dbus::APT_PATH, dbus::APT1_INTERFACE_NAME, p->connection);
    constexpr auto dayInSeconds = 24 * 60 * 60 * 1000;
    iface.setTimeout(dayInSeconds);

    return p->call(dbus::APT1_UPDATE_METHOD_REPLY_SIGNATURE, &iface, dbus::APT1_UPDATE_METHOD_NAME)
        .and_then(+[](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = dbus::APT1_UPDATE_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "execution failure"}},
                });
            }
            return {};
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Error, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<TransactionResolutionResponse, DBusManager::Error> DBusManager::checkApply(
    std::unordered_set<std::string> install, const std::unordered_set<std::string> &remove)
{
    using Result = decltype(checkApply(install, remove));
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME, dbus::APT_PATH, dbus::APT1_INTERFACE_NAME, p->connection);

    std::transform(remove.begin(), remove.end(), std::inserter(install, install.begin()), [](const std::string &item) {
        return item + "-";
    });
    QString applyList = std::accumulate(
        install.begin(), install.end(), QString(), +[](const QString &acc, const std::string &name) {
            return acc + " " + QString::fromStdString(name);
        });

    return p->call(dbus::APT1_CHECK_APPLY_METHOD_REPLY_SIGNATURE, &iface, dbus::APT1_CHECK_APPLY_METHOD_NAME, applyList)
        .and_then(+[](const QVariantList &args) -> Result {
            const int code = args.last().toInt();
            if (code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = dbus::APT1_CHECK_APPLY_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, args.at(1).toStringList().join("\n").toUtf8().toStdString()}},
                });
            }

            nlohmann::json response = nlohmann::json::parse(args.at(0).toString().toUtf8().toStdString());
            auto array = response["install_packages"];
            auto listOnInstallation = std::unordered_set<std::string>(array.begin(), array.end());
            array = response["remove_packages"];
            auto listOnRemoval = std::unordered_set<std::string>(array.begin(), array.end());
            array = response["extra_remove_packages"];
            auto listOnExtraRemoval = std::unordered_set<std::string>(array.begin(), array.end());
            return TransactionResolutionResponse{
                .onInstallation = std::move(listOnInstallation),
                .onRemoval = std::move(listOnRemoval),
                .onExtraRemoval = std::move(listOnExtraRemoval),
            };
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Error, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<void, DBusManager::Error> DBusManager::apply(std::unordered_set<std::string> install,
                                                          const std::unordered_set<std::string> &remove,
                                                          const std::unordered_set<std::string> &exclude,
                                                          const Callback &callback)
{
    using Result = decltype(apply(install, remove, exclude, callback));
    p->receiver.setCallback(callback);
    ScopeGuard guard([this] { p->receiver.setCallback(nullptr); });

    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME, dbus::APT_PATH, dbus::APT1_INTERFACE_NAME, p->connection);
    constexpr auto dayInSeconds = 24 * 60 * 60 * 1000;
    iface.setTimeout(dayInSeconds);

    std::transform(remove.begin(), remove.end(), std::inserter(install, install.begin()), [](const std::string &item) {
        return item + "-";
    });
    QString applyList = std::accumulate(
        install.begin(), install.end(), QString(), +[](const QString &acc, const std::string &name) {
            return acc + " " + QString::fromStdString(name);
        });
    QString excludeList = "\""
                          + std::accumulate(
                              exclude.begin(),
                              exclude.end(),
                              QString(),
                              +[](const QString &acc, const std::string &name) {
                                  return acc + " " + QString::fromStdString(name);
                              })
                          + "\"";

    return p->call(dbus::APT1_APPLY_METHOD_REPLY_SIGNATURE, &iface, dbus::APT1_APPLY_METHOD_NAME, excludeList, applyList)
        .and_then(+[](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = dbus::APT1_APPLY_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "execution failure"}},
                });
            }
            return {};
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Error, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::set<std::string>, DBusManager::Error> DBusManager::getStatus()
{
    using Result = decltype(getStatus());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::GLOBAL_PATH,
                         dbus::BATCH_COMPONENTS_INTERFACE_NAME,
                         p->connection);
    return p->call(dbus::BATCH_STATUS_METHOD_REPLY_SIGNATURE, &iface, dbus::BATCH_STATUS_METHOD_NAME)
        .and_then(+[](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = dbus::APT1_APPLY_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }

            std::set<std::string> lines;
            std::istringstream ss(args.first().toByteArray().toStdString());
            std::string line;
            while (std::getline(ss, line))
            {
                lines.insert(std::move(line));
            }
            return lines;
        })
        .or_else([this](Error &&error) -> Result {
            p->logger->write(ILogger::Level::Error, error.text());
            return tl::unexpected(std::move(error));
        });
}

const Component::Package::FilterOptions &DBusManager::getFilterOptions()
{
    static std::optional<Component::Package::FilterOptions> options = std::nullopt;

    if (!options.has_value())
    {
        options = {
            .kflavour = p->getCurrentKernelFlavour().value_or(""),
            .arch = p->getCurrentArch().value_or(""),
            .desktops = p->getCurrentDesktopEnvironments().value_or(std::set<std::string>()),
            .language = p->getCurrentLanguage().value_or(""),
        };
    }

    return options.value();
}

tl::expected<std::chrono::system_clock::time_point, DBusManager::Error> DBusManager::lastUpdateOfSources()
{
    return p->getDate(dbus::APT1_LAST_UPDATE_METHOD_NAME, dbus::APT1_LAST_UPDATE_METHOD_REPLY_SIGNATURE);
}

tl::expected<std::chrono::system_clock::time_point, DBusManager::Error> DBusManager::lastDistUpgrade()
{
    return p->getDate(dbus::APT1_LAST_UPGRADE_METHOD_NAME, dbus::APT1_LAST_UPGRADE_METHOD_REPLY_SIGNATURE);
}

// -- ---------------------- --
// -- PRIVATE IMPLEMENTATION --
// -- ---------------------- --

DBusManager::Private::Private()
{
    auto signalSuffix = connection.baseService().replace(':', '_').replace('.', '_');
    static const auto aptSignals = {
        dbus::APT_UPDATE_STDOUT_SIGNAL_NAME,
        dbus::APT_UPDATE_STDERR_SIGNAL_NAME,
        dbus::APT_INSTALL_STDOUT_SIGNAL_NAME,
        dbus::APT_INSTALL_STDERR_SIGNAL_NAME,
    };

    for (const auto &signal : aptSignals)
    {
        auto sig = signal + signalSuffix;
        connection.connect(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                           dbus::APT_PATH,
                           dbus::APT1_INTERFACE_NAME,
                           sig,
                           &receiver,
                           SLOT(call(QString)));
    }
}

template<class... Args>
tl::expected<QVariantList, DBusManager::Error> DBusManager::Private::call(const std::string &expectedReplySignature,
                                                                          QDBusInterface *interface,
                                                                          const std::string &method,
                                                                          Args... args)
{
    return validateInterface(interface)
        .map([&](QDBusInterface *iface) { return iface->call(QString::fromStdString(method), args...); })
        .and_then(static_cast<decltype(&Private::checkError)>(checkError))
        .and_then([&expectedReplySignature](const QDBusMessage &reply) -> tl::expected<QDBusMessage, Error> {
            if (reply.signature() != QString::fromStdString(expectedReplySignature))
            {
                return tl::unexpected(Error{
                    .type = Error::Type::InvalidSignature,
                    .service = reply.service().toUtf8().toStdString(),
                    .interface = reply.interface().toUtf8().toStdString(),
                    .object = reply.path().toUtf8().toStdString(),
                    .method = reply.member().toUtf8().toStdString(),
                    .code = std::nullopt,
                    .data = {{dbus::ERROR_INVALID_SIGNATURE_KEY_EXPECTED, expectedReplySignature},
                             {dbus::ERROR_INVALID_SIGNATURE_KEY_GOT, reply.signature().toUtf8().toStdString()}},
                });
            }
            return reply;
        })
        .and_then(+[](const QDBusMessage &reply) -> tl::expected<QVariantList, Error> { return reply.arguments(); });
}

tl::expected<QDBusInterface *, DBusManager::Error> DBusManager::Private::validateInterface(QDBusInterface *iface)
{
    if (!iface->isValid())
    {
        return tl::unexpected(Error{
            .type = Error::Type::InvalidInterface,
            .service = iface->service().toUtf8().toStdString(),
            .interface = iface->interface().toUtf8().toStdString(),
            .object = iface->objectName().toUtf8().toStdString(),
            .method = {},
            .code = std::nullopt,
            .data = {},
        });
    }
    return iface;
}

tl::expected<QDBusMessage, DBusManager::Error> DBusManager::Private::checkError(const QDBusMessage &reply)
{
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        return tl::unexpected(Error{
            .type = Error::Type::Reply,
            .service = reply.service().toUtf8().toStdString(),
            .interface = reply.interface().toUtf8().toStdString(),
            .object = reply.path().toUtf8().toStdString(),
            .method = reply.member().toUtf8().toStdString(),
            .code = std::nullopt,
            .data = {{
                dbus::ERROR_REPLY_KEY_DATA,
                reply.errorMessage().toStdString(),
            }},
        });
    }
    return reply;
}

tl::expected<std::unordered_set<std::string>, DBusManager::Error> DBusManager::Private::getAllHelper(
    const std::string &interface)
{
    using Result = decltype(getAllHelper(interface));
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::GLOBAL_PATH,
                         QString::fromStdString(interface),
                         connection);
    return call(dbus::BATCH_INFO_METHOD_REPLY_SIGNATURE, &iface, dbus::BATCH_INFO_METHOD_NAME)
        .and_then([&interface](const QVariantList &args) -> Result {
            const auto code = args.last().toInt();
            if (code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = interface,
                    .object = dbus::GLOBAL_PATH,
                    .method = dbus::BATCH_INFO_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            std::unordered_set<std::string> result;
            const auto list = args.first().toStringList();
            std::transform(list.begin(), list.end(), std::inserter(result, result.begin()), [](const QString &item) {
                return item.toUtf8().toStdString();
            });
            return result;
        });
}

tl::expected<std::string, DBusManager::Error> DBusManager::Private::getCurrentArch()
{
    using Result = decltype(getCurrentArch());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::SYSTEMINFO_PATH,
                         dbus::SYSTEMINFO_INTERFACE_NAME,
                         connection);
    return call(dbus::SYSTEMINFO_ARCH_METHOD_REPLY_SIGNATURE, &iface, dbus::SYSTEMINFO_ARCH_METHOD_NAME)
        .and_then([](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::SYSTEMINFO_INTERFACE_NAME,
                    .object = dbus::SYSTEMINFO_PATH,
                    .method = dbus::SYSTEMINFO_ARCH_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            return args.first().toStringList().join("\n").toUtf8().toStdString();
        })
        .or_else([this](Error &&error) -> Result {
            logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::string, DBusManager::Error> DBusManager::Private::getCurrentKernelFlavour()
{
    using Result = decltype(getCurrentKernelFlavour());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::SYSTEMINFO_PATH,
                         dbus::SYSTEMINFO_INTERFACE_NAME,
                         connection);
    return call(dbus::SYSTEMINFO_KERNEL_METHOD_REPLY_SIGNATURE, &iface, dbus::SYSTEMINFO_KERNEL_METHOD_NAME)
        .and_then([](const QVariantList &args) -> tl::expected<std::string, Error> {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::SYSTEMINFO_INTERFACE_NAME,
                    .object = dbus::SYSTEMINFO_PATH,
                    .method = dbus::SYSTEMINFO_KERNEL_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }

            const auto kernelVersion = args.first().toStringList().join("\n");
            const auto match = QRegularExpression("[^-]*-(.*)-[^-]*").match(kernelVersion).captured(1);
            if (match.isEmpty())
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = dbus::APT1_APPLY_METHOD_NAME,
                    .code = std::nullopt,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            return match.toUtf8().toStdString();
        })
        .or_else([this](Error &&error) -> Result {
            logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::set<std::string>, DBusManager::Error> DBusManager::Private::getCurrentDesktopEnvironments()
{
    using Result = decltype(getCurrentDesktopEnvironments());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::SYSTEMINFO_PATH,
                         dbus::SYSTEMINFO_INTERFACE_NAME,
                         connection);
    return call(dbus::SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_REPLY_SIGNATURE,
                &iface,
                dbus::SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_NAME)
        .and_then([](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::SYSTEMINFO_INTERFACE_NAME,
                    .object = dbus::SYSTEMINFO_PATH,
                    .method = dbus::SYSTEMINFO_DESKTOP_ENVIRONMENT_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }

            std::set<std::string> result;
            const auto list = args.first().toStringList();
            std::transform(list.begin(), list.end(), std::inserter(result, result.begin()), [](const QString &item) {
                return item.toUtf8().toStdString();
            });
            return result;
        })
        .or_else([this](Error &&error) -> Result {
            logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}
tl::expected<std::string, DBusManager::Error> DBusManager::Private::getCurrentLanguage()
{
    using Result = decltype(getCurrentLanguage());
    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                         dbus::SYSTEMINFO_PATH,
                         dbus::SYSTEMINFO_INTERFACE_NAME,
                         connection);
    return call(dbus::SYSTEMINFO_LOCALE_METHOD_REPLY_SIGNATURE, &iface, dbus::SYSTEMINFO_LOCALE_METHOD_NAME)
        .and_then([](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::SYSTEMINFO_INTERFACE_NAME,
                    .object = dbus::SYSTEMINFO_PATH,
                    .method = dbus::SYSTEMINFO_LOCALE_METHOD_NAME,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, "not found"}},
                });
            }
            return args.first().toStringList().join("\n").split("_").first().toUtf8().toStdString();
        })
        .or_else([this](Error &&error) -> Result {
            logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}

tl::expected<std::chrono::system_clock::time_point, DBusManager::Error> DBusManager::Private::getDate(
    const std::string &method, const std::string &replySignature)
{
    using Result = decltype(getDate(method, replySignature));

    QDBusInterface iface(dbus::ALTERATOR_MANAGER_SERVICE_NAME, dbus::APT_PATH, dbus::APT1_INTERFACE_NAME, connection);
    return call(replySignature, &iface, method)
        .and_then([&method](const QVariantList &args) -> Result {
            if (const int code = args.last().toInt(); code != 0)
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = method,
                    .code = code,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA, args.at(1).toString().toStdString()}},
                });
            }
            QString format = "yyyy-MM-dd HH:mm:ss 'UTC'";
            QDateTime lastUpdateDateTime = QDateTime::fromString(args.first().toStringList().first(), format);
            if (!lastUpdateDateTime.isValid())
            {
                return tl::unexpected(Error{
                    .type = Error::Type::Server,
                    .service = dbus::ALTERATOR_MANAGER_SERVICE_NAME,
                    .interface = dbus::APT1_INTERFACE_NAME,
                    .object = dbus::APT_PATH,
                    .method = method,
                    .code = 255,
                    .data = {{dbus::ERROR_SERVER_KEY_DATA,
                              std::string("invalid date time format: expected ") + format.toStdString()}},
                });
            }
            qint64 ms = lastUpdateDateTime.toMSecsSinceEpoch();
            return std::chrono::system_clock::time_point(std::chrono::milliseconds(ms));
        })
        .or_else([this](Error &&error) -> Result {
            logger->write(ILogger::Level::Warning, error.text());
            return tl::unexpected(std::move(error));
        });
}
} // namespace alt
