/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2026 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _REXPRESSION_H
#define _REXPRESSION_H

#include "expression.h"

namespace Cantor { class TextResult; }

class RExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    enum ServerReturnCode{SuccessCode = 0, ErrorCode, InterruptedCode};
    explicit RExpression(Cantor::Session*, bool internal = false);
    ~RExpression() override = default;

    void evaluate() override;
    void interrupt() override;
    void parseOutput(const QString&) override;
    void parsePartialOutput(const QString&);
    void showFilesAsResult(const QStringList&);

    void addInformation(const QString&) override;

  private:
    QString formatHelpText(const QString& text);

    bool m_isHelpRequest{false};
    QString m_partialOutput;
    Cantor::TextResult* m_partialResult{nullptr};
};

#endif /* _REXPRESSION_H */
