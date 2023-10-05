/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2010 Oleksiy Protas <elfy.ua@gmail.com>
    SPDX-FileCopyrightText: 2023 by Alexander Semke (alexander.semke@web.de)
*/

// TODO: setStatus in syntax and completions, to be or not to be?
// on the one hand comme il faut, on another, causes flickering in UI

#include "rserver.h"
#include "radaptor.h"
#include "rcallbacks.h"
#include "settings.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QUrl>

#include <KIO/DeleteJob>
#include <KLocalizedString>

#ifdef Q_OS_WIN
#include <process.h>
#else
#include <unistd.h>
#endif

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <R_ext/Parse.h>

const QChar RServer::recordSep(30);
const QChar RServer::unitSep(31);


RServer::RServer() : m_isInitialized(false),m_isCompletionAvailable(false)
{
    new RAdaptor(this);

    m_tmpDir = QDir::tempPath() + QString::fromLatin1("/cantor_rserver-%1").arg(getpid());
    QDir dir;
    dir.mkdir(m_tmpDir);
    qDebug()<<"RServer: "<<"storing plots at "<<m_tmpDir;

    initR();
    m_status=RServer::Idle;
    m_isInitialized=true;
}

RServer::~RServer()
{
    //delete the directory with old plots
    KIO::del(QUrl(m_tmpDir));
}

