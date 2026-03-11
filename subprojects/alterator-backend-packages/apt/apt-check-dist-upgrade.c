#include <stdio.h>
#include <glib.h>
#include <packagekit-glib2/packagekit.h>

enum Pipe {
    STDOUT,
    STDERR
};

void print_packages_by_info(PkPackageSack *sack, PkInfoEnum info_type, gint pipe)
{
    PkPackageSack *filtered_sack = pk_package_sack_filter_by_info(sack, info_type);
    GPtrArray *packages = pk_package_sack_get_array(filtered_sack);

    if (packages->len > 0) {
        for (guint i = 0; i < packages->len; i++) {
            PkPackage *pkg = g_ptr_array_index(packages, i);
            const gchar *name = pk_package_get_name(pkg);

            pipe ? g_printerr("%s\n", name) : g_print("%s\n", name);
        }
    }

    g_object_unref(filtered_sack);
}

int main(int argc, char *argv[])
{
    GError *error = NULL;
    PkClient *client = NULL;
    PkResults *results = NULL;
    PkPackageSack *sack = NULL;

    client = pk_client_new();

    results = pk_client_get_updates(client, PK_FILTER_ENUM_NONE, NULL, NULL, NULL, &error);

    if (error) {
        g_printerr("%s\n", error->message);
        g_error_free(error);
        g_object_unref(client);
        return 1;
    }

    sack = pk_results_get_package_sack(results);

    print_packages_by_info(sack, PK_INFO_ENUM_INSTALLING, STDOUT);

    print_packages_by_info(sack, PK_INFO_ENUM_REMOVING, STDERR);

    g_object_unref(results);
    g_object_unref(client);

    return 0;
}