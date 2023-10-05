/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalgebrasyntaxhelpobject.h"
#include "kalgebrasession.h"
#include <analitzagui/operatorsmodel.h>
#include <KLocalizedString>

KAlgebraSyntaxHelpObject::KAlgebraSyntaxHelpObject(const QString& command, KAlgebraSession* session)
    : SyntaxHelpObject(command, session)
{}

void KAlgebraSyntaxHelpObject::fetchInformation()
{
    OperatorsModel* opm=static_cast<KAlgebraSession*>(session())->operatorsModel();
    QModelIndexList idxs=opm->match(opm->index(0,0), Qt::DisplayRole, command(), 1, Qt::MatchExactly);
    Q_ASSERT(idxs.size()<=1);

    if(!idxs.isEmpty()) {
        QString result;
        QModelIndex idx=idxs.first();
        int c=opm->columnCount(idx);
        for(int i=0; i<c; i++) {
            result += i18n("<p><b>%1:</b> %2</p>",
                           opm->headerData(i, Qt::Horizontal).toString(),
                           opm->data(idx.sibling(idx.row(), i)).toString());
        }

        setHtml(result);
        Q_EMIT done();
    }
}

