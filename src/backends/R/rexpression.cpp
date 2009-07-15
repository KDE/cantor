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
#include "rcallbacks.h"

#include <kdebug.h>
#include <QTimer>
#include <QStringList>

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <Rinterface.h>
#include <R_ext/Parse.h>


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

    setCurrentExpression(this);

    const QString& cmd=command();
    m_stdBuffer.clear();
    m_errBuffer.clear();

    //Code to evaluate an R function (taken from RInside library)
    ParseStatus status;
    SEXP cmdSexp,  cmdexpr = R_NilValue;
    SEXP result;
    int i,  errorOccurred;
    QByteArray memBuf;

    memBuf.append(cmd);

    PROTECT(cmdSexp = allocVector(STRSXP,  1));
    SET_STRING_ELT(cmdSexp,  0,  mkChar((char*)memBuf.data()));

    cmdexpr = PROTECT(R_ParseVector(cmdSexp,  -1,  &status,  R_NilValue));
    switch (status)
    {
        case PARSE_OK:
            kDebug()<<"PARSING "<<cmd<<" went OK";
            /* Loop is needed here as EXPSEXP might be of length > 1 */
            for (i = 0; i < length(cmdexpr); i++) {

                result = R_tryEval(VECTOR_ELT(cmdexpr,  i), NULL, &errorOccurred);
                if (errorOccurred)
                    kFatal()<<"Error occurred, handle later";

            }
            memBuf.clear();
            break;
        case PARSE_INCOMPLETE:
            /* need to read another line */
            break;
        case PARSE_NULL:
            kDebug()<<"ParseStatus is null: "<<status;
            break;
        case PARSE_ERROR:
            kDebug()<<"Parse Error: "<<cmd;
            break;
        case PARSE_EOF:
            kDebug()<<"ParseStatus is eof: "<<status;
            break;
        default:
            kDebug()<<"Parse status is not documented: "<<status;
            break;
    }
    UNPROTECT(2);

    kDebug()<<"done running";

    kDebug()<<"std: "<<m_stdBuffer<<" err: "<<m_errBuffer;
    //if the command didn't print anything on its own, print the result

    //TODO: handle some known result types like lists, matrices spearately
    //      to make the output look better, by using html (tables etc.)
    if(m_stdBuffer.isEmpty()&&m_errBuffer.isEmpty())
    {
        kDebug()<<"printing result...";
        Rf_PrintValue(result);
    }

    setCurrentExpression(0); //is this save?

    if(!m_errBuffer.isEmpty())
    {
        setErrorMessage(m_errBuffer);
        setStatus(MathematiK::Expression::Error);
    }
    else
    {
        setResult(new MathematiK::TextResult(m_stdBuffer));
        setStatus(MathematiK::Expression::Done);
    }
}

void RExpression::interrupt()
{
    kDebug()<<"interruptinging command";

    setStatus(MathematiK::Expression::Interrupted);
}

void RExpression::addStdOutput(const QString& txt)
{
    m_stdBuffer+=txt;
}

void RExpression::addErrorOutput(const QString& txt)
{
    m_errBuffer+=txt;
}

#include "rexpression.moc"
