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

    virtual ~JuliaBackend() {}

    /**
     * @see Cantor::Backend::id
     */
    virtual QString id() const override;

    /**
      * @see Cantor::Backend::version
      */
    virtual QString version() const override;

    /**
     * @see Cantor::Backend::createSession
     */
    virtual Cantor::Session *createSession() override;

    /**
     * @see Cantor::Backend::capabilities
     */
    virtual Cantor::Backend::Capabilities capabilities() const override;

    /**
     * @see Cantor::Backend::description
     */
    virtual QString description() const override;

    /**
     * @see Cantor::Backend::helpUrl
     */
    virtual QUrl helpUrl() const override;

    /**
     * @see Cantor::Backend::requirementsFullfilled
     */
    virtual bool requirementsFullfilled() const override;

    /**
     * @see Cantor::Backend::settingsWidget
     */
    virtual QWidget *settingsWidget(QWidget *parent) const override;

    /**
     * @see Cantor::Backend::config
     */
    virtual KConfigSkeleton *config() const override;
};
