#pragma once
#include <QString>

inline QString toPythonVersionSpecific(const char *str)
{
    QString result(QString::fromUtf8(str));
#ifdef BUILD_WITH_PYTHON3
    result.replace(QString::fromUtf8("ython2"), QString::fromUtf8("ython3"));
#endif
    return result;
}