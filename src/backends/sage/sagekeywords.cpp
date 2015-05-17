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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */
#include "sagekeywords.h"

#include <QStandardPaths>
#include <QFile>
#include <QXmlStreamReader>
#include <QtAlgorithms>

#include <QDebug>

SageKeywords::SageKeywords()
{

}

SageKeywords::~SageKeywords()
{

}

SageKeywords* SageKeywords::instance()
{
    static SageKeywords* inst=0;
    if(inst==0)
    {
        inst=new SageKeywords();
        inst->loadFromFile();
	qSort(inst->m_keywords);
    }

    return inst;
}

void SageKeywords::loadFromFile()
{
    //load the known keywords from an xml file
    QFile file(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("sagebackend/keywords.xml")));

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"error opening keywords.xml file. highlighting and completion won't work";
        return;
    }

    QXmlStreamReader xml(&file);

    xml.readNextStartElement();
    while(xml.readNextStartElement())
    {
        const QStringRef name=xml.name();

        if(name==QLatin1String("keywords"))
        {
            while(xml.readNextStartElement())
            {
                Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("word"));

                const QString text=xml.readElementText();

		m_keywords<<text;
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError())
    {
        qDebug()<<"error parsing element";
        qDebug()<<"error: "<<xml.errorString();
    }

}

const QStringList& SageKeywords::keywords() const
{
    return m_keywords;
}
