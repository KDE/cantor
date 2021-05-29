/*
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANTOR_OCTAVE_BACKEND_H
#define CANTOR_OCTAVE_BACKEND_H

#include <backend.h>

class OctaveBackend : public Cantor::Backend
{
    Q_OBJECT
    public:
    explicit OctaveBackend( QObject* parent = nullptr,const QList<QVariant>& args = QList<QVariant>());
     ~OctaveBackend() override = default;
    QString id() const override;
    QString version() const override;
    Cantor::Backend::Capabilities capabilities() const override;
    Cantor::Session* createSession() override;

    bool requirementsFullfilled(QString* const reason = nullptr) const override;
    QUrl helpUrl() const override;
    QString description() const override;
    QWidget* settingsWidget(QWidget* parent) const override;
    KConfigSkeleton* config() const override;
};

#endif
