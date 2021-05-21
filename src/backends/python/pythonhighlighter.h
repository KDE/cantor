/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2013 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONHIGHLIGHTER_H
#define _PYTHONHIGHLIGHTER_H

#include <QRegularExpression>

#include "defaulthighlighter.h"
class PythonSession;

class PythonHighlighter : public Cantor::DefaultHighlighter
{
    Q_OBJECT

  public:
    explicit PythonHighlighter(QObject* parent, PythonSession* session);
    ~PythonHighlighter() override = default;

  protected:
    void highlightBlock(const QString& text) override;

  private:
     QRegularExpression commentStartExpression;
     QRegularExpression commentEndExpression;
};

#endif /* _PYTHONHIGHLIGHTER_H */
