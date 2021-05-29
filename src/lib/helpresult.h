/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _HELPRESULT_H
#define _HELPRESULT_H

#include "textresult.h"

namespace Cantor
{

/** this is basically a TextResult, just with a different Type
    so that the application can show it in another way than the
    normal results
**/
class HelpResultPrivate;
class CANTOR_EXPORT HelpResult: public Result
{
  public:
    enum {Type=3};
    explicit HelpResult( const QString& text, bool isHtml=false);
    ~HelpResult() override;

    QVariant data() override;
    QString toHtml() override;

    int type() override;
    QString mimeType() override;

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;
    void save(const QString& filename) override;

  private:
    HelpResultPrivate* d;
};

}

#endif /* _HELPRESULT_H */
