#ifndef COMMON_ICON_CREATION_H
#define COMMON_ICON_CREATION_H

#include <QIcon>

extern QIcon push_button_default_icon_;
extern QIcon push_button_save_icon_;
extern QIcon push_button_info_icon_;

//Call this when first setting up the GUI to create common QIcon objects that will be shared in multiple places.
//Allows us to only create this oftern used icons once and share them, saving RAM.
void icon_setup();

#endif // COMMON_ICON_CREATION_H
