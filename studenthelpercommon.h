#ifndef STUDENTHELPERCOMMON
#define STUDENTHELPERCOMMON

#include <QString>


class SHException
{
public:
    SHException(const QString &msg)
        : _msg(msg) {}

    const QString &getMsg() const { return _msg; }

private:
    QString _msg;
};


#endif // STUDENTHELPERCOMMON

