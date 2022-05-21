/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 by Alexander Semke (alexander.semke@web.de)
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
    void parseOutput(const QString&) override;
    void parseError(const QString&) override;

    void addInformation(const QString&) override;

private Q_SLOTS:
    void imageChanged();

private:
    void parseResult(const QString&);

    QTemporaryFile* m_tempFile = nullptr;
    QFileSystemWatcher m_fileWatch;
    bool m_isHelpRequest = false;
    bool m_isHelpRequestAdditional = false;
    bool m_isPlot = false;
    bool m_isDraw = false;
    Cantor::Result* m_plotResult = nullptr;
    int m_plotResultIndex = -1;
    QString m_errorBuffer;
    bool m_gotErrorContent = false;
};

#endif /* _MAXIMAEXPRESSION_H */
