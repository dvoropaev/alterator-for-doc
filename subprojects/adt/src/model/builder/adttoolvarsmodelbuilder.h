#ifndef ADTTOOLVARSMODELBUILDER_H
#define ADTTOOLVARSMODELBUILDER_H

#include <memory>
#include <QStandardItemModel>

#include <vars/adtvarsinterface.h>

#include <model/adttool.h>

class ADTToolVarsModelBuilder final
{
public:
    ADTToolVarsModelBuilder()  = default;
    ~ADTToolVarsModelBuilder() = default;

    std::unique_ptr<QStandardItemModel> buildVarsModel(std::vector<std::unique_ptr<ADTVarInterface>> &vars,
                                                       ADTTool *tool);

private:
    void buildRow(int row, std::unique_ptr<ADTVarInterface> &var, std::unique_ptr<QStandardItemModel> &model, const QString &toolId);
};

#endif // ADTTOOLVARSMODELBUILDER_H
