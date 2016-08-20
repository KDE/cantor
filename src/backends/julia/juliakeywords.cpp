/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliakeywords.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QtAlgorithms>
#include <QStandardPaths>
#include <QDebug>

JuliaKeywords *JuliaKeywords::instance()
{
    static JuliaKeywords *inst = 0;
    if (inst == 0) {
        inst = new JuliaKeywords();
        inst->loadFromFile();
        qSort(inst->m_keywords);
        qSort(inst->m_variables);
    }

    return inst;
}

void JuliaKeywords::loadFromFile()
{
    //load the known keywords from an xml file
    QFile file(
        QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QLatin1String("cantor/juliabackend/keywords.xml")
        )
    );

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "error opening keywords.xml file. highlighting and"
                   << "completion won't work";
        return;
    }

    QXmlStreamReader xml(&file);

    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        const QStringRef name = xml.name();

        if (name == QLatin1String("keywords")
                or name == QLatin1String("variables")) {
            while (xml.readNextStartElement()) {
                Q_ASSERT(
                    xml.isStartElement() and xml.name() == QLatin1String("word")
                );

                const QString text = xml.readElementText();

                if (name == QLatin1String("keywords")) {
                    m_keywords << text;
                } else if (name == QLatin1String("variables")) {
                    m_variables << text;
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError()) {
        qWarning() << "Error parsing keywords.xml:" << xml.errorString();
    }
}
