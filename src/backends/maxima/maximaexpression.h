/*
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

    ---
    Copyright (C) 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMAEXPRESSION_H
#define _MAXIMAEXPRESSION_H

#include "expression.h"
#include <QStringList>
#include <QFileSystemWatcher>

class QTemporaryFile;

class MaximaExpression : public Cantor::Expression
{
  Q_OBJECT

public:
    explicit MaximaExpression(Cantor::Session*, bool internal = false);
    ~MaximaExpression() override;

    void evaluate() override;
    void interrupt() override;

    QString internalCommand() override;

    //Forces the status of this Expression to done
    void forceDone();

    //reads from @param out until a prompt indicates that a new expression has started
    bool parseOutput(QString&);
    void parseError(const QString&);

    void addInformation(const QString&) override;

private Q_SLOTS:
    void imageChanged();

private:
    void parseResult(const QString&);

    QTemporaryFile *m_tempFile;
    QFileSystemWatcher m_fileWatch;
    bool m_isHelpRequest;
    bool m_isHelpRequestAdditional;
    bool m_isPlot;
    Cantor::Result* m_plotResult;
    int m_plotResultIndex;
    QString m_errorBuffer;
    bool m_gotErrorContent;
};

#endif /* _MAXIMAEXPRESSION_H */
