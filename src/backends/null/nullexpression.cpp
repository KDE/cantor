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

#include "nullexpression.h"

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include <kdebug.h>
#include <QTimer>
#include <QStringList>

NullExpression::NullExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();
    m_timer=new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(evalFinished()));
}

NullExpression::~NullExpression()
{

}


void NullExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);

    m_timer->start(1000);
}

void NullExpression::interrupt()
{
    kDebug()<<"interruptinging command";
    m_timer->stop();
    setStatus(MathematiK::Expression::Interrupted);
}

void NullExpression::evalFinished()
{
    kDebug()<<"evaluation finished";
    if ( command()=="img" )
        setResult( new MathematiK::ImageResult( KUrl("/home/alex/out.png"), "alternativ" ) );
    else if (command().startsWith("context: "))
        setResult( new MathematiK::ContextHelpResult(QStringList()<<"Not supported"));
    else
        setResult(new MathematiK::TextResult("result: "+command()));

    setStatus(MathematiK::Expression::Done);
}

#include "nullexpression.moc"
