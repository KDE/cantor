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

#ifndef _SAGEBACKEND_H
#define _SAGEBACKEND_H

#include "backend.h"

class SageBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit SageBackend( QObject* parent = nullptr, const QList<QVariant>& args = QList<QVariant>());
    ~SageBackend() override;

    QString id() const override;
    QString version() const override;
    Cantor::Session *createSession() override;
    Cantor::Backend::Capabilities capabilities() const override;
    bool requirementsFullfilled(QString* const reason = nullptr) const override;

    QWidget* settingsWidget(QWidget* parent) const override;
    KConfigSkeleton* config() const override;

    QUrl helpUrl() const override;
    QString description() const override;
};


#endif /* _SAGEBACKEND_H */
