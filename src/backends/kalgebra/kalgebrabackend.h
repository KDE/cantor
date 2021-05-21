/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALGEBRA_BACKEND_H
#define KALGEBRA_BACKEND_H

#include "backend.h"
class KAlgebraBackend : public Cantor::Backend
{
    Q_OBJECT
    public:
        explicit KAlgebraBackend( QObject* parent = nullptr, const QList<QVariant> args = QList<QVariant>());
        ~KAlgebraBackend() override = default;

        QString id() const override;
        Cantor::Session *createSession() override;
        Cantor::Backend::Capabilities capabilities() const override;

        QWidget* settingsWidget(QWidget* parent) const override;
        KConfigSkeleton* config() const override;

        QUrl helpUrl() const override;
        QString version() const override;
        bool requirementsFullfilled(QString* const reason) const override;
};

#endif /* _NULLBACKEND_H */
