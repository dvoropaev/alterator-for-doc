#ifndef ALTERATORCTL_PACKAGES_H
#define ALTERATORCTL_PACKAGES_H

#include <gio/gio.h>
#include <glib-object.h>

#include "alteratorctlapp.h"
#include "alteratorctlcommon.h"
#include "alteratorctlgdbussource.h"

#define TYPE_ALTERATOR_CTL_PACKAGES_MODULE (alterator_ctl_packages_module_get_type())
#define ALTERATOR_CTL_PACKAGES_MODULE(obj)                                 \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ALTERATOR_CTL_PACKAGES_MODULE, \
                                AlteratorCtlPackagesModule))
#define IS_ALTERATOR_CTL_PACKAGES_MODULE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ALTERATOR_CTL_PACKAGES_MODULE))
#define ALTERATOR_CTL_PACKAGES_MODULE_CLASS(klass)                        \
    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ALTERATOR_CTL_PACKAGES_MODULE, \
                             AlteratorCtlPackagesModuleClass))
#define IS_ALTERATOR_CTL_PACKAGES_MODULE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ALTERATOR_CTL_PACKAGES_MODULE))
#define ALTERATOR_CTL_PACKAGES_MODULE_GET_CLASS(obj)                      \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ALTERATOR_CTL_PACKAGES_MODULE, \
                               AlteratorCtlPackagesModuleClass))

enum packages_sub_commands
{
    PACKAGES_RPM,
    PACKAGES_APT,
    PACKAGES_REPO
};

enum packages_apt_subcommand
{
    APT_APPLY_ASYNC = 1 << 7,
    APT_INFO        = 1,
    APT_CHECK_APPLY_PRIV,
    APT_INSTALL,
    APT_LIST_ALL_PACKAGES,
    APT_REINSTALL,
    APT_REMOVE,
    APT_SEARCH,
    APT_UPDATE,
    APT_LAST_UPDATE
};

enum packages_rpm_subcommand
{
    RPM_FILES,
    RPM_INFO,
    RPM_INSTALL,
    RPM_LIST,
    RPM_PACKAGE_INFO,
    RPM_REMOVE
};

enum packages_repo_subcommand
{
    REPO_ADD,
    REPO_INFO,
    REPO_LIST,
    REPO_REMOVE
};

typedef struct
{
    GObjectClass parent_class;

} AlteratorCtlPackagesModuleClass;

typedef struct
{
    GObject parnet_instance;

    GHashTable** commands;

    AlteratorGDBusSource* gdbus_source;

    AlteratorCtlApp* alterator_ctl_app;

} AlteratorCtlPackagesModule;

alterator_ctl_module_t* get_packages_module();

int packages_module_run_with_args(gpointer self, int argc, char** argv);

int packages_module_run(gpointer self, gpointer data);

int packages_module_print_help(gpointer self);

#endif // ALTERATORCTL_PACKAGES_H
