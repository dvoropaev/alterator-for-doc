#ifndef ADTTOOLVARSMODELROWBUILDER_H
#define ADTTOOLVARSMODELROWBUILDER_H

#include "adtvarmodelbuilderinterface.h"

class ADTToolVarModelRowBuilderInt : public ADTVarModelBuilderInterface
{
public:
    void build(int row, std::unique_ptr<ADTVarInterface> &var, const QString &toolId, QStandardItemModel *model);
};

class ADTToolVarModelRowBuilderString : public ADTVarModelBuilderInterface
{
public:
    void build(int row, std::unique_ptr<ADTVarInterface> &var, const QString &toolId, QStandardItemModel *model);
};

class ADTToolVarModelRowBuilderEnumInt : public ADTVarModelBuilderInterface
{
public:
    void build(int row, std::unique_ptr<ADTVarInterface> &var, const QString &toolId, QStandardItemModel *model);
};

class ADTToolVarModelRowBuilderEnumString : public ADTVarModelBuilderInterface
{
public:
    void build(int row, std::unique_ptr<ADTVarInterface> &var, const QString &toolId, QStandardItemModel *model);
};

#endif // ADTTOOLVARSMODELROWBUILDER_H
