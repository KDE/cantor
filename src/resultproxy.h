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

#ifndef _RESULTPROXY_H
#define _RESULTPROXY_H

#include <QObject>
#include <QTextCursor>
#include <QSize>
#include <kurl.h>

namespace Cantor
{
    class Result;
}

/**
   This class is used to translate from the Cantor::Result classes,
   which need to be indipendent of the rendering used, to the actually
   used QTextDocument and the containing QTextObjects
 **/
class ResultProxy : public QObject
{
 Q_OBJECT
  public:
    ResultProxy( QTextDocument* parent );
    ~ResultProxy();

    void insertResult(QTextCursor& pos, Cantor::Result* result);

    void setScale(qreal scale);
    void scale(qreal value);
    qreal scale();

    void useHighResolution(bool use);

    //this doesn't belong here!
    bool renderEpsToResource(const KUrl& url, QSize* size = 0);

  private:
    QTextCharFormat renderEps(Cantor::Result* result);
    QTextCharFormat renderGif(Cantor::Result* result);

  private:    
    QTextDocument* m_document;
    qreal m_scale;
    bool m_useHighRes;
};

#endif /* _RESULTPROXY_H */
