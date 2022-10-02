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
    Copyright (C) 2020-2022 Alexander Semke <alexander.semke@web.de>
 */

#ifndef BACKENDSETTINGSWIDGET_H
#define BACKENDSETTINGSWIDGET_H

#include <QWidget>

class QTabWidget;
class QtHelpConfig;
class KUrlRequester;

class BackendSettingsWidget : public QWidget
{
Q_OBJECT

public:
    explicit BackendSettingsWidget(QWidget* parent = nullptr, const QString& id = QString());

private:
    QtHelpConfig* m_docWidget = nullptr;

protected:
    QString m_id;
    QTabWidget* m_tabWidget = nullptr;
    QWidget* m_tabDocumentation = nullptr;
    KUrlRequester* m_urlRequester = nullptr;

public Q_SLOTS:
    void tabChanged(int);
    void fileNameChanged(const QString&);
};

#endif /* BACKENDSETTINGSWIDGET_H */
