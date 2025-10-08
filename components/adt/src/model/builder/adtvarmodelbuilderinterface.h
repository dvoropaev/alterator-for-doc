#ifndef ADTVARMODELBUILDERINTERFACE_H
#define ADTVARMODELBUILDERINTERFACE_H

#include "vars/adtvarsinterface.h"
#include <memory>
#include <QStandardItemModel>
#include <QString>

class ADTVarModelBuilderInterface
{
public:
    enum VarBuilderType
    {
        INT,
        STRING,
        ENUM_INT,
        ENUM_STRING
    };

public:
    virtual ~ADTVarModelBuilderInterface() = default;

    virtual void build(int row, std::unique_ptr<ADTVarInterface> &var, const QString &toolId, QStandardItemModel *model)
        = 0;
};

#endif // ADTVARMODELBUILDERINTERFACE_H
