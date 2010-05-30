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

#include "rserver.h"
#include <kio/netaccess.h>
#include "radaptor.h"
#include "rcallbacks.h"
#include "settings.h"

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <unistd.h>

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <Rinterface.h>
#include <R_ext/Parse.h>

RServer::RServer() : m_isInitialized(false),m_isCompletionAvailable(false)
{
    new RAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/R",  this);

    m_tmpDir=KGlobal::dirs()->saveLocation("tmp",  QString("cantor/rserver-%1").arg(getpid()));
    kDebug()<<"storing plots at "<<m_tmpDir;

    initR();
    m_status=RServer::Idle;
    m_isInitialized=true;
    emit ready();
}

RServer::~RServer()
{
    //delete the directory with old plots
    KIO::NetAccess::del(KUrl(m_tmpDir), NULL);
}

void RServer::initR()
{
    //Setup environment variables
    // generated as littler.h via from svn/littler/littler.R
    #include "renvvars.h"

    for (int i = 0; R_VARS[i] != NULL; i+= 2)
    {
        if (setenv(R_VARS[i], R_VARS[i+1], 1) != 0)
            kFatal()<<"ERROR: couldn't set/replace an R environment variable";
    }

    //R_SignalHandlers = 0;               // Don't let R set up its own signal handlers

    const char *R_argv[] = {"Cantor",  "--no-save",  "--no-readline",  "",  ""}; //--gui=none
    const char *R_argv_opt[] = {"--vanilla",  "--slave"};
    int R_argc = (sizeof(R_argv) - sizeof(R_argv_opt) ) / sizeof(R_argv[0]);

    Rf_initEmbeddedR(R_argc,  (char**) R_argv);

    R_ReplDLLinit();            // this is to populate the repl console buffers

    setupCallbacks(this);

    autoload();

    //Setting up some settings dependent stuff
    if(RServerSettings::self()->integratePlots())
    {
        kDebug()<<"integrating plots";
        newPlotDevice();
    }

    kDebug()<<"done initializing";
}

