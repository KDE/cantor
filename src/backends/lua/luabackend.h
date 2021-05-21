/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014 Lucas Hermann Negri <lucashnegri@gmail.com>
*/

#ifndef _LUABACKEND_H
#define _LUABACKEND_H

#include "backend.h"

class LuaBackend : public Cantor::Backend
{
Q_OBJECT
public:
    explicit LuaBackend( QObject* parent = nullptr, const QList<QVariant> args = QList<QVariant>());
    ~LuaBackend() override = default;

    QString id() const override;
    QString version() const override;

    Cantor::Session* createSession() override;
    Cantor::Backend::Capabilities capabilities() const override;

    bool requirementsFullfilled(QString* const reason = nullptr) const override;
    QUrl helpUrl() const override;
    QString description() const override;

    QWidget *settingsWidget(QWidget *parent) const override;
    KConfigSkeleton *config() const override;
};


#endif /* _LUABACKEND_H */
