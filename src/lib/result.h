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

#ifndef _RESULT_H
#define _RESULT_H

#include <QVariant>
#include <QDomElement>
#include <cantor_export.h>

class KZip;

namespace Cantor
{

class ResultPrivate;

/**
 * Base class for different results, like text, image, animation. etc.
 */
class CANTOR_EXPORT Result
{
  public:
    /**
     * Default constructor
     */
    Result( );
    /**
     * Destructor
     */
    virtual ~Result();

    /**
     * returns html code, that represents this result,
     * e.g. an img tag for images
     * @return html code representing this result
     */
    virtual QString toHtml() = 0;

    /**
     * returns latex code, that represents this result
     * e.g. a includegraphics command for images
     * it falls back to toHtml if not implemented
     * @return latex code representing this result
     */
    virtual QString toLatex();

    /**
     * returns data associated with this result
     * (text/images/etc)
     * @return data associated with this result
     */
    virtual QVariant data() = 0;

    /**
     * returns an url, data for this result resides at
     * @return an url, data for this result resides at
     */
    virtual QUrl url();

    /**
     * returns an unique number, representing the type of this
     * result. Every subclass should define their own Type.
     * @return the type of this result
     */
    virtual int type() = 0;
    /**
     * returns the mimetype, this result is
     * @return the mimetype, this result is
     */
    virtual QString mimeType() = 0;

    /**
     * returns a DomElement, containing the information of the result
     * @param doc DomDocument used for storing the information
     * @return DomElement, containing the information of the result
     */
    virtual QDomElement toXml(QDomDocument& doc) = 0;
    /**
     * saves all the data, that can't be saved in xml
     * in an extra file in the archive.
     */
    virtual void saveAdditionalData(KZip* archive);

    /**
     * saves this to a file
     * @param filename name of the file
     */
    virtual void save(const QString& filename) = 0;
  private:
    ResultPrivate* d;
};

}

#endif /* _RESULT_H */
