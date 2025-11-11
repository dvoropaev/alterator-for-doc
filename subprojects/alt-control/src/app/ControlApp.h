#pragma once

#include <QApplication>
#include "AppSettings.h"

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<ControlApp *>(QCoreApplication::instance()))

class ControlApp : public QApplication
{
    Q_OBJECT
public:
    ControlApp(int& argc, char** argv);
    ~ControlApp();

    AppSettings& settings();

    int exec();

private:
    class Private;
    Private* d;
};
