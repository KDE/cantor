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
    QTimer::singleShot(0, this, SLOT(fetchInformation()));
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
