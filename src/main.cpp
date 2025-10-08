#include "app/application.h"

#include <memory>

int main(int argc, char *argv[])
{
    auto app = std::make_unique<alt::Application>(argc, argv);

    return app->exec();
}
