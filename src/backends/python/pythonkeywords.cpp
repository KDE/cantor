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
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonkeywords.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QtAlgorithms>

#include <QDebug>
#include <KStandardDirs>

PythonKeywords::PythonKeywords()
{
    qDebug() << "PythonKeywords construtor";

}


PythonKeywords::~PythonKeywords()
{

}

PythonKeywords* PythonKeywords::instance()
{
    static PythonKeywords* inst = 0;
    if(inst == 0)
    {
        inst = new PythonKeywords();
        inst->loadFromFile();
        qSort(inst->m_variables);
        qSort(inst->m_functions);
        qSort(inst->m_keywords);
    }

    return inst;
}

void PythonKeywords::loadFromFile()
{
    qDebug() << "PythonKeywords loadFromFile()";

    //load the known keywords from an xml file
    QFile file(QLatin1String(":xml/keywords.xml"));

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "error opening keywords.xml file. highlighting and completion won't work";
        return;
    }

    QXmlStreamReader xml(&file);

    xml.readNextStartElement();
    while(xml.readNextStartElement())
    {
        const QStringRef name = xml.name();

        if((name == QLatin1String("keywords")) || (name == QLatin1String("variables")) || (name == QLatin1String("functions")))
        {
            while(xml.readNextStartElement())
            {
                Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("word"));

                const QString text = xml.readElementText();

                if(name == QLatin1String("keywords"))
                    m_keywords << text;

                else if(name == QLatin1String("variables")){
                    m_variables << text;
                }

                else if(name == QLatin1String("functions"))
                    m_functions << text;
            }
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError())
    {
        qDebug() << "error parsing element";
        qDebug() << "error: "<< xml.errorString();
    }

}

void PythonKeywords::loadFromModule(const QString& module, const QStringList& keywords)
{
    qDebug() << "Module imported" << module;
    qDebug() << "keywords" << keywords;

    if (module.isEmpty()){
        for(int contKeyword = 0; contKeyword < keywords.size(); contKeyword++){
            m_functions << keywords.at(contKeyword);
        }
    } else {
        m_variables << module;

        for(int contKeyword = 0; contKeyword < keywords.size(); contKeyword++){
            m_functions << module + QLatin1String(".") + keywords.at(contKeyword);
        }
    }
}

void PythonKeywords::addVariable(const QString& variable)
{
    qDebug() << "Variable added" << variable;

    if (!m_variables.contains(variable)){
        m_variables << variable;
    }

}

const QStringList& PythonKeywords::variables() const
{
    return m_variables;
}

const QStringList& PythonKeywords::functions() const
{
    return m_functions;
}

const QStringList& PythonKeywords::keywords() const
{
    return m_keywords;
}
