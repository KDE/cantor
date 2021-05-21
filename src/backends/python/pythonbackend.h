/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Filipe Saraiva <filipe@kde.org>
*/

#ifndef _PYTHONBACKEND_H
#define _PYTHONBACKEND_H

#include "backend.h"

class PythonBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit PythonBackend(QObject* parent = nullptr, const QList<QVariant>& args = QList<QVariant>());

    QWidget* settingsWidget(QWidget* parent) const override;

    Cantor::Session* createSession() override;

    QString id() const override;
    QString version() const override;
    Cantor::Backend::Capabilities capabilities() const override;
    QUrl helpUrl() const override;
    QString description() const override;
    bool requirementsFullfilled(QString* const reason = nullptr) const override;
    KConfigSkeleton* config() const override;
};


#endif /* _PYTHONBACKEND_H */
