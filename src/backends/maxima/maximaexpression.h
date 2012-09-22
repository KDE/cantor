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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _MAXIMAEXPRESSION_H
#define _MAXIMAEXPRESSION_H

#include "expression.h"
#include "kdirwatch.h"
#include <QStringList>
#include <QXmlStreamReader>

class KTemporaryFile;
class QTimer;


class MaximaExpression : public Cantor::Expression
{
  Q_OBJECT  
  public:
    explicit MaximaExpression( Cantor::Session* session);
    ~MaximaExpression();

    void evaluate();
    void interrupt();

    void addInformation(const QString& information);

    bool needsLatexResult();

    //returns the command that should be send to 
    //the Maxima process, it's different from the
    //command() for example to allow plot embedding 
    QString internalCommand();

    //Forces the status of this Expression to done
    void forceDone();

    //reads from @param out until a prompt indicates that a new expression has started
    bool parseOutput(QString& out);

  private slots:
    void imageChanged();

  private:
    virtual QString additionalLatexHeaders();
    Cantor::Result* parseResult(int* idx,QString& out,QString& textBuffer,QString& latexBuffer);
  private:
    KTemporaryFile *m_tempFile;
    KDirWatch m_fileWatch;
    bool m_isHelpRequest; 
    bool m_isPlot;
    QTimer* m_askTimer;
};

#endif /* _MAXIMAEXPRESSION_H */
