/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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
