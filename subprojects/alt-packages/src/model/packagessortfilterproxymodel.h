#ifndef PACKAGESFILTERPROXYMODEL_H
#define PACKAGESFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class PackagesFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    void setFilterGroup(const QString &group);
    void setFilterArch(const QString &arch);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_group{};
    QString m_arch{};
};

#endif // PACKAGESFILTERPROXYMODEL_H
