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
    Copyright (C) 2018 Sirgienko Nikita <warquark@gmail.com>
 */

#include <QApplication>
#include <QUrl>
#include <QDebug>
#include "scripteditorwidget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ScriptEditorWidget* editor = new ScriptEditorWidget(QLatin1String(""),QLatin1String(""));

    if (argc == 2)
        {
        // Open file, passed in arguments
        QString filename = QLatin1String(argv[1]);
        QUrl path = QUrl::fromLocalFile(filename);
        qDebug() << "open " << path;
        editor->open(path);
        }

    editor->show();

    return app.exec();
}
