#include "ensembleproxymodel.h"

namespace alt
{
EnsembleProxyModel::EnsembleProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , m_rootModel(nullptr)
{}

void EnsembleProxyModel::setSourceModel(QAbstractItemModel *model)
{
    std::ignore = model;
}

void EnsembleProxyModel::setRootModel(QAbstractItemModel *model)
{
    m_rootModel = model;
    if (weightToProxy.empty())
    {
        QIdentityProxyModel::setSourceModel(m_rootModel);
        return;
    }

    auto *firstProxy = weightToProxy.begin()->second;
    if (firstProxy == nullptr || firstProxy->sourceModel() == m_rootModel)
    {
        return;
    }

    firstProxy->setSourceModel(m_rootModel);
}

QAbstractItemModel *EnsembleProxyModel::rootModel()
{
    return m_rootModel;
}

QModelIndex EnsembleProxyModel::mapToMember(const QModelIndex &proxyIndex, const QAbstractItemModel *memberModel)
{
    if (!proxyIndex.isValid() || memberModel == nullptr
        || (!weightToProxy.empty() && weightToProxy.rbegin()->second != sourceModel()))
    {
        return {};
    }

    QModelIndex memberIndex = mapToSource(proxyIndex);
    for (auto iter = weightToProxy.rbegin(); iter != weightToProxy.rend() && memberIndex.isValid(); ++iter)
    {
        if (memberIndex.model() == memberModel)
        {
            return memberIndex;
        }
        const auto &[_, model] = *iter;
        if (model == nullptr && memberIndex.model() != model)
        {
            return {};
        }
        memberIndex = model->mapToSource(memberIndex);
    }
    return memberIndex;
}

void EnsembleProxyModel::insert(size_t weight, QAbstractProxyModel *proxyModel)
{
    if (proxyModel == nullptr)
    {
        return;
    }

    auto found = std::find_if(weightToProxy.begin(),
                              weightToProxy.end(),
                              [weight](const std::pair<size_t, QAbstractProxyModel *> &item) {
                                  return item.first == weight;
                              });
    if (found != weightToProxy.end())
    {
        proxyToWeight.erase(found->second);
        found->second = proxyModel;
    }
    else
    {
        weightToProxy[weight] = proxyModel;
    }
    proxyToWeight[proxyModel] = weight;
    relink();
}

void EnsembleProxyModel::erase(QAbstractProxyModel *proxyModel)
{
    if (proxyModel == nullptr)
    {
        return;
    }

    auto found = std::find_if(proxyToWeight.begin(),
                              proxyToWeight.end(),
                              [proxyModel](const std::pair<QAbstractProxyModel *, size_t> &item) {
                                  return item.first == proxyModel;
                              });
    if (found == proxyToWeight.end())
    {
        return;
    }

    found->first->setSourceModel(nullptr);
    weightToProxy.erase(found->second);
    proxyToWeight.erase(found);
    relink();
}

void EnsembleProxyModel::erase(size_t weight)
{
    auto found = std::find_if(weightToProxy.begin(),
                              weightToProxy.end(),
                              [weight](const std::pair<size_t, QAbstractProxyModel *> &item) {
                                  return item.first == weight;
                              });
    if (found == weightToProxy.end())
    {
        return;
    }

    found->second->setSourceModel(nullptr);
    proxyToWeight.erase(found->second);
    weightToProxy.erase(found);
    relink();
}

void EnsembleProxyModel::relink()
{
    if (weightToProxy.empty())
    {
        return;
    }

    auto rend = weightToProxy.rend();
    --rend;
    auto rbegin = weightToProxy.rbegin();
    for (auto iter = rbegin; iter != rend;)
    {
        iter->second->setSourceModel((++iter)->second);
    }
    rend->second->setSourceModel(rootModel());
    QIdentityProxyModel::setSourceModel(rbegin->second);
}
} // namespace alt
