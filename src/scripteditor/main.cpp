/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2018 Sirgienko Nikita <warquark@gmail.com>
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
