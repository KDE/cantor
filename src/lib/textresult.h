/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
*/

#ifndef _TEXTRESULT_H
#define _TEXTRESULT_H

#include "result.h"

#include "cantor_export.h"

namespace Cantor
{

class TextResultPrivate;
class CANTOR_EXPORT TextResult : public Result
{
  public:
    enum { Type=1 };
    enum Format { PlainTextFormat, LatexFormat};

    TextResult(const QString& text);
    TextResult(const QString& text, const QString& plain);
    ~TextResult() override;

    void setIsWarning(bool);
    bool isWarning() const;

    QString toHtml() override;
    QVariant data() override;

    QString plain();

    int type() override;
    QString mimeType() override;

    Format format();
    void setFormat(Format f);

    bool isStderr() const;
    void setStdErr(bool value);

    QDomElement toXml(QDomDocument& doc) override;
    QJsonValue toJupyterJson() override;

    void save(const QString& filename) override;

  private:
    QJsonArray jupyterText(const QString& text, bool addEndNewLine = false);

  private:
    TextResultPrivate* d;
};

}
#endif /* _TEXTRESULT_H */
