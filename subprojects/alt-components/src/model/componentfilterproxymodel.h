#ifndef COMPONENT_FILTER_PROXY_MODEL_H
#define COMPONENT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

namespace alt
{
class ComponentFilterProxyModel : public QSortFilterProxyModel
{
public:
    enum FilterOptions : uint8_t
    {
        None = 0x00,
        Draft = 0x01,
        NonEdition = 0x02,
    };
    struct ComponentsCount
    {
        int total = 0;
        int installed = 0;
    };

public:
    explicit ComponentFilterProxyModel(QObject *parent = nullptr);

public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setFilter(FilterOptions option, bool value);
    bool testFilter(FilterOptions option);
    ComponentsCount countComponents(const QModelIndex &index = {}) const;

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QFlags<FilterOptions> options;
};
} // namespace alt

#endif // COMPONENT_FILTER_PROXY_MODEL_H
