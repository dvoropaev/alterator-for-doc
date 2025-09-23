#ifndef INTERFACEDATA_H
#define INTERFACEDATA_H

#include <QString>

struct DBusConfig
{
    const char *managerService   = "org.altlinux.alterator";
    const char *managerPath      = "/org/altlinux/alterator";
    const char *managerInterface = "org.altlinux.alterator.manager";
    const char *managerGetMethod = "GetObjects";
    const char *diagInterface    = "org.altlinux.alterator.diag1";
    const char *diagRunMethod    = "Run";
    const char *diagInfoMethod   = "Info";
    const char *diagReportMethod = "Report";
    const char *diagListMethod   = "List";
};

#endif // INTERFACEDATA_H
