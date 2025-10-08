#ifndef AOB_CONSTANTS_H
#define AOB_CONSTANTS_H

#define CONST const char *const

namespace ao_builder
{
namespace dbus
{
CONST SERVICE_NAME = "org.altlinux.alterator";

namespace method
{
CONST GET_OBJECTS = "GetObjects";
CONST INFO = "Info";
CONST LIST = "List";
CONST RUN = "Run";
} // namespace method
namespace path
{
CONST ALTERATOR = "/org/altlinux/alterator";
CONST GLOBAL = "/org/altlinux/alterator/global";
} // namespace path

namespace interface
{
CONST MANAGER = "org.altlinux.alterator.manager";
CONST CATEGORY = "org.altlinux.alterator.categories1";
CONST LOCAL_APP = "org.altlinux.alterator.application1";
CONST LEGACY_OBJECT = "org.altlinux.alterator.legacy1";
CONST OBJECT = "org.altlinux.alterator.object1";
CONST APT = "org.altlinux.alterator.apt1";
CONST RPM = "org.altlinux.alterator.rpm1";
CONST REPO = "org.altlinux.alterator.repo1";
CONST COMPONENTS = "org.altlinux.alterator.components1";
CONST DIAG = "org.altlinux.alterator.diag1";
CONST LICENSE = "org.altlinux.alterator.license1";
CONST SYSTEMINFO = "org.altlinux.alterator.systeminfo1";
CONST CONTROL = "org.altlinux.alterator.control1";
CONST RELEASE_NOTES = "org.altlinux.alterator.release_notes1";
} // namespace interface
} // namespace dbus

/*
 * Only type-specific keys here.
 * See builders/keyprovider.h for list of base object keys
*/
namespace key
{
namespace desktop
{
CONST SECTION_NAME = "Desktop Entry";

CONST TERMINAL = "Terminal";
CONST X_ALTERATOR_URI = "X-Alterator-URI";
CONST X_ALTERATOR_WEIGHT = "X-Alterator-Weight";
CONST X_ALTERATOR_HELP = "X-Alterator-Help";
CONST X_ALTERATOR_UI = "X-Alterator-UI";
CONST X_ALTERATOR_INTERNAL = "X-Alterator-Internal-Name";
CONST X_ALTERATOR_CATEGORY = "X-Alterator-Category";

} // namespace desktop
namespace alterator
{
CONST CATEGORY = "category";
CONST INTERFACE = "interface";
CONST DISPLAY_NAME = "display_name";
CONST OVERRIDE = "override";
} // namespace alterator
} // namespace key

namespace type
{
CONST APPLICATION = "Application";
CONST CATEGORY = "Category";
CONST OBJECT = "Object";
CONST DIAG1 = "Diag";
} // namespace type

namespace category
{
CONST NAME = "__default";
CONST DISPLAY_NAME = "Others";
CONST DISPLAY_NAME_EN = "Others";
CONST DISPLAY_NAME_RU = "Другое";
CONST ICON = "groups/other";
CONST COMMENT = "Objects without category";
CONST COMMENT_EN = "Objects without category";
CONST COMMENT_RU = "Объекты без категории";
} // namespace category
} // namespace ao_builder

#undef CONST

#endif // AB_CONSTANTS_H
