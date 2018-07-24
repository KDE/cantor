/*
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
*/

#ifndef CANTOR_OCTAVE_BACKEND_H
#define CANTOR_OCTAVE_BACKEND_H

#include <backend.h>

class OctaveBackend : public Cantor::Backend
{
    Q_OBJECT
    public:
    explicit OctaveBackend( QObject* parent = nullptr,const QList<QVariant>& args = QList<QVariant>());
     ~OctaveBackend() override;
    QString id() const Q_DECL_OVERRIDE;
    QString version() const override;
    Cantor::Backend::Capabilities capabilities() const Q_DECL_OVERRIDE;
    Cantor::Session* createSession() Q_DECL_OVERRIDE;

    bool requirementsFullfilled() const Q_DECL_OVERRIDE;
    QUrl helpUrl() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;
    QWidget* settingsWidget(QWidget* parent) const Q_DECL_OVERRIDE;
    KConfigSkeleton* config() const Q_DECL_OVERRIDE;
};

#endif
