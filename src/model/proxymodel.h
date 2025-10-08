#ifndef MODEL_PROXY_MODEL_H
#define MODEL_PROXY_MODEL_H

#include <QSortFilterProxyModel>

namespace alt
{
class ProxyModel : public QSortFilterProxyModel
{
public:
    explicit ProxyModel(QObject *parent = nullptr);

    void setShowDrafts(bool showDrafts);
    void setFilterNonEditionComponents(bool show);

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool showDrafts = false;
    bool showNonEditionComponentsSection = false;

    bool hasRelationToEdition(const QModelIndex &index) const;
};
} // namespace alt

#endif // MODEL_PROXY_MODEL_H
