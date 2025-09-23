#include "adttoolvarsmodelbuilder.h"

#include "adtvarmodelbuilderinterface.h"
#include "adtvarmodelbuildfactory.h"

std::unique_ptr<QStandardItemModel> ADTToolVarsModelBuilder::buildVarsModel(
    std::vector<std::unique_ptr<ADTVarInterface>> &vars, ADTTool *tool)
{
    std::unique_ptr<QStandardItemModel> model{new QStandardItemModel()};

    for (size_t row = 0; row < vars.size(); row++)
    {
        buildRow(row, vars.at(row), model, tool->id());
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Value"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Comment"));

    return model;
}

void ADTToolVarsModelBuilder::buildRow(int row,
                                       std::unique_ptr<ADTVarInterface> &var,
                                       std::unique_ptr<QStandardItemModel> &model,
                                       const QString &toolId)
{
    switch (var->getType())
    {
    case ADTVarInterface::INT:
        ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::INT>::create()->build(row,
                                                                                                   var,
                                                                                                   toolId,
                                                                                                   model.get());
        break;

    case ADTVarInterface::STRING:
        ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::STRING>::create()->build(row,
                                                                                                      var,
                                                                                                      toolId,
                                                                                                      model.get());
        break;

    case ADTVarInterface::ENUM_INT:
        ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::ENUM_INT>::create()->build(row,
                                                                                                        var,
                                                                                                        toolId,
                                                                                                        model.get());
        break;

    case ADTVarInterface::ENUM_STRING:
        ADTVarModelBuildFactory<ADTVarModelBuilderInterface::VarBuilderType::ENUM_STRING>::create()->build(row,
                                                                                                           var,
                                                                                                           toolId,
                                                                                                           model.get());
        break;
    default:
        break;
    }
}
