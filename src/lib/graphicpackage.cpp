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
    Copyright (C) 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#include "graphicpackage.h"

#include <QDir>
#include <QDomDocument>
#include <QDebug>

#include "session.h"
#include "expression.h"

using namespace Cantor;

class Cantor::GraphicPackagePrivate
{
  public:
    QString id;
    QString name;
    QString testPresenceCommand;
    QString enableSupportCommand;
    QString disableSupportCommand;
    QString saveToFileCommandTemplate;
    QStringList plotPrecenseKeywords;
};

Cantor::GraphicPackage::GraphicPackage(const Cantor::GraphicPackage& other): d(new GraphicPackagePrivate)
{
    *d = *other.d;
}

Cantor::GraphicPackage::GraphicPackage(): d(new GraphicPackagePrivate)
{
}

Cantor::GraphicPackage::~GraphicPackage()
{
    delete d;
}

QString Cantor::GraphicPackage::id() const
{
    return d->id;
}

QString Cantor::GraphicPackage::name() const
{
    return d->name;
}

Expression* Cantor::GraphicPackage::isAvailable(Session* session) const
{
    return session->evaluateExpression(d->testPresenceCommand, Expression::FinishingBehavior::DoNotDelete, true);
}

QString Cantor::GraphicPackage::enableSupportCommand(QString additionalInfo) const
{
    return d->enableSupportCommand.arg(additionalInfo);
}

QString Cantor::GraphicPackage::disableSupportCommand() const
{
    return d->disableSupportCommand;
}

QString Cantor::GraphicPackage::savePlotCommand(QString filenamePrefix, int plotNumber, QString additionalInfo) const
{
    return d->saveToFileCommandTemplate.arg(filenamePrefix, QString::number(plotNumber), additionalInfo);
}

bool Cantor::GraphicPackage::isHavePlotCommand() const
{
    return !d->saveToFileCommandTemplate.isEmpty();
}

const QStringList & Cantor::GraphicPackage::plotCommandPrecentsKeywords() const
{
    return d->plotPrecenseKeywords;
}

QList<GraphicPackage> Cantor::GraphicPackage::loadFromFile(const QString& filename)
{
    QList<GraphicPackage> packages;

    if (!QFile::exists(filename))
        return packages;

    QFile fin(filename);
    if (fin.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        if (doc.setContent(fin.readAll()) && doc.firstChildElement(QLatin1String("GraphicPackages")).isNull() == false)
        {
            const auto& elements = doc.elementsByTagName(QLatin1String("GraphicPackage"));
            for (int i = 0; i < elements.size(); i++)
            {
                const QDomNode& root = elements.item(i);

                GraphicPackage package;
                package.d->id = root.firstChildElement(QLatin1String("Id")).text().trimmed();
                package.d->name = root.firstChildElement(QLatin1String("Name")).text().trimmed();
                package.d->testPresenceCommand = root.firstChildElement(QLatin1String("TestPresenceCommand")).text().trimmed();
                package.d->enableSupportCommand = root.firstChildElement(QLatin1String("EnableCommand")).text().trimmed();
                package.d->disableSupportCommand = root.firstChildElement(QLatin1String("DisableCommand")).text().trimmed();
                package.d->saveToFileCommandTemplate = root.firstChildElement(QLatin1String("ToFileCommandTemplate")).text().trimmed();

                QString delimiter = QLatin1String("\n");
                const QDomElement& delimiterElement = root.firstChildElement(QLatin1String("PlotPrecenseKeywordsDelimiter"));
                if (!delimiterElement.isNull())
                    delimiter = delimiterElement.text().trimmed();
                package.d->plotPrecenseKeywords = root.firstChildElement(QLatin1String("PlotPrecenseKeywords")).text().trimmed().split(delimiter, QString::SkipEmptyParts);
                for (QString& name : package.d->plotPrecenseKeywords)
                    name = name.trimmed();

                packages.append(package);
            }
        }
        else
            qWarning() << "fail parse" << filename << "as xml file";
    }

    return packages;
}

int Cantor::GraphicPackage::findById(const GraphicPackage& package, const QList<GraphicPackage>& list)
{
    for (int i = 0; i < list.size(); i++)
        if (list[i].id() == package.id())
            return i;
    return -1;
}


