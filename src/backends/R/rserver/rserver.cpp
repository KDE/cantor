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

#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>

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

#undef error
#undef isObject

const QChar RServer::recordSep(30);
const QChar RServer::unitSep(31);

namespace
{
QString rString(SEXP string)
{
    if (string == NA_STRING)
        return QStringLiteral("NA");
    return QString::fromUtf8(translateCharUTF8(string));
}

QString rTypeName(SEXP value)
{
    const SEXP classAttribute = Rf_getAttrib(value, R_ClassSymbol);
    if (TYPEOF(classAttribute) == STRSXP && XLENGTH(classAttribute) > 0)
        return rString(STRING_ELT(classAttribute, 0));
    return QString::fromLatin1(type2char(TYPEOF(value)));
}

QString rDimensions(SEXP value)
{
    if (Rf_inherits(value, "data.frame"))
    {
        const qsizetype rows = XLENGTH(value) == 0 ? 0 : XLENGTH(VECTOR_ELT(value, 0));
        return QStringLiteral("%1x%2").arg(rows).arg(XLENGTH(value));
    }

    const SEXP dimensions = Rf_getAttrib(value, R_DimSymbol);
    if (TYPEOF(dimensions) == INTSXP && XLENGTH(dimensions) > 0)
    {
        QStringList values;
        for (R_xlen_t i = 0; i < XLENGTH(dimensions); ++i)
            values.append(QString::number(INTEGER(dimensions)[i]));
        return values.join(QLatin1Char('x'));
    }

    return QString::number(XLENGTH(value));
}

int rPreviewType(SEXP value)
{
    if (Rf_inherits(value, "data.frame") || Rf_isMatrix(value))
        return 1;
    if (TYPEOF(value) == VECSXP)
    {
        const SEXP names = Rf_getAttrib(value, R_NamesSymbol);
        return TYPEOF(names) == STRSXP && XLENGTH(names) == XLENGTH(value) ? 2 : 1;
    }
    if (Rf_isVectorAtomic(value) && XLENGTH(value) > 1)
        return 1;
    return 0;
}

QString rElementType(SEXP value, R_xlen_t index)
{
    if (TYPEOF(value) == VECSXP)
        return rTypeName(VECTOR_ELT(value, index));
    if (Rf_inherits(value, "factor"))
        return QStringLiteral("factor");
    return QString::fromLatin1(type2char(TYPEOF(value)));
}

QString rCompactValue(SEXP value, int depth = 0);

QString rElementValue(SEXP value, R_xlen_t index)
{
    switch (TYPEOF(value))
    {
        case STRSXP:
            return rString(STRING_ELT(value, index));
        case LGLSXP:
        {
            const int item = LOGICAL(value)[index];
            return item == NA_LOGICAL ? QStringLiteral("NA") : item ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        }
        case INTSXP:
        {
            const int item = INTEGER(value)[index];
            if (item == NA_INTEGER)
                return QStringLiteral("NA");
            if (Rf_inherits(value, "factor"))
            {
                const SEXP levels = Rf_getAttrib(value, R_LevelsSymbol);
                if (item > 0 && item <= XLENGTH(levels))
                    return rString(STRING_ELT(levels, item - 1));
            }
            return QString::number(item);
        }
        case REALSXP:
        {
            const double item = REAL(value)[index];
            return R_IsNA(item) ? QStringLiteral("NA") : QString::number(item, 'g', 15);
        }
        case CPLXSXP:
        {
            const Rcomplex item = COMPLEX(value)[index];
            return QStringLiteral("%1%2%3i").arg(item.r, 0, 'g', 15).arg(item.i < 0 ? QString() : QStringLiteral("+")).arg(item.i, 0, 'g', 15);
        }
        case RAWSXP:
            return QStringLiteral("0x%1").arg(RAW(value)[index], 2, 16, QLatin1Char('0'));
        case VECSXP:
        {
            SEXP item = VECTOR_ELT(value, index);
            if (XLENGTH(item) == 1)
                return rElementValue(item, 0);
            return rCompactValue(item, 1);
        }
        default:
            return QStringLiteral("<%1>").arg(rTypeName(value));
    }
}

QString rQuotedString(QString value)
{
    value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    value.replace(QLatin1Char('\''), QStringLiteral("\\'"));
    value.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    return QLatin1Char('\'') + value + QLatin1Char('\'');
}

QString rCompactValue(SEXP value, int depth)
{
    constexpr R_xlen_t maximumItems = 8;
    constexpr qsizetype maximumLength = 180;

    if (depth > 3)
        return QStringLiteral("<%1>").arg(rTypeName(value));

    const R_xlen_t length = XLENGTH(value);
    if (TYPEOF(value) == VECSXP)
    {
        const SEXP names = Rf_getAttrib(value, R_NamesSymbol);
        const bool hasNames = TYPEOF(names) == STRSXP && XLENGTH(names) == length;
        QStringList items;
        for (R_xlen_t i = 0; i < qMin(length, maximumItems); ++i)
        {
            QString item = rCompactValue(VECTOR_ELT(value, i), depth + 1);
            if (hasNames)
            {
                const QString name = rString(STRING_ELT(names, i));
                if (!name.isEmpty())
                    item.prepend(name + QStringLiteral(" = "));
            }
            items.append(item);
        }
        if (length > maximumItems)
            items.append(QStringLiteral("..."));
        QString result = QStringLiteral("list(%1)").arg(items.join(QStringLiteral(", ")));
        if (result.size() > maximumLength)
            result = result.left(maximumLength - 3) + QStringLiteral("...");
        return result;
    }

    if (length == 0)
        return QStringLiteral("%1(0)").arg(rTypeName(value));
    if (length == 1)
        return TYPEOF(value) == STRSXP ? rQuotedString(rElementValue(value, 0)) : rElementValue(value, 0);

    QStringList items;
    for (R_xlen_t i = 0; i < qMin(length, maximumItems); ++i)
    {
        const QString item = rElementValue(value, i);
        items.append(TYPEOF(value) == STRSXP ? rQuotedString(item) : item);
    }
    if (length > maximumItems)
        items.append(QStringLiteral("..."));
    QString result = QStringLiteral("c(%1)").arg(items.join(QStringLiteral(", ")));
    if (result.size() > maximumLength)
        result = result.left(maximumLength - 3) + QStringLiteral("...");
    return result;
}

QByteArray rReferenceData(const QString& variableName, const QJsonArray& path, const QString& displayName)
{
    QJsonObject object;
    object.insert(QLatin1String("name"), variableName);
    object.insert(QLatin1String("path"), path);
    object.insert(QLatin1String("displayName"), displayName);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

QJsonObject rCell(SEXP container, R_xlen_t index, const QString& variableName = QString(), const QJsonArray& path = {}, const QString& displayName = QString())
{
    QJsonObject cell;
    cell.insert(QLatin1String("value"), rElementValue(container, index));
    cell.insert(QLatin1String("valueType"), rElementType(container, index));

    if (TYPEOF(container) == VECSXP)
    {
        SEXP item = VECTOR_ELT(container, index);
        const int type = rPreviewType(item);
        if (type != 0)
        {
            QJsonArray itemPath = path;
            itemPath.append(static_cast<qint64>(index));
            QJsonObject reference;
            reference.insert(QLatin1String("displayName"), displayName);
            reference.insert(QLatin1String("backendData"), QString::fromUtf8(rReferenceData(variableName, itemPath, displayName)));
            reference.insert(QLatin1String("type"), type);
            cell.insert(QLatin1String("reference"), reference);
        }
    }
    return cell;
}
}


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
    const auto scripts = RServerSettings::self()->autorunScripts();
    for (const QString& path : scripts)
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
        else if (cmd.startsWith(QLatin1String("%variable preview ")))
        {
            const QStringList parts = cmd.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() == 5)
                previewVariable(QByteArray::fromBase64(parts.at(2).toLatin1()), parts.at(3).toLongLong(), parts.at(4).toLongLong());
            else
            {
                Q_EMIT expressionFinished(RServer::ErrorCode, QStringLiteral("Invalid variable preview request"), QStringList());
                setStatus(RServer::Idle);
            }
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

