/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com
 */
#pragma once

#include <QObject>
#include <QString>

class JuliaServer: public QObject
{
    Q_OBJECT
public:
    JuliaServer(QObject *parent = nullptr);

    virtual ~JuliaServer();

public Q_SLOTS:
    Q_SCRIPTABLE void login(const QString &path) const;
    Q_SCRIPTABLE void runJuliaCommand(const QString &command);
    Q_SCRIPTABLE QString getOutput() const;
    Q_SCRIPTABLE QString getError() const;
    Q_SCRIPTABLE bool getWasException() const;

private:
    QString m_error;
    QString m_output;
    bool m_was_exception;
};
