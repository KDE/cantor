/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Alexander Rieder <alexanderrieder@gmail.com>
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
    enum EquationType{ InlineEquation = 0, FullEquation = 1, CustomEquation = 2};
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

    static bool isLatexAvailable();

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
