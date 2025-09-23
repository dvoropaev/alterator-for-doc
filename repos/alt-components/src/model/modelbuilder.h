#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "model/model.h"
#include "model/objects/category.h"
#include "model/objects/component.h"
#include "model/objects/edition.h"

#define TOML_EXCEPTIONS 0
#include <toml++/toml.h>
#include <unordered_map>
#include <QDBusConnection>
#include <QSet>

namespace alt
{
class Controller;
class ModelBuilder : public QObject
{
    Q_OBJECT

public:
    enum class BuildStatus
    {
        Building,
        Done
    };

public:
    explicit ModelBuilder(Controller *ctrl);

    void buildBySections(Model *model, bool hard = true);
    void buildByTags(Model *model, bool hard = true);
    void buildPlain(Model *model, bool hard = true);
    std::unique_ptr<Edition> buildEdition();

signals:
    void buildStarted();
    void buildDone(Edition *current_edition, int numberOfComponentsBuilt, int editionComponentsBuilt);

private:
    Controller *controller;
};
} // namespace alt

#endif // MODELBUILDER_H
