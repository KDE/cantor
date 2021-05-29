/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
*/
#pragma once

#include "backend.h"

/**
 * Backend for Julia language
 *
 * @see https://julialang.org/
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
