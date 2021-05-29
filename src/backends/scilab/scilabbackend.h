/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _SCILABBACKEND_H
#define _SCILABBACKEND_H

#include "backend.h"

class ScilabBackend : public Cantor::Backend
{
    Q_OBJECT

    public:
        explicit ScilabBackend( QObject* parent = nullptr,const QList<QVariant> args = QList<QVariant>());
        ~ScilabBackend() override;

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

#endif /* _SCILABBACKEND_H */
