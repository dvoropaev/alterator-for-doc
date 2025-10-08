#include "controller.h"
#include "mainwindow.h"
#include "model/model.h"

#include <memory>
#include <utility>
#include <vector>
#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QLayout>
#include <QLocale>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>

#include <QMessageBox>

namespace ab
{
class ControllerPrivate
{
public:
    std::shared_ptr<MainWindow> window{nullptr};
    std::unique_ptr<model::Model> model{nullptr};
    std::unique_ptr<ao_builder::DataSourceInterface> dataSource{nullptr};
    std::unique_ptr<ao_builder::AOBuilderInterface> modelBuilder{nullptr};

    ControllerPrivate(std::shared_ptr<MainWindow> w,
                      std::unique_ptr<model::Model> m,
                      std::unique_ptr<ao_builder::DataSourceInterface> ds,
                      std::unique_ptr<ao_builder::AOBuilderInterface> mb)
        : window(std::move(w))
        , model(std::move(m))
        , dataSource(std::move(ds))
        , modelBuilder(std::move(mb))
    {}
};

Controller::Controller(std::shared_ptr<MainWindow> w,
                       std::unique_ptr<model::Model> m,
                       std::unique_ptr<ao_builder::DataSourceInterface> ds,
                       std::unique_ptr<ao_builder::AOBuilderInterface> mb,
                       QObject *parent)
    : QObject{parent}
    , d{new ControllerPrivate(std::move(w), std::move(m), std::move(ds), std::move(mb))}
{
    buildModel();
}

Controller::~Controller()
{
    delete d;
}

void Controller::moduleClicked(ao_builder::Object *object)
{
    auto apps = d->model->getApps(object->m_interface);
    if (apps.empty())
    {
        qWarning() << object->m_name << ": no applications are available for this module";
        return;
    }

    auto app = *apps.begin();
    auto proc = new QProcess(this);
    connect(proc, &QProcess::readyReadStandardOutput, this, [app, proc]() {
        qInfo() << QString("%1: ").arg(app->m_name) << proc->readAllStandardOutput();
    });
    connect(proc, &QProcess::readyReadStandardOutput, this, [app, proc]() {
        qWarning() << QString("%1: ").arg(app->m_name) << proc->readAllStandardError();
    });
    connect(proc,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            [this, proc, app](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0)
                {
                    qInfo() << QString("application '%1' run succesfuly").arg(app->m_displayName);
                }
                else
                {
                    qWarning() << QString("application '%1' run failed with code %2")
                                      .arg(app->m_displayName)
                                      .arg(exitCode);
                }
            });
    QString exec = app->m_exec;
    exec.replace("%o", object->m_dbus_path);
    proc->start("/bin/bash", QStringList() << "-c" << exec);
}

void Controller::translateModel()
{
    QLocale locale;
    QString language = locale.system().name().split("_").at(0);
    d->model->translateModel(language);

    d->window->clearUi();
    if (d->model != nullptr)
    {
        d->window->setModel(d->model.get());
    }
}

void Controller::buildModel()
{
    if (!d->dataSource->ping()){
        QMessageBox::critical(d->window.get(), tr("warning"), tr("service did not respond"));
        return;
    }

    d->model->clear();

    auto apps = d->modelBuilder->buildLocalApps();

    std::set<QString> ifaces;
    for ( const auto& app : apps )
        ifaces.insert(app->m_interfaces.begin(), app->m_interfaces.end());

    auto categories = d->modelBuilder->buildCategories();
    auto objects = d->modelBuilder->buildObjects(ifaces);

    d->model->build(std::move(categories), std::move(apps), std::move(objects));
    translateModel();
}

void Controller::switchBack()
{
    auto proc = new QProcess(this);
    proc->startDetached(legacy_app, QStringList());
    d->window.get()->close();
}

} // namespace ab
