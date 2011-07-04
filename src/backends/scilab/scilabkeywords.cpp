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
    Copyright (C) 2011 Filipe Saraiva <filip.saraiva@gmail.com>
 */

#include "scilabkeywords.h"

#include <QFile>
#include <QXmlStreamReader>

#include <kdebug.h>
#include <kstandarddirs.h>

ScilabKeywords::ScilabKeywords()
{
    kDebug() << "ScilabKeywords construtor";

}


ScilabKeywords::~ScilabKeywords()
{

}

ScilabKeywords* ScilabKeywords::instance()
{
    static ScilabKeywords* inst=0;
    if(inst==0)
    {
        inst=new ScilabKeywords();
        inst->loadFromFile();
    }

    return inst;
}

void ScilabKeywords::loadFromFile()
{
    kDebug() << "ScilabKeywords lodFromFile()";

    //load the known keywords from an xml file
    QFile file(KStandardDirs::locate("appdata",  "scilabbackend/keywords.xml"));

    if(!file.open(QIODevice::ReadOnly))
    {
        kDebug() << "error opening keywords.xml file. highlighting and completion won't work";
        return;
    }

    QXmlStreamReader xml(&file);

    xml.readNextStartElement();
    while(xml.readNextStartElement())
    {
        const QStringRef name=xml.name();

        if((name == "keywords")|| (name == "variables") || (name == "functions"))
        {
            while(xml.readNextStartElement())
            {
                Q_ASSERT(xml.isStartElement() && xml.name() == "word");

                const QString text = xml.readElementText();

                if(name == "keywords")
                    m_keywords << text;

                else if(name == "variables")
                    m_variables << text;

                else if(name == "functions")
                    m_functions<<text;
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError())
    {
        kDebug() << "error parsing element";
        kDebug() << "error: "<< xml.errorString();
    }

}

const QStringList& ScilabKeywords::variables() const
{
    return m_variables;
}

const QStringList& ScilabKeywords::functions() const
{
    return m_functions;
}

const QStringList& ScilabKeywords::keywords() const
{
    return m_keywords;
}
