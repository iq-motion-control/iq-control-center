#ifndef FLASHTYPE_HPP
#define FLASHTYPE_HPP

#include <QString>

class FlashType
{
public:
    FlashType();
    FlashType(QString type, QString start, int length, int major, int minor, int patch, int style);
    QString GetType() { return type_; }
    QString GetStart() { return start_; }
    int GetLength() { return length_; }
    int GetMajor() { return major_; }
    int GetMinor() { return minor_; }
    int GetPatch() { return patch_; }
    int GetStyle() { return style_; }

    void Init(QString type, QString start, int length, int major, int minor, int patch, int style);
private:
    QString type_;
    QString start_;
    int length_;
    int major_;
    int minor_;
    int patch_;
    int style_;
};

#endif // FLASHTYPE_HPP
