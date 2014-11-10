/*************************************************************************************
 *  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "kalgebrasyntaxhelpobject.h"
#include "kalgebrasession.h"
#include <analitzagui/operatorsmodel.h>
#include <KLocale>

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
        emit done();
    }
}

