/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
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

    QString toHtml() override;
    QVariant data() override;
    QUrl url() override;

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;
    void saveAdditionalData(KZip* archive) override;

    void save(const QString& filename) override;
  private:
    AnimationResultPrivate* d;

};

}

#endif /* _ANIMATIONRESULT_H */
