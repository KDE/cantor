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
#include "textresult.h"

//#include "imageresult.h"
//#include "helpresult.h"
#include "rsession.h"
#include "rexpression.h"

#include <kdebug.h>
#include <klocale.h>
#include <QTimer>
#include <QStringList>

RExpression::RExpression( MathematiK::Session* session ) : MathematiK::Expression(session)
{
    kDebug();

}

RExpression::~RExpression()
{

}


void RExpression::evaluate()
{
    kDebug()<<"evaluating "<<command();
    setStatus(MathematiK::Expression::Computing);

    static_cast<RSession*>(session())->queueExpression(this);
}

void RExpression::interrupt()
{
    kDebug()<<"interruptinging command";
    if(status()==MathematiK::Expression::Computing)
        session()->interrupt();
    setStatus(MathematiK::Expression::Interrupted);
}

void RExpression::finished(int returnCode, const QString& text)
{
    if(returnCode==RExpression::SuccessCode)
    {
        setResult(new MathematiK::TextResult(text));
        setStatus(MathematiK::Expression::Done);
    }else if (returnCode==RExpression::ErrorCode)
    {
        setResult(new MathematiK::TextResult(text));
        setStatus(MathematiK::Expression::Error);
        setErrorMessage(text);
    }
}

void RExpression::evaluationStarted()
{
    setStatus(MathematiK::Expression::Computing);
}


#include "rexpression.moc"
