#include "flash_type.hpp"

FlashType::FlashType(){}

FlashType::FlashType(QString type, QString start, int length, int major, int minor, int patch, int style):
    type_(type),
    start_(start),
    length_(length),
    major_(major),
    minor_(minor),
    patch_(patch),
    style_(style)
{

}

void FlashType::Init(QString type, QString start, int length, int major, int minor, int patch, int style){
    type_ = type;
    start_ = start;
    length_ = length;
    major_ = major;
    minor_ = minor;
    patch_ = patch;
    style_ = style;
}
