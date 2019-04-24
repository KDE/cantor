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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#pragma once

#include "backend.h"

/**
 * Backend for Julia language
 *
 * @see http://julialang.org/
 * @see JuliaServer
 */
class JuliaBackend: public Cantor::Backend
{
  Q_OBJECT
public:
    /**
     * Constructs julia backend
     *
     * @param parent QObject parent. Defaults to nullptr.
     * @param args Additional arguments for the backend.
     *             Defaults to QList<QVariant>().
     */
    explicit JuliaBackend(
        QObject *parent = nullptr,
        const QList<QVariant> &args = QList<QVariant>());

    ~JuliaBackend() override = default;

    /**
     * @see Cantor::Backend::id
     */
    QString id() const override;

    /**
      * @see Cantor::Backend::version
      */
    QString version() const override;

    /**
     * @see Cantor::Backend::createSession
     */
    Cantor::Session *createSession() override;

    /**
     * @see Cantor::Backend::capabilities
     */
    Cantor::Backend::Capabilities capabilities() const override;

    /**
     * @see Cantor::Backend::description
     */
    QString description() const override;

    /**
     * @see Cantor::Backend::helpUrl
     */
    QUrl helpUrl() const override;

    /**
     * @see Cantor::Backend::requirementsFullfilled
     */
    bool requirementsFullfilled(QString* const reason = nullptr) const override;

    /**
     * @see Cantor::Backend::settingsWidget
     */
    QWidget *settingsWidget(QWidget *parent) const override;

    /**
     * @see Cantor::Backend::config
     */
    KConfigSkeleton *config() const override;
};
