#pragma once
#include <QString>

inline QString toPythonVersionSpecific(const char *str)
    {
#ifdef BUILD_WITH_PYTHON3
    QString result(QString::fromUtf8(str));
    result.replace(QString::fromUtf8("ython2"), QString::fromUtf8("ython3"));
    return result;
#else
    return QString::fromUtf8(str);
#endif
    }