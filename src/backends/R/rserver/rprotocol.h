/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
*/

#ifndef RPROTOCOL_H
#define RPROTOCOL_H

#include <QString>
#include <QStringList>
#include <QDataStream>

namespace RProtocol {

// Message types sent from client to server
enum class ClientMessage : quint8 {
    RunCommand = 1,
    AnswerRequest = 2,
    Interrupt = 3
};

// Message types sent from server to client
enum class ServerMessage : quint8 {
    StatusChanged = 1,
    ExpressionFinished = 2,
    InputRequested = 3,
    PartialOutput = 4
};

// Server status
enum class Status : quint8 {
    Idle = 0,
    Busy = 1
};

// Return codes for expression evaluation
enum class ReturnCode : quint8 {
    Success = 0,
    Error = 1
};

// Serialize/deserialize helpers
inline void writeString(QDataStream& stream, const QString& str) {
    QByteArray utf8 = str.toUtf8();
    stream << static_cast<quint32>(utf8.size());
    stream.writeRawData(utf8.constData(), utf8.size());
}

inline QString readString(QDataStream& stream) {
    quint32 len;
    stream >> len;
    QByteArray buf(len, Qt::Uninitialized);
    stream.readRawData(buf.data(), len);
    return QString::fromUtf8(buf);
}

inline void writeStringList(QDataStream& stream, const QStringList& list) {
    stream << static_cast<quint32>(list.size());
    for (const QString& str : list) {
        writeString(stream, str);
    }
}

inline QStringList readStringList(QDataStream& stream) {
    quint32 count;
    stream >> count;
    QStringList result;
    result.reserve(count);
    for (quint32 i = 0; i < count; ++i) {
        result.append(readString(stream));
    }
    return result;
}

} // namespace RProtocol

#endif // RPROTOCOL_H
