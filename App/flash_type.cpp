#include "flash_type.hpp"

FlashType::FlashType(QString type, QString start, int length, int major, int minor, int patch):
    type_(type),
    start_(start),
    length_(length),
    major_(major),
    minor_(minor),
    patch_(patch)
{

}
