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

#ifndef ADT_COMMAND_LINE_PARSER_H
#define ADT_COMMAND_LINE_PARSER_H

#include "adtcommandlineoptions.h"

#include <QApplication>

class ADTCommandLineParserPrivate;

class ADTCommandLineParser
{
public:
    enum CommandLineParseResult
    {
        CommandLineOk,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested,
        CommandLineListOfObjectsRequested,
        CommandLineListOfTestsRequested,
        CommandLineRunAllTestsRequested,
        CommandLineRunSpecifiedTestRequested,
        CommandLineGetReportSpecifiedTool
    };

public:
    ADTCommandLineParser(QApplication &application);
    ~ADTCommandLineParser();

    CommandLineParseResult parseCommandLine(ADTCommandLineOptions *options, QString *errorMessage);

    void showHelp();
    void showVersion();

private:
    ADTCommandLineParser(const ADTCommandLineParser &)            = delete;
    ADTCommandLineParser(ADTCommandLineParser &&)                 = delete;
    ADTCommandLineParser &operator=(const ADTCommandLineParser &) = delete;
    ADTCommandLineParser &operator=(ADTCommandLineParser &&)      = delete;

private:
    ADTCommandLineParserPrivate *d;
};

#endif // ADT_COMMAND_LINE_PARSER_H
