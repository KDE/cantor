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

#ifndef _SAGEEXPRESSION_H
#define _SAGEEXPRESSION_H

#include "expression.h"

class SageExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    SageExpression( Cantor::Session* session);
    ~SageExpression() override;

    void evaluate() Q_DECL_OVERRIDE;
    void interrupt() Q_DECL_OVERRIDE;

    void parseOutput(const QString& text);
    void parseError(const QString& text);

    void addFileResult(const QString& path);

    void onProcessError(const QString& msg);
  public Q_SLOTS:
    void evalFinished();

  protected:
    QString additionalLatexHeaders() Q_DECL_OVERRIDE;
  private:
    QString m_outputCache;
    QString m_imagePath;
    bool m_isHelpRequest;
    int m_promptCount;
};

#endif /* _SAGEEXPRESSION_H */