//Code from the RInside library
void RServer::autoload()
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
    for(i = 0; i < packc; ++i){
	idx += (i != 0)? packobjc[i-1] : 0;
	for (j = 0; j < packobjc[i]; ++j){
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
    
    /* Initialize the completion libraries if needed, adapted from sys-std.c of R */ 
    // TODO: should we do this or init on demand?
    // if (completion is needed) // TODO: discuss how to pass parameter
    {
        /* First check if namespace is loaded */
        if  (findVarInFrame(R_NamespaceRegistry,install("utils"))==R_UnboundValue)
        { /* Then try to load it */
            SEXP cmdSexp, cmdexpr;
            ParseStatus status;
            int i;
            const char *p="try(loadNamespace('rcompgen'), silent=TRUE)";

            PROTECT(cmdSexp=mkString(p));
            cmdexpr=PROTECT(R_ParseVector(cmdSexp,-1,&status,R_NilValue));
            if(status==PARSE_OK)
            {
                for(i=0;i<length(cmdexpr);i++)
                    eval(VECTOR_ELT(cmdexpr,i),R_GlobalEnv);
            }
            UNPROTECT(2);
            /* Completion is available if the namespace is correctly loaded */
            m_isCompletionAvailable= (findVarInFrame(R_NamespaceRegistry,install("utils"))!=R_UnboundValue);
        }
    }
}

void RServer::endR()
{
   Rf_endEmbeddedR(0);
}

void RServer::runCommand(const QString& cmd, bool internal)
{
    kDebug()<<"running command "<<cmd;
    Expression* expr=new Expression;
    expr->cmd=cmd;
    expr->hasOtherResults=false;

    setStatus(RServer::Busy);

    setCurrentExpression(expr);

    expr->std_buffer.clear();
    expr->err_buffer.clear();

    ReturnCode returnCode=RServer::SuccessCode;
    QString returnText;

    //Code to evaluate an R function (taken from RInside library)
    ParseStatus status;
    SEXP cmdSexp,  cmdexpr = R_NilValue;
    SEXP result;
    int i,  errorOccurred;
    QByteArray memBuf;

    memBuf.append(cmd.toUtf8());

    PROTECT(cmdSexp = allocVector(STRSXP,  1));
    SET_STRING_ELT(cmdSexp,  0,  mkChar((char*)memBuf.data()));

    cmdexpr = PROTECT(R_ParseVector(cmdSexp,  -1,  &status,  R_NilValue));
    switch (status)
    {
        case PARSE_OK:
            kDebug()<<"PARSING "<<cmd<<" went OK";
            /* Loop is needed here as EXPSEXP might be of length > 1 */
            for (i = 0; i < length(cmdexpr); ++i) {

                result = R_tryEval(VECTOR_ELT(cmdexpr,  i), NULL, &errorOccurred);
                if (errorOccurred)
                {
                    kDebug()<<"Error occurred.";
                    break;
                }

            }
            memBuf.clear();
            break;
        case PARSE_INCOMPLETE:
            /* need to read another line */
            kDebug()<<"parse incomplete..";
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

    if(status==PARSE_OK)
    {
        kDebug()<<"done running";

        kDebug()<<"std: "<<expr->std_buffer<<" err: "<<expr->err_buffer;
        //if the command didn't print anything on its own, print the result

        //TODO: handle some known result types like lists, matrices spearately
        //      to make the output look better, by using html (tables etc.)
        if(expr->std_buffer.isEmpty()&&expr->err_buffer.isEmpty())
        {
            kDebug()<<"printing result...";
            Rf_PrintValue(result);
        }

        setCurrentExpression(0); //is this save?

        if(!expr->err_buffer.isEmpty())
        {
            returnCode=RServer::ErrorCode;
            returnText=expr->err_buffer;
        }
        else
        {
            returnCode=RServer::SuccessCode;
            returnText=expr->std_buffer;

        }
    }else
    {
        returnCode=RServer::ErrorCode;
        returnText=i18n("Error Parsing Command");
    }

    if(internal)
    {
        kDebug()<<"internal result: "<<returnCode<<" :: "<<returnText;
        return;
    }

    QFileInfo f(m_curPlotFile);
    kDebug()<<"file: "<<m_curPlotFile<<" exists: "<<f.exists()<<" size: "<<f.size();
    if(f.exists())
    {
        expr->hasOtherResults=true;
        newPlotDevice();
        showFiles(QStringList()<<f.filePath());
    }

    //Check if the expression got results other than plain text (like files,etc)
    if(!expr->hasOtherResults)
        emit expressionFinished(returnCode, returnText);

    setStatus(Idle);
}

void RServer::completeCommand(const QString& cmd)
{
    // TODO: is static okay? guess RServer is a singletone, but ...
    // TODO: error handling?
    // TODO: investigate encoding problem
    // TODO: propage the flexibility of token selection upward
    // TODO: what if install() fails? investigate
    // TODO: investigate why errors break the whole foodchain of RServer callbacks in here
    // TODO: make 100% sure on how to set linebuffer end
    static SEXP comp_env=R_FindNamespace(mkString("utils"));
    static SEXP tokenizer_func=install(".guessTokenFromLine");
    static SEXP linebuffer_func=install(".assignLinebuffer");
    static SEXP buffer_end_func=install(".assignEnd");
    static SEXP complete_func=install(".completeToken");
    static SEXP retrieve_func=install(".retrieveCompletions");
    
    /* Setting buffer parameters */
    int errorOccurred=0; // TODO: error cheks, too lazy to do it now
    R_tryEval(lang2(linebuffer_func,mkString(cmd.toUtf8().data())),comp_env,&errorOccurred);
    R_tryEval(lang2(buffer_end_func,ScalarInteger(cmd.size()-1)),comp_env,&errorOccurred);
    
    /* Passing the tokenizing work to professionals */
    R_tryEval(lang1(tokenizer_func),comp_env,&errorOccurred);
    
    /* Doing the actual stuff */
    R_tryEval(lang1(complete_func),comp_env,&errorOccurred);
    SEXP completions=PROTECT(R_tryEval(lang1(retrieve_func),comp_env,&errorOccurred));
    
    /* Populating the list of completions */
    QStringList completionOptions;
    for (int i=0;i<length(completions);i++)
        completionOptions<<translateCharUTF8(STRING_ELT(completions,i));
    UNPROTECT(1);
    
    emit completionFinished(completionOptions);
}

void RServer::setStatus(Status status)
{
    if(m_status!=status)
    {
        m_status=status;
        if(m_isInitialized)
            emit statusChanged(status);
    }
}

QString RServer::requestInput(const QString& prompt)
{
    emit inputRequested(prompt);

    //Wait until the input arrives over dbus
    QEventLoop loop;
    connect(this, SIGNAL(requestAnswered()), &loop, SLOT(quit()));
    loop.exec();

    return m_requestCache;
}

void RServer::answerRequest(const QString& answer)
{
    m_requestCache=answer;

    emit(requestAnswered());
}

void RServer::showFiles(const QStringList& files)
{
    if(m_isInitialized)
        emit showFilesNeeded(files);
}

void RServer::newPlotDevice()
{
    static int deviceNum=0;
    //For some reason, the PostScript returned by R doesn't seem to render,
    //so fallback to png

    /*static const QString psCommand=QString("pdf(horizontal = FALSE, onefile = TRUE, " \
                                           "paper=\"special\", width=5, height=4,"\
                                           "print.it=FALSE, bg=\"white\", file=\"%1\" )");*/
    static const QString command=QString("png(filename=\"%1\", width = 480, height = 480, units = \"px\")");
    m_curPlotFile=QString("%1/Rplot%2.png").arg(m_tmpDir, QString::number(deviceNum++));
    if(m_isInitialized)
        runCommand("dev.off()", true);
    runCommand(command.arg(m_curPlotFile), true);
}

#include "rserver.moc"
