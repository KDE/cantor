/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2012 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2017-2021 by Alexander Semke (alexander.semke@web.de)
*/

#include "maximaexpression.h"

#include <config-cantorlib.h>

#include "maximasession.h"
#include "textresult.h"
#include "epsresult.h"
#include "imageresult.h"
#include "helpresult.h"
#include "latexresult.h"
#include "settings.h"

#include <QDir>
#include <QTemporaryFile>

#include <KLocalizedString>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>
#include <QChar>
#include <QUrl>

// MaximaExpression use real id from Maxima as expression id, so we don't know id before executing
MaximaExpression::MaximaExpression( Cantor::Session* session, bool internal ) : Cantor::Expression(session, internal, -1)
{
}

MaximaExpression::~MaximaExpression() {
    if(m_tempFile)
        delete m_tempFile;
}

void MaximaExpression::evaluate()
{
    m_isHelpRequest = false;
    m_gotErrorContent = false;

    if(m_tempFile)
    {
        delete m_tempFile;
        m_tempFile = nullptr;
        m_isPlot = false;
        m_plotResult = nullptr;
        m_plotResultIndex = -1;
    }

    QString cmd = command();

    //if the user explicitly has entered quit(), do a logout here
    //otherwise maxima's process will be stopped after the evaluation of this command
    //and we re-start it because of "maxima has crashed".
    if (cmd.remove(QLatin1Char(' ')) == QLatin1String("quit()"))
    {
        session()->logout();
        return;
    }

    //check if this is a ?command
    if(cmd.startsWith(QLatin1String("??"))
        || cmd.startsWith(QLatin1String("describe("))
        || cmd.startsWith(QLatin1String("example("))
        || cmd.startsWith(QLatin1String(":lisp(cl-info::info-exact")))
        m_isHelpRequest=true;

    if (MaximaSettings::self()->integratePlots()
        && !cmd.contains(QLatin1String("ps_file"))
        && cmd.contains(QRegularExpression(QStringLiteral("(?:plot2d|plot3d|contour_plot)\\s*\\([^\\)]"))))
    {
        m_isPlot=true;
#ifdef WITH_EPS
        m_tempFile=new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_maxima-XXXXXX.eps" ));
#else
        m_tempFile=new QTemporaryFile(QDir::tempPath() + QLatin1String("/cantor_maxima-XXXXXX.png"));
#endif
        m_tempFile->open();

        m_fileWatch.removePaths(m_fileWatch.files());
        m_fileWatch.addPath(m_tempFile->fileName());
        connect(&m_fileWatch, &QFileSystemWatcher::fileChanged, this, &MaximaExpression::imageChanged,  Qt::UniqueConnection);
    }

    bool isComment = true;
    int commentLevel = 0;
    bool inString = false;
    for (int i = 0; i < cmd.size(); ++i) {
        if (cmd[i] == QLatin1Char('\\')) {
            ++i; // skip the next character
            if (commentLevel == 0 && !inString) {
                isComment = false;
            }
        } else if (cmd[i] == QLatin1Char('"') && commentLevel == 0) {
            inString = !inString;
            isComment = false;
        } else if (cmd.mid(i,2) == QLatin1String("/*") && !inString) {
            ++commentLevel;
            ++i;
        } else if (cmd.mid(i,2) == QLatin1String("*/") && !inString) {
            if (commentLevel == 0) {
                qDebug() << "Comments mismatched!";
                setErrorMessage(i18n("Error: Too many */"));
                setStatus(Cantor::Expression::Error);
                return;
            }
            ++i;
            --commentLevel;
        } else if (isComment && commentLevel == 0 && !cmd[i].isSpace()) {
            isComment = false;
        }
    }

    if (commentLevel > 0) {
        qDebug() << "Comments mismatched!";
        setErrorMessage(i18n("Error: Too many /*"));
        setStatus(Cantor::Expression::Error);
        return;
    }
    if (inString) {
        qDebug() << "String not closed";
        setErrorMessage(i18n("Error: expected \" before ;"));
        setStatus(Cantor::Expression::Error);
        return;
    }
    if(isComment)
    {
        setStatus(Cantor::Expression::Done);
        return;
    }

    session()->enqueueExpression(this);
}

