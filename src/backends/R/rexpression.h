/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _REXPRESSION_H
#define _REXPRESSION_H

#include "expression.h"

class RExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    enum ServerReturnCode{SuccessCode=0, ErrorCode, InterruptedCode};
    explicit RExpression( Cantor::Session*, bool internal = false);
    ~RExpression() override = default;

    void evaluate() override;
    void interrupt() override;
    void parseOutput(const QString&);
    void parseError(const QString&);
    void showFilesAsResult(const QStringList&);

    void addInformation(const QString&) override;

  private:
    bool m_isHelpRequest{false};
};

#endif /* _REXPRESSION_H */
