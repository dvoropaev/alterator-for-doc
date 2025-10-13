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

#include <QStringList>

#include "treeitem.h"

TreeItem::TreeItem(const QList<QVariant> &data, ADTTool *tool, TreeItem *parent)
    // idk, but without any data model doesn't show up/Let it be any empty string
    : itemData(data)
    , m_parentItem(parent)
    , m_tool(tool)
    , m_icon(QIcon::fromTheme("system-run"))
{
    if (!tool->icon().isEmpty())
    {
        QIcon newIcon = QIcon::fromTheme(tool->icon());
        if (!newIcon.isNull())
        {
            m_icon = QIcon::fromTheme(tool->icon());
        }
    }
}

TreeItem::TreeItem(const QList<QVariant> &data)
    : itemData(data)
    , m_parentItem(nullptr)
    , m_tool(nullptr)
    , m_icon(QIcon::fromTheme("system-run"))
{}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

TreeItem *TreeItem::parent()
{
    return m_parentItem;
}

int TreeItem::row() const
{
    if (m_parentItem)
    {
        return m_parentItem->childItems.indexOf(const_cast<TreeItem *>(this));
    }

    return 0;
}

bool TreeItem::isChecked() const
{
    return m_checked;
}

void TreeItem::setChecked(bool state)
{
    m_checked = state;
}

ADTTool *TreeItem::getTool() const
{
    return m_tool.get();
}

QIcon TreeItem::getIcon()
{
    return m_icon;
}

void TreeItem::setlocaleForTool(QString locale)
{
    m_tool->setLocale(locale);
}
