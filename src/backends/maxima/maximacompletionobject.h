/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _MAXIMACOMPLETIONOBJECT_H
#define _MAXIMACOMPLETIONOBJECT_H

#include "completionobject.h"

class MaximaSession;

class MaximaCompletionObject : public Cantor::CompletionObject
{
  public:
    MaximaCompletionObject( const QString& cmd, int index, MaximaSession* session );
    ~MaximaCompletionObject() override = default;

  protected:
    bool mayIdentifierContain(QChar c) const override;
    bool mayIdentifierBeginWith(QChar c) const override;

  protected Q_SLOTS:
    void fetchCompletions() override;
    void fetchIdentifierType() override;
};

#endif /* _MAXIMACOMPLETIONOBJECT_H */
