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

#include "adtapp.h"
#include "adtsettings.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QIcon>
#include <QTranslator>

#include "version.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // NOTE: set app variables which will be used to construct settings path
    app.setOrganizationName(QCoreApplication::translate("main", "BaseALT"));
    app.setOrganizationDomain("basealt.ru");
    app.setApplicationName("ADT");
    app.setApplicationVersion(getApplicationVersion());
    app.setWindowIcon(QIcon(":adt.svg"));

    QTranslator translator;
    QString language = QLocale::system().name().split("_").at(0);
    if (!translator.load(QString("adt_%1").arg(language), ":/") && !translator.load("adt_en", ":/"))
    {
        qWarning() << "Can't load language: " << language;
        return -1;
    }

    app.installTranslator(&translator);

    ADTSettings settings{};

    QDBusConnection connection(QDBusConnection::systemBus());

    ADTApp adtApp(&app, &settings, connection, language);

    return adtApp.runApp();
}
