/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _LOADEDEXPRESSION_H
#define _LOADEDEXPRESSION_H

#include "lib/expression.h"

#include <QIODevice>
#include <KZip>
#include <QDomElement>

class QJsonObject;

/** This class is used to hold expressions
    loaded from a file. they can't be evaluated
    and only show the result, they loaded from xml.
    this is used to avoid most exceptions when
    dealing with loaded Worksheets instead of newly
    created ones.
**/
class LoadedExpression : public Cantor::Expression
{
  public:
    explicit LoadedExpression( Cantor::Session* session );
    ~LoadedExpression() override = default;

    void evaluate() override;
    void interrupt() override;

    void parseOutput(const QString&) override {};
    void parseError(const QString&) override {};

    void loadFromXml(const QDomElement& xml, const KZip& file);
    void loadFromJupyter(const QJsonObject& cell);
};

#endif /* _LOADEDEXPRESSION_H */
