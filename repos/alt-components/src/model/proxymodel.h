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

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool showDrafts = false;
};
} // namespace alt

#endif // MODEL_PROXY_MODEL_H
