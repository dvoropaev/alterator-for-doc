#include "repocontroller.h"

#include "ui/mainwindow/mainwindow.h"

#include <vector>
#include <QDebug>
#include <QInputDialog>
#include <QObject>

RepoController::RepoController(DataSourceInterface *dataSource, MainController *mc)
    : BaseController(dataSource, mc)
{
    m_proxyModel->sort(columnSign, Qt::AscendingOrder);

    updateModel();
}

RepoController::~RepoController() = default;

void RepoController::onSelected() {}

void RepoController::onDeselected() {}

void RepoController::onWindowClose() {}

void RepoController::onRemoveRepoRequested(const QModelIndexList &indexes)
{
    for (auto index : indexes)
    {
        if (index.column() == columnRepo)
        {
            const auto &repo = index.data(Qt::UserRole).value<Repo>();
            QStringList backendResult;
            int code = getDataSource()->repoRemove(backendResult, repo.m_full);
            if (code)
            {
                emit showError(code, backendResult);
                return;
            }
        }
    }

    emit requestToUpdateData();
}

void RepoController::onAddRepoRequested()
{
    bool ok;
    QString repoName = QInputDialog::getText(m_window.get(),
                                             tr("Alterator Packages Repo"),
                                             tr("Enter repository addres or name"),
                                             QLineEdit::Normal,
                                             "",
                                             &ok);
    if (!ok || repoName.isEmpty())
    {
        return;
    }

    QStringList backendResult;
    int exitCode = getDataSource()->repoAdd(backendResult, repoName);
    if (exitCode)
    {
        emit showError(exitCode, backendResult);
        return;
    }
    updateModel();
}

void RepoController::updateModel()
{
    const auto &data = getModelData();

    m_model->clear();

    auto repoFields = {&Repo::m_type, &Repo::m_branch, &Repo::m_url, &Repo::m_component};

    for (auto &repo : data)
    {
        QList<QStandardItem *> list;

        QStandardItem *repoItem = new QStandardItem();
        repoItem->setData(QVariant::fromValue(repo), Qt::UserRole);
        list << repoItem;

        for (auto m_field : repoFields)
        {
            list << std::make_unique<QStandardItem>(repo.*m_field).release();
        }

        m_model->appendRow(list);
    }

    updateModelHeaderData();
}

void RepoController::updateModelHeaderData()
{
    m_model->setHeaderData(columnType, Qt::Horizontal, tr("Type"), Qt::DisplayRole);
    m_model->setHeaderData(columnSign, Qt::Horizontal, tr("Sign"), Qt::DisplayRole);
    m_model->setHeaderData(columnUrl, Qt::Horizontal, tr("URL"), Qt::DisplayRole);
    m_model->setHeaderData(columnComponent, Qt::Horizontal, tr("Component"), Qt::DisplayRole);
}

std::vector<Repo> RepoController::getModelData()
{
    std::vector<Repo> data;

    QStringList backendResult;
    int exitCode = getDataSource()->repoList(backendResult);
    if (exitCode)
    {
        qWarning() << "Failed to get repo list from APT";
        emit showError(exitCode, backendResult);
        return {};
    }

    QRegularExpression regExp("(rpm|rpm-dir|rpm-src) (?:(\\[\\w*\\]) )?(.*) (\\w*)");
    for (QString repo : backendResult)
    {
        auto match = regExp.match(repo);
        data.push_back({match.captured(1), match.captured(2), match.captured(3), match.captured(4), repo});
    }

    return data;
}
