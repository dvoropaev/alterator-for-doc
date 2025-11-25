#ifndef ENTITY_CONSTANTS_H
#define ENTITY_CONSTANTS_H

namespace entity
{
const char *const ALTERATOR_SECTION_NAME = "Alterator Entry";

const char *const COMPONENT_NAME_KEY_NAME = "name";
const char *const COMPONENT_DRAFT_KEY_NAME = "draft";
const char *const COMPONENT_OBJECT_TYPE_KEY_NAME = "type";
const char *const COMPONENT_TYPE_KEY_VALUE = "component";
const char *const COMPONENT_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const COMPONENT_TYPE_KEY_NAME = "component";
const char *const COMPONENT_COMMENT_KEY_NAME = "comment";
const char *const COMPONENT_CATEGORY_KEY_NAME = "category";
const char *const COMPONENT_ICON_KEY_NAME = "icon";
const char *const COMPONENT_PACKAGES_KEY_NAME = "packages";
const char *const COMPONENT_TAGS_KEY_NAME = "tags";
const char *const COMPONENT_DEFAULT_LANUAGE = "en";

const char *const PACKAGE_META_KEY = "meta";
const char *const PACKAGE_KERNEL_MODULE_KEY = "kernel_module";
const char *const PACKAGE_IMAGE_IGNORE_KEY = "image_ignore";
const char *const PACKAGE_LANGUAGE_KEY = "language";
const char *const PACKAGE_DESKTOP_KEY = "desktop";
const char *const PACKAGE_ARCH_KEY = "arch";
const char *const PACKAGE_EXCLUDE_ARCH_KEY = "exclude_arch";

const char *const DEFAULT_CATEGORY_ID = "__default";
const char *const DEFAULT_CATEGORY_DISPLAY_NAME = "Other";
const char *const DEFAULT_CATEGORY_DISPLAY_NAME_RU = "Другое";

const char *const EDITION_NAME_KEY_NAME = "name";
const char *const EDITION_OBJECT_TYPE_KEY_NAME = "type";
const char *const EDITION_TYPE_KEY_VALUE = "edition";
const char *const EDITION_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const EDITION_LICENSE_KEY_NAME = "license";
const char *const EDITION_ARCHES_KEY_NAME = "arches";
const char *const EDITION_DE_KEY_NAME = "desktop_environment";
const char *const EDITION_KFLAVOURS_KEY_NAME = "kflavours";
const char *const EDITION_LANGUAGES_KEY_NAME = "languages";
const char *const EDITION_DEFAULT_LANUAGE = "en";
const char *const EDITION_SECTIONS_KEY_NAME = "sections";
const char *const EDITION_TAGS_KEY_NAME = "tags";

const char *const SECTION_DISPLAY_NAME_KEY_NAME = "display_name";
const char *const SECTION_COMPONENTS_KEY_NAME = "components";
const char *const SECTION_DEFAULT_LANUAGE = "en";

const char *const DEFAULT_SECTION_NAME = "__other";
const int DEFAULT_SECTION_WEIGHT = 99;
} // namespace entity
#endif // ENTITY_CONSTANTS_H