    QStringList vars, values, types, dimensions, funcs, constants;
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
                types << (rPreviewType(value) == 2 ? QStringLiteral("named list") : rTypeName(value));
                dimensions << rDimensions(value);
            }
            UNPROTECT(1);
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

    const QString output = vars.join(recordSep) + unitSep + values.join(recordSep) + unitSep + funcs.join(recordSep) + unitSep
        + constants.join(recordSep) + unitSep + types.join(recordSep) + unitSep + dimensions.join(recordSep);
    Q_EMIT expressionFinished(RServer::SuccessCode, output, QStringList());
    setStatus(Idle);
}

void RServer::previewVariable(const QByteArray& referenceData, qsizetype offset, qsizetype limit)
{
    setStatus(RServer::Busy);

    QJsonParseError parseError;
    const QJsonDocument referenceDocument = QJsonDocument::fromJson(referenceData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !referenceDocument.isObject())
    {
        Q_EMIT expressionFinished(RServer::ErrorCode, QStringLiteral("Invalid variable reference"), QStringList());
        setStatus(RServer::Idle);
        return;
    }

    const QJsonObject reference = referenceDocument.object();
    const QString variableName = reference.value(QLatin1String("name")).toString();
    const QJsonArray path = reference.value(QLatin1String("path")).toArray();
    SEXP value = findVar(install(variableName.toUtf8().constData()), R_GlobalEnv);
    if (value == R_UnboundValue)
    {
        Q_EMIT expressionFinished(RServer::ErrorCode, QStringLiteral("The variable no longer exists"), QStringList());
        setStatus(RServer::Idle);
        return;
    }

    for (const auto& pathItem : path)
    {
        const qsizetype index = pathItem.toInteger();
        if (TYPEOF(value) != VECSXP || index < 0 || index >= XLENGTH(value))
        {
            Q_EMIT expressionFinished(RServer::ErrorCode, QStringLiteral("The nested value no longer exists"), QStringList());
            setStatus(RServer::Idle);
            return;
        }
        value = VECTOR_ELT(value, index);
    }

    offset = qMax<qsizetype>(0, offset);
    limit = qMax<qsizetype>(1, limit);
    const int previewType = rPreviewType(value);
    QJsonObject result;
    result.insert(QLatin1String("type"), previewType);
    result.insert(QLatin1String("typeName"), rTypeName(value));
    result.insert(QLatin1String("dimensions"), rDimensions(value));
    result.insert(QLatin1String("offset"), offset);

    QJsonArray columns;
    QJsonArray rows;
    qsizetype totalRows = 0;

    if (previewType == 2)
    {
        columns = {QStringLiteral("@key"), QStringLiteral("@type"), QStringLiteral("@value")};
        const SEXP names = Rf_getAttrib(value, R_NamesSymbol);
        totalRows = XLENGTH(value);
        for (qsizetype i = offset; i < qMin(offset + limit, totalRows); ++i)
        {
            const QString key = rString(STRING_ELT(names, i));
            const QString childName = QStringLiteral("%1[[%2]]").arg(reference.value(QLatin1String("displayName")).toString(), key);
            QJsonArray row;
            QJsonObject keyCell;
            keyCell.insert(QLatin1String("value"), key);
            row.append(keyCell);
            QJsonObject typeCell;
            typeCell.insert(QLatin1String("value"), rElementType(value, i));
            row.append(typeCell);
            row.append(rCell(value, i, variableName, path, childName));
            rows.append(row);
        }
    }
    else if (previewType == 1 && Rf_inherits(value, "data.frame"))
    {
        const SEXP names = Rf_getAttrib(value, R_NamesSymbol);
        columns.append(QStringLiteral("@index"));
        for (R_xlen_t column = 0; column < XLENGTH(value); ++column)
            columns.append(rString(STRING_ELT(names, column)));
        totalRows = XLENGTH(value) == 0 ? 0 : XLENGTH(VECTOR_ELT(value, 0));
        for (qsizetype rowIndex = offset; rowIndex < qMin(offset + limit, totalRows); ++rowIndex)
        {
            QJsonArray row;
            QJsonObject indexCell;
            indexCell.insert(QLatin1String("value"), QString::number(rowIndex + 1));
            row.append(indexCell);
            for (R_xlen_t column = 0; column < XLENGTH(value); ++column)
                row.append(rCell(VECTOR_ELT(value, column), rowIndex));
            rows.append(row);
        }
    }
    else if (previewType == 1 && Rf_isMatrix(value))
    {
        const SEXP dimensions = Rf_getAttrib(value, R_DimSymbol);
        const qsizetype rowCount = INTEGER(dimensions)[0];
        const qsizetype columnCount = INTEGER(dimensions)[1];
        columns.append(QStringLiteral("@index"));
        for (qsizetype column = 0; column < columnCount; ++column)
            columns.append(QString::number(column + 1));
        totalRows = rowCount;
        for (qsizetype rowIndex = offset; rowIndex < qMin(offset + limit, totalRows); ++rowIndex)
        {
            QJsonArray row;
            QJsonObject indexCell;
            indexCell.insert(QLatin1String("value"), QString::number(rowIndex + 1));
            row.append(indexCell);
            for (qsizetype column = 0; column < columnCount; ++column)
                row.append(rCell(value, rowIndex + rowCount * column));
            rows.append(row);
        }
    }
    else if (previewType == 1)
    {
        columns = {QStringLiteral("@index"), QStringLiteral("@type"), QStringLiteral("@value")};
        totalRows = XLENGTH(value);
        for (qsizetype i = offset; i < qMin(offset + limit, totalRows); ++i)
        {
            QJsonArray row;
            QJsonObject indexCell;
            indexCell.insert(QLatin1String("value"), QString::number(i + 1));
            row.append(indexCell);
            QJsonObject typeCell;
            typeCell.insert(QLatin1String("value"), rElementType(value, i));
            row.append(typeCell);
            const QString childName = QStringLiteral("%1[[%2]]").arg(reference.value(QLatin1String("displayName")).toString()).arg(i + 1);
            row.append(rCell(value, i, variableName, path, childName));
            rows.append(row);
        }
    }
    else
        result.insert(QLatin1String("errorCode"), QStringLiteral("unsupportedType"));

    result.insert(QLatin1String("columns"), columns);
    result.insert(QLatin1String("rows"), rows);
    result.insert(QLatin1String("totalRows"), totalRows);
    result.insert(QLatin1String("hasMore"), offset + limit < totalRows);

    const QByteArray output = QJsonDocument(result).toJson(QJsonDocument::Compact).toBase64();
    Q_EMIT expressionFinished(RServer::SuccessCode, QStringLiteral("__CANTOR_VARIABLE_PREVIEW__") + QString::fromLatin1(output), QStringList());
    setStatus(RServer::Idle);
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
        const int dpi = QGuiApplication::primaryScreen()->physicalDotsPerInchX();
        w = w / 2.54 * dpi;
        h = h / 2.54 * dpi;
        command = QLatin1String("png(\"%1\", width = %2, height = %3)");
        extension = QLatin1String("png");
    }

    m_curPlotFile = QString::fromLatin1("%1/Rplot%2.%3").arg(m_tmpDir, QString::number(deviceNum++), extension);
    if(m_isInitialized)
        runCommand(QLatin1String("dev.off()"), true);

    runCommand(command.arg(m_curPlotFile, QString::number(w), QString::number(h)), true);
}
