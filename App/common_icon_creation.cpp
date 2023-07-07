#include "common_icon_creation.h"

QIcon push_button_default_icon_;
QIcon push_button_save_icon_;
QIcon push_button_info_icon_;

void icon_setup(){
    push_button_default_icon_.addFile(QString(":/res/default_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
    push_button_save_icon_.addFile(QString(":/res/save.png"), QSize(), QIcon::Normal, QIcon::Off);
    push_button_info_icon_.addFile( QString(":/res/info_icon.png"), QSize(), QIcon::Normal, QIcon::Off);
}
