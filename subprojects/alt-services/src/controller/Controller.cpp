#include "Controller.h"

#include <QMessageBox>

#include "data/Service.h"

#include <QDebug>
#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtGlobal>

#include <map>
#include <set>

#include "ControllerPrivate.h"

namespace {
class ScopedLogCapture {
public:
    explicit ScopedLogCapture(Controller* controller)
        : m_controller(controller)
    {
        m_out = QObject::connect(controller, &Controller::stdout, controller, [this](const QString& s){ append(s); });
        m_err = QObject::connect(controller, &Controller::stderr, controller, [this](const QString& s){ append(s); });

        // Install a temporary Qt message handler to capture warnings/critical/debug
        s_prevHandler = qInstallMessageHandler(&ScopedLogCapture::qtHandler);
        s_active = this;
    }
    ~ScopedLogCapture()
    {
        QObject::disconnect(m_out);
        QObject::disconnect(m_err);
        // Restore previous message handler
        s_active = nullptr;
        qInstallMessageHandler(s_prevHandler);
    }

    QString logs() const { return m_buffer; }
    bool empty() const { return m_buffer.trimmed().isEmpty(); }

private:
    static void qtHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
    {
        Q_UNUSED(context);
        if (!s_active) return;
        switch (type) {
            case QtDebugMsg:
                // Often filtered by QT_LOGGING_RULES; include when present.
                s_active->append(QString{"[debug] %1"}.arg(msg));
                break;
            case QtInfoMsg:
                s_active->append(QString{"[info] %1"}.arg(msg));
                break;
            case QtWarningMsg:
                s_active->append(QString{"[warn] %1"}.arg(msg));
                break;
            case QtCriticalMsg:
                s_active->append(QString{"[crit] %1"}.arg(msg));
                break;
            case QtFatalMsg:
                s_active->append(QString{"[fatal] %1"}.arg(msg));
                break;
        }
    }

    void append(const QString& s)
    {
        if (!m_buffer.isEmpty()) m_buffer.append('\n');
        m_buffer.append(s);
    }

    Controller* m_controller{nullptr};
    QMetaObject::Connection m_out;
    QMetaObject::Connection m_err;
    QString m_buffer;

    // Global handler state
    inline static QtMessageHandler s_prevHandler = nullptr;
    inline static ScopedLogCapture* s_active = nullptr;
};

static void showLogError(QWidget* parent, const QString& title, const QString& summary, const QString& logs)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(title);
    box.setText(summary);
    // Always provide a Details area; if no logs captured, show a short placeholder.
    box.setDetailedText(!logs.trimmed().isEmpty() ? logs : QObject::tr("No additional logs were captured."));
    box.exec();
}
} // namespace

Controller::Controller()
    : d { new Private(this) }
{
    connect(&d->m_datasource, &DBusProxy::stdout, this, &Controller::stdout);
    connect(&d->m_datasource, &DBusProxy::stderr, this, &Controller::stderr);

    {
        auto* section = new QAction{tr("Tables style"), this};
        section->setSeparator(true);
        auto* compact = new QAction{tr("&Compact"), this};
        auto* detailed = new QAction{tr("&Detailed"), this};
        auto* detailed_multiline = new QAction{tr("&Word wrap"), this};

        d->m_table_actions << section
                           << compact
                           << detailed
                           << detailed_multiline;

        compact->setCheckable(true);
        detailed->setCheckable(true);

        detailed->setChecked(true);
        d->m_table_mode_group.setExclusive(true);
        d->m_table_mode_group.addAction(compact);
        d->m_table_mode_group.addAction(detailed);

        ( ServicesApp::instance()->settings()->tablesDetailed()
                ? detailed
                : compact )->setChecked(true);
        connect(detailed, &QAction::toggled, ServicesApp::instance()->settings(), &AppSettings::set_tablesDetailed);

        detailed_multiline->setCheckable(true);
        detailed_multiline->setChecked(ServicesApp::instance()->settings()->tablesDetailedMultiline());
        connect(detailed, &QAction::toggled, detailed_multiline, &QAction::setEnabled);
        connect(detailed_multiline, &QAction::toggled, ServicesApp::instance()->settings(), &AppSettings::set_tablesDetailedMultiline);
    }
}

Controller::~Controller() { delete d; }

Service* Controller::getByIndex(const QModelIndex& index)
{
    try { return d->m_services.at(index.row()).get(); }
    catch ( std::out_of_range& ) {
        qWarning() << "invalid index";
        return nullptr;
    }
}

Service* Controller::findByName(const QString& name)
{
    for ( const auto& service : d->m_services )
        if ( service->name() == name )
            return service.get();

    qWarning() << "service with name" << name << "not found";
    return nullptr;
}