void MaximaExpression::interrupt()
{
    qDebug()<<"interrupting";
    setStatus(Cantor::Expression::Interrupted);
}

QString MaximaExpression::internalCommand()
{
    QString cmd=command();

    if(m_isPlot)
    {
        if(!m_tempFile)
        {
            qDebug()<<"plotting without tempFile";
            return QString();
        }
        QString fileName = m_tempFile->fileName();

#ifdef WITH_EPS
        const QString psParam=QLatin1String("[gnuplot_ps_term_command, \"set size 1.0,  1.0; set term postscript eps color solid \"]");
        const QString plotParameters = QLatin1String("[ps_file, \"")+ fileName+QLatin1String("\"],")+psParam;
#else
        const QString plotParameters = QLatin1String("[gnuplot_term, \"png size 500,340\"], [gnuplot_out_file, \"")+fileName+QLatin1String("\"]");

#endif
        cmd.replace(QRegularExpression(QStringLiteral("((plot2d|plot3d|contour_plot)\\s*\\(.*)\\)([;\n$]|$)")),
                    QLatin1String("\\1, ") + plotParameters + QLatin1String(");"));

    }

    if (!cmd.endsWith(QLatin1Char('$')))
    {
        if (!cmd.endsWith(QLatin1String(";")))
            cmd+=QLatin1Char(';');
    }

    //replace all newlines with spaces, as maxima isn't sensitive about
    //whitespaces, and without newlines the whole command
    //is executed at once, without outputting an input
    //prompt after each line
    cmd.replace(QLatin1Char('\n'), QLatin1Char(' '));

    //lisp-quiet doesn't print a prompt after the command
    //is completed, which causes the parsing to hang.
    //replace the command with the non-quiet version
    cmd.replace(QRegularExpression(QStringLiteral("^:lisp-quiet")), QStringLiteral(":lisp"));

    return cmd;
}

void MaximaExpression::forceDone()
{
    qDebug()<<"forcing Expression state to DONE";
    setResult(nullptr);
    setStatus(Cantor::Expression::Done);
}

/*!
    example output for the simple expression '5+5':
    latex mode - "<cantor-result><cantor-text>\n(%o1) 10\n</cantor-text><cantor-latex>\\mbox{\\tt\\red(\\mathrm{\\%o1}) \\black}10</cantor-latex></cantor-result>\n<cantor-prompt>(%i2) </cantor-prompt>\n"
    text mode  - "<cantor-result><cantor-text>\n(%o1) 10\n</cantor-text></cantor-result>\n<cantor-prompt>(%i2) </cantor-prompt>\n"
 */
