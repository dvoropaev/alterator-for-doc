#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "model/model.h"
#include "model/objects/edition.h"

#define TOML_EXCEPTIONS 0
#include <toml++/toml.h>
#include <QDBusConnection>
#include <QSet>

namespace alt
{
class Controller;
class ModelBuilder : public QObject
{
public:
    explicit ModelBuilder();

    void buildBySections(Model *model, bool hard = true);
    void buildByTags(Model *model, bool hard = true);
    void buildPlain(Model *model, bool hard = true);
    std::unique_ptr<Edition> buildEdition();

    void setEditionRelationshipForAllComponents();
};
} // namespace alt

#endif // MODELBUILDER_H
