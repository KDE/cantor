/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABHIGHLIGHTER_H
#define _SCILABHIGHLIGHTER_H

#include <QRegularExpression>

#include "defaulthighlighter.h"
#include "scilabexpression.h"

class ScilabHighlighter : public Cantor::DefaultHighlighter
{
    Q_OBJECT

    public:
        ScilabHighlighter(QObject* parent, Cantor::Session* session);
        ~ScilabHighlighter() override = default;

    protected:
        void highlightBlock(const QString&) override;
        QString nonSeparatingCharacters() const override;

    private:
        Cantor::Session* m_session;
        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;
};

#endif /* _SCILABHIGHLIGHTER_H */