bool MaximaExpression::parseOutput(QString& out)
{
    const int promptStart = out.indexOf(QLatin1String("<cantor-prompt>"));
    const int promptEnd = out.indexOf(QLatin1String("</cantor-prompt>"));
    const QString prompt = out.mid(promptStart + 15, promptEnd - promptStart - 15).simplified();

    //check whether the result is part of the promt - this is the case when additional input is required from the user
    if (prompt.contains(QLatin1String("<cantor-result>")))
    {
        //text part of the output
        const int textContentStart = prompt.indexOf(QLatin1String("<cantor-text>"));
        const int textContentEnd = prompt.indexOf(QLatin1String("</cantor-text>"));
        QString textContent = prompt.mid(textContentStart + 13, textContentEnd - textContentStart - 13).trimmed();

        qDebug()<<"asking for additional input for " << textContent;
        emit needsAdditionalInformation(textContent);
        return true;
    }

    qDebug()<<"new input label: " << prompt;

    QString errorContent;

    //parse the results
    int resultStart = out.indexOf(QLatin1String("<cantor-result>"));
    if (resultStart != -1)
        errorContent += out.mid(0, resultStart);

    while (resultStart != -1)
    {
        int resultEnd = out.indexOf(QLatin1String("</cantor-result>"), resultStart + 15);
        const QString resultContent = out.mid(resultStart + 15, resultEnd - resultStart - 15);
        parseResult(resultContent);

        //search for the next openning <cantor-result> tag after the current closing </cantor-result> tag
        resultStart = out.indexOf(QLatin1String("<cantor-result>"), resultEnd + 16);
    }

    //parse the error message, the part outside of the <cantor*> tags
    int lastResultEnd = out.lastIndexOf(QLatin1String("</cantor-result>"));
    if (lastResultEnd != -1)
        lastResultEnd += 16;
    else
        lastResultEnd = 0;

    errorContent += out.mid(lastResultEnd, promptStart - lastResultEnd).trimmed();
    if (errorContent.isEmpty())
    {
        // For plots we set Done status in imageChanged
        if (!m_isPlot || m_plotResult)
            setStatus(Cantor::Expression::Done);
    }
    else
    {
        qDebug() << "error content: " << errorContent;

        if (out.contains(QLatin1String("cantor-value-separator")))
        {
            //when fetching variables, in addition to the actual result with variable names and values,
            //Maxima also writes out the names of the variables to the error buffer.
            //we don't interpret this as an error.
            setStatus(Cantor::Expression::Done);
        }
        else if(m_isHelpRequest || m_isHelpRequestAdditional) //help messages are also part of the error output
        {
            //we've got help result, but maybe additional input is required -> check this
            const int index = prompt.trimmed().indexOf(MaximaSession::MaximaInputPrompt);
            if (index == -1) {
                // No input label found in the prompt -> additional info is required
                qDebug()<<"asking for additional input for the help request" << prompt;
                m_isHelpRequestAdditional = true;
                emit needsAdditionalInformation(prompt);
            }

            //set the help result
            errorContent.prepend(QLatin1Char(' '));
            Cantor::HelpResult* result = new Cantor::HelpResult(errorContent);
            setResult(result);

            //if a new input prompt was found, no further input is expected and we're done
            if (index != -1) {
                m_isHelpRequestAdditional = false;
                setStatus(Cantor::Expression::Done);
            }
        }
        else
        {
            errorContent = errorContent.replace(QLatin1String("\n\n"), QLatin1String("\n"));
            setErrorMessage(errorContent);
            setStatus(Cantor::Expression::Error);
        }
    }

    return true;
}

