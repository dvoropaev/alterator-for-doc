/***********************************************************************************************************************
**
** Copyright (C) 2023 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#ifndef ADT_APP_H
#define ADT_APP_H

#include "adtsettings.h"

#include <QApplication>
#include <QDBusConnection>

class ADTAppPrivate;

class ADTApp : public QObject
{
    Q_OBJECT
public:
    ADTApp(QApplication *application, ADTSettings *settings, QDBusConnection conn, QString locale);
    ~ADTApp();

    int runApp();

private:
    void buildModel();

private:
    ADTAppPrivate *d;

private:
    ADTApp(const ADTApp &)            = delete;
    ADTApp(ADTApp &&)                 = delete;
    ADTApp &operator=(const ADTApp &) = delete;
    ADTApp &operator=(ADTApp &&)      = delete;
};

#endif // ADT_APP_H
