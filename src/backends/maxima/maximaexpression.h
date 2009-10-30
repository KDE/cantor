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

class KTemporaryFile;
class QTimer;

class MaximaExpression : public Cantor::Expression
{
  Q_OBJECT  
  public:
    enum Type{NormalExpression, HelpExpression, TexExpression};
    explicit MaximaExpression( Cantor::Session* session, Type = NormalExpression);
    ~MaximaExpression();

    Type type();
    void setType(Type type);

    void evaluate();
    void interrupt();

    void addInformation(const QString& information);

    void parseOutput(const QString& text);
    void parseNormalOutput(const QString& text);
    void parseTexResult(const QString& text);

    bool needsLatexResult();

    //returns the command that should be send to 
    //the Maxima process, it's different from the
    //command() for example to allow plot embedding 
    QString internalCommand();

    //Forces the status of this Expression to done
    void forceDone();
    
  public slots:
    void evalFinished();

  private slots:
    void askForInformation();
    void imageChanged();
  private:
    Type m_type;
    QString m_outputCache;
    QString m_errCache;
    bool m_onStdoutStroke;
    KTemporaryFile *m_tempFile;
    KDirWatch m_fileWatch;
    bool m_isHelpRequest; 
    bool m_isPlot;
    QTimer* m_askTimer;
};

#endif /* _MAXIMAEXPRESSION_H */
