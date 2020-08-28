/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#include "rhighlighter.h"
#include "rkeywords.h"
#include "rsession.h"
#include "rvariablemodel.h"

#include <QTextEdit>
#include <QDebug>

const QStringList RHighlighter::operators_list=QStringList()
   << QLatin1String("(\\+|\\-|\\*{1,2}|/|&lt;=?|&gt;=?|={1,2}|\\!=?|\\|{1,2}|&amp;{1,2}|:{1,3}|\\^|@|\\$|~)")
   << QLatin1String("%[^%]*%"); // Taken in r.xml syntax file from KSyntaxHighlighter

const QStringList RHighlighter::specials_list=QStringList()
    << QLatin1String("BUG") << QLatin1String("TODO") << QLatin1String("FIXME") << QLatin1String("NB") << QLatin1String("WARNING") << QLatin1String("ERROR");

RHighlighter::RHighlighter(QObject* parent, RSession* session) : Cantor::DefaultHighlighter(parent, session)
{
    Cantor::DefaultVariableModel* model = session->variableModel();
    if (model)
    {
        RVariableModel* RModel = static_cast<RVariableModel*>(model);
        connect(RModel, &RVariableModel::constantsAdded, this, &RHighlighter::addVariables);
        connect(RModel, &RVariableModel::constantsRemoved, this, &RHighlighter::removeRules);
    }

    addKeywords(RKeywords::instance()->keywords());

    foreach (const QString& s, operators_list)
        addRule(QRegularExpression(s), operatorFormat());
    foreach (const QString& s, specials_list)
        addRule(QRegularExpression(QLatin1String("\\b")+s+QLatin1String("\\b")), commentFormat());

    addRule(QRegularExpression(QStringLiteral("\"[^\"]*\"")), stringFormat());
    addRule(QRegularExpression(QStringLiteral("'[^']*'")), stringFormat());

    addRule(QRegularExpression(QStringLiteral("#[^\n]*")), commentFormat());
}

QStringList RHighlighter::parseBlockTextToWords(const QString& originalText)
{
    QString text = originalText;

    static const QString replacer1 = QLatin1String("___CANTOR_R_REPLACER_1___");
    static const QString replacer2 = QLatin1String("___CANTOR_R_REPLACER_2___");

    text.replace(QLatin1String("-"), replacer1);
    text.replace(QLatin1String("."), replacer2);

    QStringList words = text.split(QRegularExpression(QStringLiteral("\\b")), QString::SkipEmptyParts);

    for (int i = 0; i < words.size(); i++)
    {
        words[i].replace(replacer1, QLatin1String("-"));
        words[i].replace(replacer2, QLatin1String("."));
    }

    return words;
}
