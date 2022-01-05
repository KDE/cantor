/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#include "textresult.h"
using namespace Cantor;

#include <QDebug>

#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>

QString rtrim(const QString& s)
{
    QString result = s;
    while (result.count() > 0 && result[result.count()-1].isSpace() )
    {
        result = result.left(result.count() -1 );
    }
    return result;
}

class Cantor::TextResultPrivate
{
public:
    QString data;
    QString plain;
    TextResult::Format format{TextResult::PlainTextFormat};
    bool isStderr{false};
    bool isWarning{false};
};

TextResult::TextResult(const QString& data) : d(new TextResultPrivate)
{
    d->data=rtrim(data);
    d->plain=d->data;
}

TextResult::TextResult(const QString& data, const QString& plain) : d(new TextResultPrivate)
{
    d->data=rtrim(data);
    d->plain=rtrim(plain);
}

TextResult::~TextResult()
{
    delete d;
}

void TextResult::setIsWarning(bool value)
{
    d->isWarning = value;
}

bool TextResult::isWarning() const
{
    return d->isWarning;
}

QString TextResult::toHtml()
{
    QString s=d->data.toHtmlEscaped();
    s.replace(QLatin1Char('\n'), QLatin1String("<br/>\n"));
    s.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    return s;
}

QVariant TextResult::data()
{
    return QVariant(d->data);
}

QString TextResult::plain()
{
    return d->plain;
}

int TextResult::type()
{
    return TextResult::Type;
}

QString TextResult::mimeType()
{
    qDebug()<<"format: "<<format();
    switch(format())
    {
        case TextResult::PlainTextFormat:
            return QStringLiteral("text/plain");

        case TextResult::LatexFormat:
            return QStringLiteral("text/x-tex");

        default:
            return QString();
    }
}

TextResult::Format TextResult::format()
{
    return d->format;
}

void TextResult::setFormat(TextResult::Format f)
{
    d->format=f;
}

QDomElement TextResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving textresult "<<toHtml();
    QDomElement e=doc.createElement(QStringLiteral("Result"));
    e.setAttribute(QStringLiteral("type"), QStringLiteral("text"));
    e.setAttribute(QStringLiteral("stderr"), d->isStderr);
    if (d->format == LatexFormat)
        e.setAttribute(QStringLiteral("format"), QStringLiteral("latex"));
    QDomText txt=doc.createTextNode(data().toString());
    e.appendChild(txt);

    return e;
}

QJsonValue Cantor::TextResult::toJupyterJson()
{
    QJsonObject root;

    switch (d->format)
    {
        case PlainTextFormat:
        {
            if (executionIndex() != -1)
            {
                root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
                root.insert(QLatin1String("execution_count"), executionIndex());

                QJsonObject data;
                data.insert(QLatin1String("text/plain"), jupyterText(d->data));
                root.insert(QLatin1String("data"), data);

                root.insert(QLatin1String("metadata"), jupyterMetadata());
            }
            else
            {
                root.insert(QLatin1String("output_type"), QLatin1String("stream"));
                if (d->isStderr)
                    root.insert(QLatin1String("name"), QLatin1String("stderr"));
                else
                    root.insert(QLatin1String("name"), QLatin1String("stdout"));

                // Jupyter don't support a few text result (it merges them into one text),
                // so add additional \n to end
                // See https://github.com/jupyter/notebook/issues/4699
                root.insert(QLatin1String("text"), jupyterText(d->data, true));
            }
            break;
        }

        case LatexFormat:
        {
            if (executionIndex() != -1)
            {
                root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
                root.insert(QLatin1String("execution_count"), executionIndex());
            }
            else
                root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

            QJsonObject data;
            data.insert(QLatin1String("text/latex"), jupyterText(d->data));
            data.insert(QLatin1String("text/plain"), jupyterText(d->plain));
            root.insert(QLatin1String("data"), data);

            root.insert(QLatin1String("metadata"), jupyterMetadata());
            break;
        }
    }

    return root;
}

void TextResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    stream<<d->data;

    file.close();
}

QJsonArray TextResult::jupyterText(const QString& text, bool addEndNewLine)
{
    QJsonArray array;

    const QStringList& lines = text.split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); i++)
    {
        QString line = lines[i];
        if (i != lines.size() - 1 || addEndNewLine)
            line.append(QLatin1Char('\n'));
        array.append(line);
    }

    return array;
}

bool Cantor::TextResult::isStderr() const
{
    return d->isStderr;
}

void Cantor::TextResult::setStdErr(bool value)
{
    d->isStderr = value;
}
