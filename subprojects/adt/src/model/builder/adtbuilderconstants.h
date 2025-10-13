#ifndef ADTBUILDERCONSTANTS_H
#define ADTBUILDERCONSTANTS_H

#include "vars/adtvarbuilderinterface.h"
#include <vars/adtvarsinterface.h>

struct ADTBuilderToolConstants
{
    const char *NAME_KEY_NAME                = "name";
    const char *TYPE_KEY_NAME                = "type";
    const char *DIAG_KEY_VALUE               = "Diag";
    const char *DISPLAY_NAME_KEY_NAME        = "display_name";
    const char *ICON_KEY_NAME                = "icon";
    const char *COMMENT_KEY_NAME             = "comment";
    const char *CATEGORY_KEY_NAME            = "category";
    const char *ALTERATOR_ENTRY_SECTION_NAME = "Alterator Entry";
    const char *REPORT_FILE_SUFFIX_KEY_NAME  = "report_suffix";
    const char *TESTS_SECTION_NAME           = "tests";
    const char *PARAMS_SECTION_NAME          = "parameters";
    const char *DEFAULT_ICON                 = "system-run";
};

struct ADTBuilderTestConstants
{
    const char *TABLE_NAME            = "tests";
    const char *DISPLAY_NAME_KEY_NAME = "display_name";
    const char *COMMENT_KEY_NAME      = "comment";
    const char *ICON_KEY_NAME         = "icon";
    const char *DEFAULT_ICON          = "system-run";
};

struct ADTBuilderParamConstants
{
    const char *TYPE_KEY_NAME               = "type";
    const char *TYPE_KEY_VALUE_TRUE         = "true";
    const char *VALUE_KEY_NAME              = "value";
    const char *DISPLAY_NAME_KEY_NAME       = "display_name";
    const char *COMMENT_KEY_NAME            = "comment";
    const char *STRING_TYPE_VALUE           = "string";
    const char *INT_TYPE_VALUE              = "int";
    const char *ENUM_TYPE_VALUE             = "enum";
    const char *ENUM_VALUES_KEY             = "enum_values";
    const char *ENUM_TYPE_TYPE_KEY_NAME     = "type";
    const char *ENUM_TYPE_STRING_TYPE_VALUE = "string";
    const char *ENUM_TYPE_INT_TYPE_VALUE    = "int";
    const char *ENUM_TYPE_VALUES_KEY_NAME   = "values";
    const char *ENUM_TYPE_DEFAULT_KEY_NAME  = "default";
};

const static ADTBuilderParamConstants PARAM_BUILDER_CONSTANTS;

static std::map<QString, ADTVarBuilderInterface::BuilderType> STRING_TO_ADTVARBUILDER_TYPE{
    {QString(PARAM_BUILDER_CONSTANTS.STRING_TYPE_VALUE), ADTVarBuilderInterface::BuilderType::STRING},
    {QString(PARAM_BUILDER_CONSTANTS.INT_TYPE_VALUE), ADTVarBuilderInterface::BuilderType::INT},
    {QString(PARAM_BUILDER_CONSTANTS.ENUM_TYPE_VALUE), ADTVarBuilderInterface::BuilderType::ENUM},
};

#endif // ADTBUILDERCONSTANTS_H