void Controller::selectByPath(const QString& path)
{
    auto it = std::find_if(d->m_services.cbegin(), d->m_services.cend(),
        [&path](auto& service){ return service->dbusPath() ==  path; }
    );

    if ( it != d->m_services.cend() )
        emit select(std::distance(d->m_services.cbegin(), it));
    else
        qWarning() << "object" << path << "not found";
}


QAbstractItemModel* Controller::model() { return &d->m_model; }

QList<QAction*> Controller::tableActions() { return d->m_table_actions; }

bool Controller::updateStatus(Service* service)
{
    emit beginRefresh();

    QByteArray data;
    int code = d->m_datasource.status(service->dbusPath(), data);
    qDebug() << service->name() << "status:" << data;

    if ( code )
        qDebug() << "non-zero" << service->name() << "Status() exit code!";

    service->setStatus(code, data);

    auto index = std::find_if(
        d->m_services.begin(),
        d->m_services.end(),
        [=](auto& servicePtr){return servicePtr.get() == service;}
    );
    if ( index != d->m_services.end() ) {
        int row = std::distance(d->m_services.begin(), index);
        auto modelIndex = d->m_model.index(row, 1);
        emit d->m_model.dataChanged(modelIndex, modelIndex);
    }

    emit endRefresh();
    return code == 0;
}

bool Controller::call(const Action& action)
{
    auto serializedParameters = QJsonDocument{action.parameters}.toJson(QJsonDocument::Compact).append('\n');
    qDebug() << serializedParameters;

    using DBusMethod = bool(DBusProxy::*)(const QString&, const QString&);
    static const std::map<Parameter::Context, DBusMethod> actions_
    {
        { Parameter::Context::Deploy,    &DBusProxy::deploy    },
        { Parameter::Context::Undeploy,  &DBusProxy::undeploy  },
        { Parameter::Context::Configure, &DBusProxy::configure },
        { Parameter::Context::Backup,    &DBusProxy::backup    },
        { Parameter::Context::Restore,   &DBusProxy::restore   },
    };

    bool success = true;
    ScopedLogCapture capture(this);

    if (action.action == Parameter::Context::Diag)
    {
        /*
         *  For diag-only action logic is simple,
         *  since it does not alter service state
         */

        success = diag(
            action.service,

            action.service->isDeployed()
                ? DiagTool::Test::Mode::PostDeploy
                : DiagTool::Test::Mode::PreDeploy,

            action.options.prediag
                ? action.options.prediagTests
                : action.options.postdiagTests
        );
    }
    else
    {
        /*
         *  Any other action alters service state.
         *  We need to re-read Status() after that.
         *  Also, optional pre/post -diagnostics may be enabled
         */
        try {
            auto& method = actions_.at(action.action);

            emit beginRefresh();

            if ( action.options.prediag )
                success = diag(action.service, DiagTool::Test::Mode::PreDeploy, action.options.prediagTests);

            if ( success )
            {
                emit actionBegin(actionName(action.action));
                bool result = std::invoke( actions_.at(action.action), &d->m_datasource,
                                          action.service->dbusPath(), serializedParameters );
                emit actionEnd(result);
                success = result;
                updateStatus(action.service);
            }

            if ( success && action.options.autostart )
            {
                emit actionBegin(tr("Starting service"));
                emit actionEnd(start(action.service));
            }

            if (success && action.options.postdiag)
                success = diag(action.service, DiagTool::Test::Mode::PostDeploy, action.options.postdiagTests);


            emit endRefresh();
        }
        catch (std::out_of_range&) { qCritical() << "not implemented"; }
    }

    if ( !success ) {
        // Logs are visible in the wizard execution page.
        QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("%1 failed.").arg(actionName(action.action)));
    }

    return success;
}

bool Controller::start(Service* service)
{
    ScopedLogCapture capture(this);

    emit beginRefresh();
    if ( !d->m_datasource.start(service->dbusPath()) )
        showLogError(QApplication::activeWindow(), tr("Error"), tr("Failed to start service"), capture.logs());

    emit endRefresh();

    updateStatus(service);
    return service->isStarted();
}

bool Controller::stop(Service* service)
{
    ScopedLogCapture capture(this);

    emit beginRefresh();
    if ( !d->m_datasource.stop(service->dbusPath()) )
        showLogError(QApplication::activeWindow(), tr("Error"), tr("Failed to stop service"), capture.logs());

    emit endRefresh();

    updateStatus(service);
    return !service->isStarted();
}

