/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMABACKEND_H
#define _MAXIMABACKEND_H

#include "backend.h"

class MaximaBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit MaximaBackend( QObject* parent = nullptr,const QList<QVariant> args = QList<QVariant>());
    ~MaximaBackend() override;

    QString id() const override;
    QString version() const override;
    Cantor::Session *createSession() override;
    Cantor::Backend::Capabilities capabilities() const override;
    bool requirementsFullfilled(QString* const reason = nullptr) const override;

    QUrl helpUrl() const override;
    QWidget* settingsWidget(QWidget* parent) const override;
    KConfigSkeleton* config() const override;

    QString description() const override;
};


#endif /* _MAXIMABACKEND_H */
