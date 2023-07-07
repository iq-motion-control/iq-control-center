#ifndef COMMON_ICON_CREATION_H
#define COMMON_ICON_CREATION_H

#include <QIcon>

//TODO: Should we move all this stuff into the parent frame instead? Might make sense there, not sure. Would need to be static...
extern QIcon push_button_default_icon_;
extern QIcon push_button_save_icon_;
extern QIcon push_button_info_icon_;

void icon_setup();

#endif // COMMON_ICON_CREATION_H
