#ifndef FLASHTYPE_HPP
#define FLASHTYPE_HPP

#include <QString>

class FlashType
{
public:
    FlashType(QString type, QString start, int length, int major, int minor, int patch);
    QString GetType() { return type_; }
    QString GetStart() { return start_; }
    int GetLength() { return length_; }
    int GetMajor() { return major_; }
    int GetMinor() { return minor_; }
    int GetPatch() { return patch_; }

private:
    QString type_;
    QString start_;
    int length_;
    int major_;
    int minor_;
    int patch_;
};

#endif // FLASHTYPE_HPP
