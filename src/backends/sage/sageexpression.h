/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _SAGEEXPRESSION_H
#define _SAGEEXPRESSION_H

#include "expression.h"

class SageExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    explicit SageExpression(Cantor::Session*, bool internal = false);

    void evaluate() override;
    void parseOutput(const QString&) override;
    void parseError(const QString&) override;

    void addFileResult(const QString&);

    void onProcessError(const QString&);

  public Q_SLOTS:
    void evalFinished();

  protected:
    QString additionalLatexHeaders() override;

  private:
    QString m_outputCache;
    QString m_imagePath;
    bool m_isHelpRequest;
    int m_promptCount;
    bool m_syntaxError;
};

#endif /* _SAGEEXPRESSION_H */
