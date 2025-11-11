#pragma once

#include <QtSolutions/QtSingleApplication>
#include <QObject>
#include "AppSettings.h"
#include "data/Parameter.h"
#include "QJsonObject"

class Service;
class Controller;

class ServicesApp : public QtSingleApplication
{
    Q_OBJECT
public:
    ServicesApp(int& argc, char** argv);
    ~ServicesApp();

    int run();

    AppSettings* settings();
    Controller* controller();

    inline static ServicesApp* instance() {
        return (ServicesApp*)QApplication::instance();
    }

    bool notify(QObject*, QEvent*) override;

    struct ParsedParameters {
        Service* service;
        Parameter::Contexts contexts;
        QJsonObject data;
    };
    std::optional<ParsedParameters> importParameters(const QString& fileName);

private:
    class Private;
    Private* d;

};

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<ServicesApp *>(QCoreApplication::instance()))
