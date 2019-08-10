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
    Copyright (C) 2011 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _LATEXRENDERER_H
#define _LATEXRENDERER_H

#include <QObject>
#include "cantor_export.h"

namespace Cantor{
class LatexRendererPrivate;

class CANTOR_EXPORT LatexRenderer : public QObject
{
  Q_OBJECT
  public:
    enum Method{ LatexMethod = 0, MmlMethod = 1};
    enum EquationType{ InlineEquation = 0, FullEquation = 1};
    explicit LatexRenderer( QObject* parent = nullptr);
    ~LatexRenderer() override;

    QString latexCode() const;
    void setLatexCode(const QString& src);
    QString header() const;
    void addHeader(const QString& header);
    void setHeader(const QString& header);
    Method method() const;
    void setMethod( Method method);
    void setEquationOnly(bool isEquationOnly);
    bool isEquationOnly() const;
    void setEquationType(EquationType type);
    EquationType equationType() const;

    QString errorMessage() const;
    bool renderingSuccessful() const;

    QString imagePath() const;
    QString uuid() const;

    static QString genUuid();

  Q_SIGNALS:
    void done();
    void error();

  public Q_SLOTS:
    bool render();

    void renderBlocking();

  private:
    void setErrorMessage(const QString& msg);

  private Q_SLOTS:
    bool renderWithLatex();
    bool renderWithMml();
    void convertToPs();
    void convertingDone();

  private:
    LatexRendererPrivate* d;
};
}
#endif /* _LATEXRENDERER_H */
