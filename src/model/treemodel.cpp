/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "treemodel.h"
#include "treeitem.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <qnamespace.h>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , rootItem(new TreeItem({""}))
{}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer())->columnCount()
                            : rootItem->columnCount();
}

QStringList TreeModel::getTools()
{
    QStringList tools{};

    for (int i = 0; i < this->rootItem->childCount(); i++)
    {
        auto toolItem = rootItem->child(i);
        tools.append(toolItem->getTool()->id());
    }

    return tools;
}

ADTTool *TreeModel::getTool(QString id)
{
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        auto toolItem = rootItem->child(i);
        if (!QString::compare(id, toolItem->getTool()->id(), Qt::CaseSensitive))
            return toolItem->getTool();
    }

    return nullptr;
}

QString TreeModel::getToolByPath(QString path)
{
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        auto toolItem = rootItem->child(i);
        if (!QString::compare(path, toolItem->getTool()->dbusPath(), Qt::CaseSensitive))
            return toolItem->getTool()->id();
    }

    return {};
}

void TreeModel::setToolFilter(QString toolId, QString filter)
{
    auto tool = getTool(toolId);
    if (!tool)
        return;

    tool->setFilter(filter);
}

QStringList TreeModel::getTests(QString toolId)
{
    QStringList tests{};

    auto tool = getTool(toolId);
    if (!tool)
        return tests;

    return tool->getTests();
}

QStringList TreeModel::getFilteredTests(QString toolId)
{
    QStringList filtTests{};

    auto tool = getTool(toolId);
    if (!tool)
        return filtTests;

    return tool->getFilteredTests();
}

ADTTest *TreeModel::getTest(QString toolId, QString testId)
{
    auto tool = getTool(toolId);
    if (!tool)
        return nullptr;

    return tool->getTest(testId);
}

void TreeModel::setLocaleForElements(QString locale)
{
    for (int i = 0; i < rootItem->childCount(); i++)
    {
        setLocaleForItem(rootItem->child(i), locale);
    }
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto item = static_cast<TreeItem *>(index.internalPointer());

    switch (role)
    {
    case Qt::DisplayRole:
        return QVariant(item->getTool()->displayName());
    case Qt::DecorationRole:
        return QVariant(item->getIcon());
    default:
        return QVariant();
    }
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::ItemFlag::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return rootItem->data(section);
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : rootItem;
    TreeItem *childItem  = parentItem->child(row);

    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return createIndex(0, 0, rootItem);
    }

    TreeItem *childItem  = static_cast<TreeItem *>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    return parentItem == rootItem ? QModelIndex() : createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem *>(parent.internalPointer()) : rootItem;

    return parentItem->childCount();
}

void TreeModel::setLocaleForItem(TreeItem *item, QString locale)
{
    item->setlocaleForTool(locale);

    for (int i = 0; i < item->childCount(); i++)
    {
        setLocaleForItem(item->child(i), locale);
    }
}
