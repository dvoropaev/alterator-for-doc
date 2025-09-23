#include "application.h"
#include "controller/controller.h"

namespace alt
{
Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , controller(std::make_unique<Controller>())
{}

Application::~Application() = default;

QLocale Application::getLocale()
{
    return Application::currentLocale;
}

void Application::setLocale(const QLocale &locale)
{
    currentLocale = locale;
}

int Application::exec()
{
    this->controller->init();
    return QApplication::exec();
}
} // namespace alt