bool Controller::diag(Service* service, DiagTool::Test::Mode mode, const Action::TestSet& testSet)
{
    emit beginRefresh();
    emit actionBegin(mode == DiagTool::Test::Mode::PreDeploy ? tr("Premilinary diagnostics") : tr("Post-diagnostics"));

    d->m_datasource.clearEnv();

    for ( const auto& parameter : service->parameters() ) {
        if ( parameter->contexts() & Parameter::Context::Diag ) {
            auto value = parameter->value(
                parameter->isConstant()
                    ? Parameter::ValueScope::Default
                    : Parameter::ValueScope::Edit
                );
            QString env;

            switch ( parameter->valueType() ) {
                case Property::Type::Array:{
                    QJsonDocument doc;
                    doc.setArray(value->serialize().toArray());
                    env = doc.toJson(QJsonDocument::Compact);
                    break;
                }

                case Property::Type::Enum:
                case Property::Type::Composite:{
                    QJsonDocument doc;
                    doc.setObject(value->serialize().toObject());
                    env = doc.toJson(QJsonDocument::Compact);
                    break;
                }

                case Property::Type::Bool:
                case Property::Type::String:
                case Property::Type::Int:
                    env = value->get().toString();
                    break;
            }

            d->m_datasource.setEnv( parameter->name(), env );
        }
    }
    d->m_datasource.setEnv("service_deploy_mode", mode == DiagTool::Test::Mode::PostDeploy ? "post" : "pre");

    std::vector<Parameter*> parameters;
    for ( const auto& parameter : service->parameters() )
        if ( parameter->contexts().testFlag(Parameter::Context::Diag) )
            parameters.push_back(parameter.get());

    bool success = std::accumulate(service->diagTools().cbegin(), service->diagTools().cend(), true,
    [this, mode, &testSet](bool res, const auto& tool) -> bool
    {
        if ( testSet.hasTool(tool.get()) || tool->hasRequiredTests(mode) )
        {
            emit actionBegin(tr("Diagnostic tool \"%1\"").arg(tool->name()));

            bool success = std::accumulate(tool->tests().cbegin(), tool->tests().cend(), true,
            [this, &tool, mode, &testSet](bool res, const auto& test) -> bool
            {
                if (!test->required().testFlag(mode) && !testSet.hasTest(test.get()) )
                    return res;

                emit actionBegin(tr("Running test \"%1\"").arg(test->name()));
                bool result = d->m_datasource.runDiag(tool->path(), test->name(), tool->session());
                emit actionEnd(result);
                return res & result;
            });

            emit actionEnd(success);
            return success & res;
        }
        return res;
    });

    emit actionEnd(success);
    emit endRefresh();
    return success;
}

bool Controller::findConflict(Service* deployService, Resource* deployResource, Service** other, Resource** conflicting)
{
    for ( auto& service : d->m_services ) {
        if ( bool conflict = service->hasConflict(deployService, deployResource, conflicting) ){
            *other = service.get();
            return true;
        }
    }

    return false;
}

const QString& Controller::actionName(Parameter::Context context)
{
    static const std::map<Parameter::Contexts, QString> actionNames{
        { Parameter::Context::Configure , QObject::tr("Configuration")},
        { Parameter::Context::Deploy    , QObject::tr("Deployment")},
        { Parameter::Context::Undeploy  , QObject::tr("Undeployment")},
        { Parameter::Context::Diag      , QObject::tr("Diagnostic")},
        { Parameter::Context::Backup    , QObject::tr("Backup")},
        { Parameter::Context::Restore   , QObject::tr("Restore")},
    };
    return actionNames.at(context);
}

const QIcon& Controller::actionIcon(Parameter::Context context)
{
    static const std::map<Parameter::Contexts, QIcon> actionIcons{
        { Parameter::Context::Configure , QIcon::fromTheme("preferences-system")},
        { Parameter::Context::Deploy    , QIcon::fromTheme("go-up")},
        { Parameter::Context::Undeploy  , QIcon::fromTheme("window-close")},
        { Parameter::Context::Diag      , QIcon::fromTheme("applications-system")},
        { Parameter::Context::Backup    , QIcon::fromTheme("document-save")},
        { Parameter::Context::Restore   , QIcon::fromTheme("edit-undo")},
    };
    return actionIcons.at(context);
}


void Controller::refresh(){
    emit beginRefresh();

    d->m_model.setItems({});
    d->m_services.clear();

    auto paths = d->m_datasource.getServicePaths();

    for ( auto& path : paths ) {
        if ( auto s = d->buildService(path, d->m_datasource.getServiceInfo(path)) )
        {       
            s->setLocale(QLocale::system());
            d->m_services.push_back(std::move(s));
        } else
            qWarning() << "failed to build" << path;
    }

    d->m_model.setItems(d->m_services);
    for ( const auto& s : d->m_services )
        updateStatus(s.get());

    emit endRefresh();
}
