/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RBACKEND_H
#define _RBACKEND_H

#include "backend.h"

class RBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit RBackend(QObject* parent = nullptr, const QList<QVariant>& args = QList<QVariant>());
    ~RBackend() override;

    QString id() const override;
    QString version() const override;
    Cantor::Session* createSession() override;
    Cantor::Backend::Capabilities capabilities() const override;
    bool requirementsFullfilled(QString* const reason = nullptr) const override;

    QWidget* settingsWidget(QWidget* parent) const override;
    KConfigSkeleton* config() const override;

    QUrl helpUrl() const override;
    QString defaultHelp() const override;
    QString description() const override;
};


#endif /* _RBACKEND_H */
