/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _ANIMATIONRESULT_H
#define _ANIMATIONRESULT_H

#include "result.h"

namespace Cantor
{
class AnimationResultPrivate;

class CANTOR_EXPORT AnimationResult : public Result
{
  public:
    enum{Type=6};
    explicit AnimationResult( const QUrl& url, const QString& alt=QString() );
    ~AnimationResult() override;

    QString toHtml() Q_DECL_OVERRIDE;
    QVariant data() Q_DECL_OVERRIDE;
    QUrl url() Q_DECL_OVERRIDE;

    int type() Q_DECL_OVERRIDE;
    QString mimeType() Q_DECL_OVERRIDE;

    QDomElement toXml(QDomDocument& doc) Q_DECL_OVERRIDE;
    void saveAdditionalData(KZip* archive) Q_DECL_OVERRIDE;

    void save(const QString& filename) Q_DECL_OVERRIDE;
  private:
    AnimationResultPrivate* d;

};

}

#endif /* _ANIMATIONRESULT_H */
