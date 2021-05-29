/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#include "syntaxhelpobject.h"
using namespace Cantor;

#include "session.h"

#include <QTimer>

class Cantor::SyntaxHelpObjectPrivate{
    public:
        QString command;
        Cantor::Session* session;
        QString htmlResult;
};

SyntaxHelpObject::SyntaxHelpObject(const QString& command, Cantor::Session* session) : QObject(session),
                                                                                       d(new SyntaxHelpObjectPrivate)
{
    d->command=command;
    d->session=session;
}

SyntaxHelpObject::~SyntaxHelpObject()
{
    delete d;
}

void SyntaxHelpObject::fetchSyntaxHelp()
{
    //Start a delayed fetch
    QTimer::singleShot(0, this, &SyntaxHelpObject::fetchInformation);
}


QString SyntaxHelpObject::toHtml()
{
    return d->htmlResult;
}

void SyntaxHelpObject::setHtml(const QString& result)
{
    d->htmlResult=result;
}

QString SyntaxHelpObject::command()
{
    return d->command;
}

Session* SyntaxHelpObject::session()
{
    return d->session;
}
