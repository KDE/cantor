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

#include "latexresult.h"

#include <QFile>
#include <QTextStream>
#include <QJsonValue>
#include <QJsonObject>
#include <QDebug>

#include "jupyterutils.h"

using namespace Cantor;

class Cantor::LatexResultPrivate
{
  public:
    LatexResultPrivate()
    {
        showCode=false;
    }

    bool showCode;
    QString code;
    QString plain;
};

LatexResult::LatexResult(const QString& code, const QUrl &url, const QString& plain, const QImage& image) : EpsResult( url, image ),
                                                                                       d(new LatexResultPrivate)
{
    d->code=code;
    d->plain=plain;
}

LatexResult::~LatexResult()
{
    delete d;
}

int LatexResult::type()
{
    return LatexResult::Type;
}

QString LatexResult::mimeType()
{
    if(isCodeShown())
        return QStringLiteral("text/plain");
    else
        return EpsResult::mimeType();
}

QString LatexResult::code()
{
    return d->code;
}

QString LatexResult::plain()
{
    return d->plain;
}

bool LatexResult::isCodeShown()
{
    return d->showCode;
}

void LatexResult::showCode()
{
    d->showCode=true;
}

void LatexResult::showRendered()
{
    d->showCode=false;
}

QVariant LatexResult::data()
{
    if(isCodeShown())
        return QVariant(code());
    else
        return EpsResult::data();
}

QString LatexResult::toHtml()
{
    if (isCodeShown())
    {
            QString s=code();
            return s.toHtmlEscaped();
    }
    else
    {
        return EpsResult::toHtml();
    }
}

QString LatexResult::toLatex()
{
    return code();
}

QDomElement LatexResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving textresult "<<toHtml();
    QDomElement e = EpsResult::toXml(doc);
    e.setAttribute(QStringLiteral("type"), QStringLiteral("latex"));
    QDomText txt=doc.createTextNode(code());
    e.appendChild(txt);

    return e;
}

QJsonValue Cantor::LatexResult::toJupyterJson()
{
    QJsonObject root;

    if (executionIndex() != -1)
    {
        root.insert(QLatin1String("output_type"), QLatin1String("execute_result"));
        root.insert(QLatin1String("execution_count"), executionIndex());
    }
    else
        root.insert(QLatin1String("output_type"), QLatin1String("display_data"));

    QJsonObject data;
    data.insert(QLatin1String("text/plain"), JupyterUtils::toJupyterMultiline(d->plain));
    data.insert(QLatin1String("text/latex"), JupyterUtils::toJupyterMultiline(d->code));
    if (!image().isNull())
        data.insert(JupyterUtils::pngMime, JupyterUtils::packMimeBundle(image(), JupyterUtils::pngMime));
    root.insert(QLatin1String("data"), data);

    root.insert(QLatin1String("metadata"), jupyterMetadata());

    return root;
}

void LatexResult::save(const QString& filename)
{
    if(isCodeShown())
    {
        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream stream(&file);

        stream<<code();

        file.close();
    }else
    {
        EpsResult::save(filename);
    }
}

