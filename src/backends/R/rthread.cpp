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

#include "rthread.h"

#include "rsession.h"
#include "rexpression.h"
#include "rcallbacks.h"

#include <kdebug.h>

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <Rinterface.h>
#include <R_ext/Parse.h>


RThread::RThread(RSession* parent) : QThread(parent)
{
    m_session=parent;
    connect(this, SIGNAL(expressionFinished(RExpression*)), this, SLOT(evaluateExpression()));

    //Setup The R environment. this HAS to happen in the main Thread
    initR();
}

RThread::~RThread()
{
    //Cleanup the R environment
    endR();
}

void RThread::initR()
{
    //Setup environment variables
    // generated as littler.h via from svn/littler/littler.R
    #include "renvvars.h"

    for (int i = 0; R_VARS[i] != NULL; i+= 2)
    {
        if (setenv(R_VARS[i], R_VARS[i+1], 1) != 0)
            kFatal()<<"ERROR: couldn't set/replace an R environment variable";
    }

    R_SignalHandlers = 0;               // Don't let R set up its own signal handlers

    const char *R_argv[] = {"MathematiK",  "--no-save",  "--no-readline",  "",  ""}; //--gui=none
    const char *R_argv_opt[] = {"--vanilla",  "--slave"};
    int R_argc = (sizeof(R_argv) - sizeof(R_argv_opt) ) / sizeof(R_argv[0]);

    Rf_initEmbeddedR(R_argc,  (char**) R_argv);

    R_ReplDLLinit();            // this is to populate the repl console buffers

    setupCallbacks(this);

    autoload();

    kDebug()<<"done initializing";

}

//Code from the RInside library
void RThread::autoload()
{
    #include "rautoloads.h"

    /* Autoload default packages and names from autoloads.h
     *
     * This function behaves in almost every way like
     * R's autoload:
     * function (name, package, reset = FALSE, ...)
     * {
     *     if (!reset && exists(name, envir = .GlobalEnv, inherits = FALSE))
     *        stop("an object with that name already exists")
     *     m <- match.call()
     *     m[[1]] <- as.name("list")
     *     newcall <- eval(m, parent.frame())
     *     newcall <- as.call(c(as.name("autoloader"), newcall))
     *     newcall$reset <- NULL
     *     if (is.na(match(package, .Autoloaded)))
     *        assign(".Autoloaded", c(package, .Autoloaded), env = .AutoloadEnv)
     *     do.call("delayedAssign", list(name, newcall, .GlobalEnv,
     *                                                         .AutoloadEnv))
     *     invisible()
     * }
     *
     * What's missing is the updating of the string vector .Autoloaded with
     * the list of packages, which by my code analysis is useless and only
     * for informational purposes.
     *
     */
    //void autoloads(void){

    SEXP da, dacall, al, alcall, AutoloadEnv, name, package;
    int i,j, idx=0, errorOccurred, ptct;

    /* delayedAssign call*/
    PROTECT(da = Rf_findFun(Rf_install("delayedAssign"), R_GlobalEnv));
    PROTECT(AutoloadEnv = Rf_findVar(Rf_install(".AutoloadEnv"), R_GlobalEnv));
    if (AutoloadEnv == R_NilValue){
        kError()<<"Cannot find .AutoloadEnv";
	//exit(1);
    }
    PROTECT(dacall = allocVector(LANGSXP,5));
    SETCAR(dacall,da);
    /* SETCAR(CDR(dacall),name); */          /* arg1: assigned in loop */
    /* SETCAR(CDR(CDR(dacall)),alcall); */  /* arg2: assigned in loop */
    SETCAR(CDR(CDR(CDR(dacall))),R_GlobalEnv); /* arg3 */
    SETCAR(CDR(CDR(CDR(CDR(dacall)))),AutoloadEnv); /* arg3 */

    /* autoloader call */
    PROTECT(al = Rf_findFun(Rf_install("autoloader"), R_GlobalEnv));
    PROTECT(alcall = allocVector(LANGSXP,3));
    SET_TAG(alcall, R_NilValue); /* just like do_ascall() does */
    SETCAR(alcall,al);
    /* SETCAR(CDR(alcall),name); */          /* arg1: assigned in loop */
    /* SETCAR(CDR(CDR(alcall)),package); */  /* arg2: assigned in loop */

    ptct = 5;
    for(i = 0; i < packc; i++){
	idx += (i != 0)? packobjc[i-1] : 0;
	for (j = 0; j < packobjc[i]; j++){
	    /*printf("autload(%s,%s)\n",packobj[idx+j],pack[i]);*/

	    PROTECT(name = NEW_CHARACTER(1));
	    PROTECT(package = NEW_CHARACTER(1));
	    SET_STRING_ELT(name, 0, COPY_TO_USER_STRING(packobj[idx+j]));
	    SET_STRING_ELT(package, 0, COPY_TO_USER_STRING(pack[i]));

	    /* Set up autoloader call */
	    PROTECT(alcall = allocVector(LANGSXP,3));
	    SET_TAG(alcall, R_NilValue); /* just like do_ascall() does */
	    SETCAR(alcall,al);
	    SETCAR(CDR(alcall),name);
	    SETCAR(CDR(CDR(alcall)),package);

	    /* Setup delayedAssign call */
	    SETCAR(CDR(dacall),name);
	    SETCAR(CDR(CDR(dacall)),alcall);

	    R_tryEval(dacall,R_GlobalEnv,&errorOccurred);
	    if (errorOccurred){
                kError()<<"Error calling delayedAssign!";
                //exit(1);
	    }

	    ptct += 3;
	}
    }
    UNPROTECT(ptct);
}

void RThread::endR()
{
    Rf_endEmbeddedR(0);
}

void RThread::run()
{
    kDebug()<<"running R-Thread";

    emit ready();

    kDebug()<<"ready";
    exec();

    //delete m_r;
}

void RThread::queueExpression(RExpression* expr)
{
    m_queue.append(expr);
    if(m_queue.size()==1)
        evaluateExpression();
}

void RThread::evaluateExpression()
{
    if(m_queue.isEmpty())
        return;

    RExpression* expr=m_queue.first();
    kDebug()<<"evaluating "<<expr->command();
    expr->evaluate();

    m_queue.takeFirst();

    emit expressionFinished(expr);
}

bool RThread::isBusy()
{
    kDebug()<<m_queue.size();
    return m_queue.size()>0;
}

#include "rthread.moc"
