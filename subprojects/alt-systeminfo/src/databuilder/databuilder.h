#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "entity/product.h"

#include <QStandardItemModel>
#include <QStringList>

#include <memory>

namespace alt
{
class DataBuilder
{
public:
    enum PropertyModelType
    {
        PropertyModelTypeOperationSystem,
        PropertyModelTypeHardware,
    };

public:
    static std::unique_ptr<QStandardItemModel> buildPropertyModel(PropertyModelType type);
    static QString buildHostName();
    static QString buildUsefulSources();

    static void rebuildProduct();
    static void rebuildEdition();

    static const Product &product();

public:
    DataBuilder() = delete;

public:
    DataBuilder(const DataBuilder &) = delete;
    DataBuilder(DataBuilder &&) = delete;
    DataBuilder &operator=(const DataBuilder &) = delete;
    DataBuilder &operator=(DataBuilder &&) = delete;

private:
    static QList<QList<QStandardItem *>> buildProperties(PropertyModelType type);

    static QList<QStandardItem *> buildPropertyHostName();
    static QList<QStandardItem *> buildPropertyOsName();
    static QList<QStandardItem *> buildPropertyEdition();
    static QList<QStandardItem *> buildPropertyBranch();
    static QList<QStandardItem *> buildPropertyUpdatedOn();
    static QList<QStandardItem *> buildPropertyKernel();

    static QList<QStandardItem *> buildPropertyProcessor();
    static QList<QStandardItem *> buildPropertyArch();
    static QList<QStandardItem *> buildPropertyGraphics();
    static QList<QStandardItem *> buildPropertyMemory();
    static QList<QStandardItem *> buildPropertyDrive();
    static QList<QStandardItem *> buildPropertyMotherboard();
    static QList<QStandardItem *> buildPropertyDisplays();

    static QStandardItem *buildPropertyName(const QString &name, bool isRequiredForDisplay = false);
    static QStandardItem *buildPropertyValue(const QString &value, bool isEditable);

    static void rebuildProductDisplayName();
    static void rebuildBranch();
    static void rebuildProductLicense();
    static void rebuildProductLogo();

private:
    inline static Product m_product{};
};
} // namespace alt

#endif // MODELBUILDER_H