void MaximaExpression::parseResult(const QString& resultContent)
{
    //in case we asked for additional input for the help request,
    //no need to process the result - we're not done yet and maxima is waiting for further input
    if (m_isHelpRequestAdditional)
        return;

    qDebug()<<"result content: " << resultContent;

    //text part of the output
    const int textContentStart = resultContent.indexOf(QLatin1String("<cantor-text>"));
    const int textContentEnd = resultContent.indexOf(QLatin1String("</cantor-text>"));
    QString textContent = resultContent.mid(textContentStart + 13, textContentEnd - textContentStart - 13).trimmed();
    qDebug()<<"text content: " << textContent;

    //output label can be a part of the text content -> determine it
    const QRegularExpression regex = QRegularExpression(MaximaSession::MaximaOutputPrompt.pattern());
    QRegularExpressionMatch match = regex.match(textContent);
    QString outputLabel;
    if (match.hasMatch()) // a match is found, so the output contains output label
        outputLabel = textContent.mid(match.capturedStart(0), match.capturedLength(0)).trimmed();
    qDebug()<<"output label: " << outputLabel;

    //extract the expression id
    bool ok;
    QString idString = outputLabel.mid(3, outputLabel.length()-4);
    int id = idString.toInt(&ok);
    if (ok)
        setId(id);

    qDebug()<<"expression id: " << this->id();

    //remove the output label from the text content
    textContent = textContent.remove(outputLabel).trimmed();

    //determine the actual result
    Cantor::Result* result = nullptr;

    const int latexContentStart = resultContent.indexOf(QLatin1String("<cantor-latex>"));
    //Handle system maxima output for plotting commands
    if (m_isPlot && textContent.endsWith(QString::fromLatin1("\"%1\"]").arg(m_tempFile->fileName())))
    {
        m_plotResultIndex = results().size();
        // Gnuplot could generate plot before we parse text output from maxima and after
        // If we already have plot result, just add it
        // Else set info message, and replace it by real result in imageChanged function later
        if (m_plotResult)
            result = m_plotResult;
        else
            result = new Cantor::TextResult(i18n("Waiting for the plot result"));
    }
    else if (latexContentStart != -1)
    {
        //latex output is available
        const int latexContentEnd = resultContent.indexOf(QLatin1String("</cantor-latex>"));
        QString latexContent = resultContent.mid(latexContentStart + 14, latexContentEnd - latexContentStart - 14).trimmed();
        qDebug()<<"latex content: " << latexContent;

        Cantor::TextResult* textResult;
        //replace the \mbox{} environment, if available, by the eqnarray environment
        if (latexContent.indexOf(QLatin1String("\\mbox{")) != -1)
        {
            int i;
            int pcount=0;
            for(i = latexContent.indexOf(QLatin1String("\\mbox{"))+5; i < latexContent.size(); ++i)
            {
                if(latexContent[i]==QLatin1Char('{'))
                    pcount++;
                else if(latexContent[i]==QLatin1Char('}'))
                    pcount--;

                if(pcount==0)
                    break;
            }

            QString modifiedLatexContent = latexContent.mid(i+1);
            if(modifiedLatexContent.trimmed().isEmpty())
            {
                //empty content in the \mbox{} environment (e.g. for print() outputs), use the latex string outside of the \mbox{} environment
                modifiedLatexContent = latexContent.left(latexContent.indexOf(QLatin1String("\\mbox{")));
            }

            modifiedLatexContent.prepend(QLatin1String("\\begin{eqnarray*}"));
            modifiedLatexContent.append(QLatin1String("\\end{eqnarray*}"));
            textResult = new Cantor::TextResult(modifiedLatexContent, textContent);
            qDebug()<<"modified latex content: " << modifiedLatexContent;
        }
        else
        {
            //no \mbox{} available, use what we've got.
            textResult = new Cantor::TextResult(latexContent, textContent);
        }

        textResult->setFormat(Cantor::TextResult::LatexFormat);
        result = textResult;
    }
    else
    {
        //no latex output is available, the actual result is part of the textContent string
        result = new Cantor::TextResult(textContent);
    }

    addResult(result);
}

void MaximaExpression::parseError(const QString& out)
{
    m_errorBuffer.append(out);
}

void MaximaExpression::addInformation(const QString& information)
{
    qDebug()<<"adding information";
    QString inf=information;
    if(!inf.endsWith(QLatin1Char(';')))
        inf+=QLatin1Char(';');
    Cantor::Expression::addInformation(inf);

    static_cast<MaximaSession*>(session())->sendInputToProcess(inf+QLatin1Char('\n'));
}

void MaximaExpression::imageChanged()
{
    if(m_tempFile->size()>0)
    {
#ifdef WITH_EPS
        m_plotResult = new Cantor::EpsResult( QUrl::fromLocalFile(m_tempFile->fileName()) );
#else
        m_plotResult = new Cantor::ImageResult( QUrl::fromLocalFile(m_tempFile->fileName()) );
#endif
        // Check, that we already parse maxima output for this plot, and if not, keep it up to this moment
        // If it's true, replace text info result by real plot and set status as Done
        if (m_plotResultIndex != -1)
        {
            replaceResult(m_plotResultIndex, m_plotResult);
            if (status() != Cantor::Expression::Error)
                setStatus(Cantor::Expression::Done);
        }
    }
}
