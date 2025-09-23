/***********************************************************************************************************************
**
** Copyright (C) 2024 BaseALT Ltd. <org@basealt.ru>
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

#ifndef ADT_SETTINGS_H
#define ADT_SETTINGS_H

#include <qwidget.h>

class ADTSettingsPrivate;

class ADTSettings
{
public:
    ADTSettings();
    ~ADTSettings();

public:
    void restoreWindowSettings(QWidget *window);
    void saveWindowsSettings(QWidget *window);

    void saveReportPath(QString path);
    QString getReportPath();
    QString getLogPath();

    void saveReportFilenameTemplate(QString templ);
    QString getReportFilenameTemplate();
    QString getLogFilenameTemplate();

private:
    std::unique_ptr<ADTSettingsPrivate> d;

private:
    ADTSettings(const ADTSettings &)            = delete;
    ADTSettings(ADTSettings &&)                 = delete;
    ADTSettings &operator=(const ADTSettings &) = delete;
    ADTSettings &operator=(ADTSettings &&)      = delete;
};

#endif // ADT_SETTINGS_H
