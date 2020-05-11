/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.
*/

#ifndef OCTAVEEXPRESSION_H
#define OCTAVEEXPRESSION_H

#include <config-cantorlib.h>

#include <expression.h>
#include <QStringList>

class QTemporaryFile;

class OctaveExpression : public Cantor::Expression
{
    Q_OBJECT

public:
    explicit OctaveExpression(Cantor::Session*, bool internal = false);
    ~OctaveExpression();

    void interrupt() override;
    void evaluate() override;
    QString internalCommand() override;

    void parseOutput(const QString&);
    void parseError(const QString&);
    void imageChanged();

public:
    const static QStringList plotExtensions;

private:
    QString m_resultString;
    bool m_finished = false;
    bool m_plotPending = false;
    QTemporaryFile* m_tempFile = nullptr;
};

#endif // OCTAVEEXPRESSION_H
