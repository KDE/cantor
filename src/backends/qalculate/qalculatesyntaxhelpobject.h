/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATESYNTAXHELPOBJECT_H
#define QALCULATESYNTAXHELPOBJECT_H

#include <syntaxhelpobject.h>

class QalculateSession;

class QalculateSyntaxHelpObject : public Cantor::SyntaxHelpObject
{
public:
    QalculateSyntaxHelpObject( const QString& command, QalculateSession* session );
    QString answer();

protected:
    void fetchInformation() override;

    void setPlotInformation();
    void setSaveVariablesInformation();
    void setLoadVariablesInformation();

    QString m_answer;
};

#endif // QALCULATESYNTAXHELPOBJECT_H
