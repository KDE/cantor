/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2011 Matteo Agostinelli <agostinelli@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QALCULATE_BACKEND_H
#define QALCULATE_BACKEND_H

#include "backend.h"

class QalculateBackend : public Cantor::Backend
{
    Q_OBJECT

public:
    explicit QalculateBackend( QObject* parent = nullptr, const QList<QVariant> args = QList<QVariant>());
    ~QalculateBackend() override = default;

    QString id() const override;
    QString version() const override;

    Cantor::Session *createSession() override;
    Cantor::Backend::Capabilities capabilities() const override;
    QString description() const override;
    QUrl helpUrl() const override;
    bool requirementsFullfilled(QString* const reason = nullptr) const override;

    QWidget* settingsWidget(QWidget* parent) const override;
    KConfigSkeleton* config() const override;
};


#endif /* QALCULATE_BACKEND_H */
