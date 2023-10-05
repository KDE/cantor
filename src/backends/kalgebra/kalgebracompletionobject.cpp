/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalgebracompletionobject.h"

#include <QStringList>

#include "kalgebrasession.h"
#include <analitzagui/operatorsmodel.h>

KAlgebraCompletionObject::KAlgebraCompletionObject(const QString& command, int index, KAlgebraSession* session)
    : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

void KAlgebraCompletionObject::fetchCompletions()
{
    OperatorsModel* opm=static_cast<KAlgebraSession*>(session())->operatorsModel();

    const QModelIndexList idxs = opm->match(opm->index(0,0), Qt::DisplayRole, command(), 5, Qt::MatchStartsWith);
    QStringList comp;
    for(const QModelIndex& idx : idxs)
        comp << idx.data().toString();

    setCompletions(comp);
    Q_EMIT fetchingDone();
}

bool KAlgebraCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter();
}
