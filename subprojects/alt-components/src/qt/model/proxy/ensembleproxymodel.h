#ifndef ENSEMBLE_PROXY_MODEL_H
#define ENSEMBLE_PROXY_MODEL_H

#include <QIdentityProxyModel>

namespace alt
{
class EnsembleProxyModel : public QIdentityProxyModel
{
public:
    explicit EnsembleProxyModel(QObject *parent = nullptr);

public:
    [[deprecated("Use insert(), erase() or setRootModel() instead")]]
    void setSourceModel(QAbstractItemModel *model) override;
    QModelIndex mapToMember(const QModelIndex &proxyIndex, const QAbstractItemModel *memberModel);
    void setRootModel(QAbstractItemModel *model);
    QAbstractItemModel *rootModel();
    void insert(size_t weight, QAbstractProxyModel *proxyModel);
    void erase(QAbstractProxyModel *proxyModel);
    void erase(size_t weight);

private:
    void relink();

private:
    QAbstractItemModel *m_rootModel;
    // NOTE(sheriffkorov): bidirectional map emulation
    std::map<size_t, QAbstractProxyModel *> weightToProxy;
    std::map<QAbstractProxyModel *, size_t> proxyToWeight;
};
} // namespace alt

#endif // ENSEMBLE_PROXY_MODEL_H