void RServer::initR()
{
    //Setup environment variables
    // generated as littler.h via from svn/littler/littler.R
    #include "renvvars.h"

    for (int i = 0; R_VARS[i] != nullptr; i+= 2)
	    qputenv(R_VARS[i], R_VARS[i+1]);

    //R_SignalHandlers = 0;               // Don't let R set up its own signal handlers

    const char *R_argv[] = {"Cantor",  "--no-save",  "--no-readline",  "",  ""}; //--gui=none
    const char *R_argv_opt[] = {"--vanilla",  "--slave"};
    int R_argc = (sizeof(R_argv) - sizeof(R_argv_opt) ) / sizeof(R_argv[0]);

    Rf_initEmbeddedR(R_argc,  (char**) R_argv);

    R_ReplDLLinit();            // this is to populate the repl console buffers

    setupCallbacks(this);

    autoload();

    // Set gui editor for R
    runCommand(QLatin1String("options(editor = 'cantor_scripteditor') \n"),true);

    //Setting up some settings dependent stuff
    if(RServerSettings::self()->integratePlots())
    {
        qDebug()<<"RServer: "<<"integrating plots";
        newPlotDevice();
    }

    //Loading automatic run scripts
    foreach (const QString& path, RServerSettings::self()->autorunScripts())
    {
        int errorOccurred=0;
        if (QFile::exists(path))
            R_tryEval(lang2(install("source"),mkString(path.toUtf8().data())),nullptr,&errorOccurred);
        // TODO: error handling
        else
        {
            qDebug()<<"RServer: "<<(QLatin1String("Script ")+path+QLatin1String(" not found")); // FIXME: or should we throw a messagebox
        }
    }

    qDebug()<<"RServer: "<<"done initializing";
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
        qDebug()<<"RServer: "<<"Cannot find .AutoloadEnv";
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
                qDebug()<<"RServer: "<<"Error calling delayedAssign!";
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
                for(i=0;i<length(cmdexpr);++i)
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

void RServer::addFileToOutput(const QString& file)
{
    m_expressionFiles.append(file);
}

void RServer::runCommand(const QString& cmd, bool internal)
{
    m_expressionFiles.clear();
    qDebug()<<"RServer: "<<"running " << (internal ? "internal " : "") << "command "<<cmd;

    // Handle some internal command, like variable model update, etc.
    if (internal)
    {
        const QLatin1String completionCommandPrefix("%completion ");
        if (cmd == QLatin1String("%model update"))
        {
            listSymbols();
            return;
        }
        else if (cmd.startsWith(completionCommandPrefix))
        {

            QString arg = cmd;
            arg.remove(0, completionCommandPrefix.size());
            qDebug() << "arg" << arg;
            completeCommand(arg);
            return;
        }
    }

    Expression* expr=new Expression;
    expr->cmd=cmd;
    expr->hasOtherResults=false;

    setStatus(RServer::Busy);

    setCurrentExpression(expr);

    expr->std_buffer.clear();
    expr->err_buffer.clear();

    ReturnCode returnCode=RServer::SuccessCode;
    QString returnText;
    QStringList neededFiles;

    //Code to evaluate an R function (taken from RInside library)
    ParseStatus status;
    SEXP cmdSexp,  cmdexpr = R_NilValue;
    SEXP result = nullptr;
    int i,  errorOccurred;
    QByteArray memBuf;

    memBuf.append(cmd.toUtf8());

    PROTECT(cmdSexp = allocVector(STRSXP,  1));
    SET_STRING_ELT(cmdSexp,  0,  mkChar((char*)memBuf.data()));

    cmdexpr = PROTECT(R_ParseVector(cmdSexp,  -1,  &status,  R_NilValue));
    switch (status)
    {
        case PARSE_OK:
            qDebug()<<"RServer: "<<"PARSING "<<cmd<<" went OK";
            /* Loop is needed here as EXPSEXP might be of length > 1 */
            for (i = 0; i < length(cmdexpr); ++i) {

                result = R_tryEval(VECTOR_ELT(cmdexpr,  i), nullptr, &errorOccurred);
                if (errorOccurred)
                {
                    qDebug()<<"RServer: "<<"Error occurred.";
                    break;
                }
                // TODO: multiple results
            }
            memBuf.clear();
            break;
        case PARSE_INCOMPLETE:
            /* need to read another line */
            qDebug()<<"RServer: "<<"parse incomplete..";
            break;
        case PARSE_NULL:
            qDebug()<<"RServer: "<<"ParseStatus is null: "<<status;
            break;
        case PARSE_ERROR:
            qDebug()<<"RServer: "<<"Parse Error: "<<cmd;
            break;
        case PARSE_EOF:
            qDebug()<<"RServer: "<<"ParseStatus is eof: "<<status;
            break;
        default:
            qDebug()<<"RServer: "<<"Parse status is not documented: "<<status;
            break;
    }
    UNPROTECT(2);

    if(status==PARSE_OK)
    {
        qDebug()<<"RServer: "<<"done running";

        qDebug()<<"RServer: "<<"result: " << result << " std: "<<expr->std_buffer<<" err: "<<expr->err_buffer;
        //if the command didn't print anything on its own, print the result
        //but only, if result exists, because comment expression don't create result


        //TODO: handle some known result types like lists, matrices separately
        //      to make the output look better, by using html (tables etc.)
        if(result && expr->std_buffer.isEmpty()&&expr->err_buffer.isEmpty())
        {
            qDebug()<<"RServer: "<<"printing result...";
            SEXP count=PROTECT(R_tryEval(lang2(install("length"),result),nullptr,&errorOccurred)); // TODO: error checks
            if (*INTEGER(count)==0)
                qDebug()<<"RServer: " << "no result, so show nothing";
            else
                Rf_PrintValue(result);
            UNPROTECT(1);
        }


        setCurrentExpression(nullptr); //is this save?

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
        qDebug()<<"RServer: "<<"internal result: "<<returnCode<<" :: "<<returnText;
        return;
    }

    QFileInfo f(m_curPlotFile);
    qDebug()<<"RServer: "<<"file: "<<m_curPlotFile<<" exists: "<<f.exists()<<" size: "<<f.size();
    if(f.exists())
    {
        expr->hasOtherResults=true;
        newPlotDevice();
        neededFiles<<f.filePath();
    }

    qDebug()<<"RServer: " << "files: " << neededFiles+m_expressionFiles;
    Q_EMIT expressionFinished(returnCode, returnText, neededFiles+m_expressionFiles);

    setStatus(Idle);
}

void RServer::completeCommand(const QString& cmd)
{
    setStatus(RServer::Busy);

    // TODO: is static okay? guess RServer is a singletone, but ...
    // TODO: error handling?
    // TODO: investigate encoding problem
    // TODO: propage the flexibility of token selection upward
    // TODO: what if install() fails? investigate
    // TODO: investigate why errors break the whole foodchain of RServer callbacks in here
    static SEXP comp_env=R_FindNamespace(mkString("utils"));
    static SEXP tokenizer_func=install(".guessTokenFromLine");
    static SEXP linebuffer_func=install(".assignLinebuffer");
    static SEXP buffer_end_func=install(".assignEnd");
    static SEXP complete_func=install(".completeToken");
    static SEXP retrieve_func=install(".retrieveCompletions");

    /* Setting buffer parameters */
    int errorOccurred=0; // TODO: error cheks, too lazy to do it now
    R_tryEval(lang2(linebuffer_func,mkString(cmd.toUtf8().data())),comp_env,&errorOccurred);
    R_tryEval(lang2(buffer_end_func,ScalarInteger(cmd.size())),comp_env,&errorOccurred);

    /* Passing the tokenizing work to professionals */
    SEXP token=PROTECT(R_tryEval(lang1(tokenizer_func),comp_env,&errorOccurred));

    /* Doing the actual stuff */
    R_tryEval(lang1(complete_func),comp_env,&errorOccurred);
    SEXP completions=PROTECT(R_tryEval(lang1(retrieve_func),comp_env,&errorOccurred));

    /* Populating the list of completions */
    QStringList completionOptions;
    for (int i=0;i<length(completions);i++)
        completionOptions<<QLatin1String(translateCharUTF8(STRING_ELT(completions,i)));
    QString qToken=QLatin1String(translateCharUTF8(STRING_ELT(token,0)));
    UNPROTECT(2);

    const QString output = qToken + unitSep + completionOptions.join(recordSep);
    Q_EMIT expressionFinished(RServer::SuccessCode, output, QStringList());
    setStatus(RServer::Idle);
}

// FIXME: This scheme is somewhat placeholder, I honestly don't like it too much
// I am not sure whether or not asking the server with each keypress if what he typed was
// acceptable or not is a good idea. I'll leave it under investigation, let it be this way just for now
// ~Landswellsong

void RServer::listSymbols()
{
    setStatus(RServer::Busy);

    QStringList vars, values, funcs, constants;
    int errorOccurred; // TODO: error checks

    /* Obtaining a list of user namespace objects */
    SEXP usr=PROTECT(R_tryEval(lang1(install("ls")),nullptr,&errorOccurred));
    for (int i=0;i<length(usr);i++)
    {
        SEXP object = STRING_ELT(usr,i);
        const QString& name = QString::fromUtf8(translateCharUTF8(object));
        SEXP value = findVar(installChar(object), R_GlobalEnv);

        if (Rf_isFunction(value))
            funcs << name;
        else if (RServerSettings::variableManagement())
        {
            int convertStatus;
            SEXP valueAsString = PROTECT(R_tryEval(lang2(install("toString"),value),nullptr,&convertStatus));
            if (convertStatus == 0)
            {
                vars << name;
                values << QString::fromUtf8(translateCharUTF8(asChar(valueAsString)));
            }
        }
        else
            vars << name;
    }
    UNPROTECT(1);

    /* Obtaining a list of active packages */
    SEXP packages=PROTECT(R_tryEval(lang1(install("search")),nullptr,&errorOccurred));
    //int i=1; // HACK to prevent scalability issues
    for (int i=1;i<length(packages);i++) // Package #0 is user environment, so starting with 1
    {
        QString packageName = QString::fromUtf8(translateCharUTF8(STRING_ELT(packages,i)));

        if (!m_parsedNamespaces.contains(packageName))
        {
            CachedParsedNamespace cache;

            //char pos[32];
            //sprintf(pos,"%d",i+1);
            SEXP f=PROTECT(R_tryEval(lang2(install("ls"),ScalarInteger(i+1)),nullptr,&errorOccurred));
            for (int j=0;j<length(f);j++)
            {
                SEXP object = STRING_ELT(f,j);
                const QString& name = QString::fromUtf8(translateCharUTF8(object));
                SEXP value = installChar(object);
                int errorOccurred2 = 2;
                //TODO error handling
                //FIXME without this unused typeof evaling - server crash on certain symbols
                SEXP test = PROTECT(R_tryEval(lang2(install("typeof"), value),nullptr,&errorOccurred2));
                Q_UNUSED(test);

                SEXP resultIs = PROTECT(R_tryEval(lang2(install("is.function"), value),nullptr, &errorOccurred2));
                if (QString::fromUtf8(translateCharUTF8(asChar(resultIs))) == QLatin1String("TRUE"))
                    cache.functions << name;
                else
                    cache.constants << name;
            }
            UNPROTECT(1);

            m_parsedNamespaces[packageName] = cache;
        }

        funcs += m_parsedNamespaces[packageName].functions;
        constants += m_parsedNamespaces[packageName].constants;
    }
    UNPROTECT(1);

    const QString output = vars.join(recordSep) + unitSep + values.join(recordSep) + unitSep + funcs.join(recordSep) + unitSep + constants.join(recordSep);
    Q_EMIT expressionFinished(RServer::SuccessCode, output, QStringList());
    setStatus(Idle);
}

void RServer::setStatus(Status status)
{
    if(m_status!=status)
    {
        m_status=status;
        if(m_isInitialized)
            Q_EMIT statusChanged(status);
    }
}

QString RServer::requestInput(const QString& prompt)
{
    Q_EMIT inputRequested(prompt);

    //Wait until the input arrives over dbus
    QEventLoop loop;
    connect(this, SIGNAL(requestAnswered()), &loop, SLOT(quit()));
    loop.exec();

    return m_requestCache;
}

void RServer::answerRequest(const QString& answer)
{
    m_requestCache=answer;
    Q_EMIT requestAnswered();
}

void RServer::newPlotDevice()
{
    static int deviceNum = 0;

    QString extension;
    QString command;
    int w = RServerSettings::self()->plotWidth();
    int h = RServerSettings::self()->plotHeight();
    auto format = RServerSettings::self()->inlinePlotFormat();

    if (format == 0 || format == 1) // PDF and SVG
    {
        // convert the size from cm to inches
        w =  w / 2.54;
        h = h / 2.54;

        if (format == 0)
        {
            // TODO: pdf produces corrupted output!
            command = QLatin1String("pdf(\"%1\", width = %2, height = %3)");
            extension = QLatin1String("pdf");
        }
        else
        {
            command = QLatin1String("svg(\"%1\", width = %2, height = %3)");
            extension = QLatin1String("svg");
        }
    }
    else // PNG
    {
        // convert the size from cm to pixels with the current desktop resolution
        w = w / 2.54 * QApplication::desktop()->physicalDpiX();
        h = h / 2.54 * QApplication::desktop()->physicalDpiX();
        command = QLatin1String("png(\"%1\", width = %2, height = %3)");
        extension = QLatin1String("png");
    }

    m_curPlotFile = QString::fromLatin1("%1/Rplot%2.%3").arg(m_tmpDir, QString::number(deviceNum++), extension);
    if(m_isInitialized)
        runCommand(QLatin1String("dev.off()"), true);

    runCommand(command.arg(m_curPlotFile, QString::number(w), QString::number(h)), true);
}
