/************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                             *
*  Copyright (C) 2011 by Matteo Agostinelli <agostinelli@gmail.com>                 *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#ifndef QALCULATE_BACKEND_H
#define QALCULATE_BACKEND_H

#include "backend.h"

class QalculateBackend : public Cantor::Backend
{
    Q_OBJECT

public:
    explicit QalculateBackend( QObject* parent = 0,const QList<QVariant> args = QList<QVariant>());
    ~QalculateBackend();

    QString id() const;
    QString version() const override;

    virtual Cantor::Session *createSession();
    virtual Cantor::Backend::Capabilities capabilities() const;
    virtual QString description() const;
    virtual QUrl helpUrl() const;

    QWidget* settingsWidget(QWidget* parent) const;
    KConfigSkeleton* config() const;
};


#endif /* QALCULATE_BACKEND_H */
