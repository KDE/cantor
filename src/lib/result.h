/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _RESULT_H
#define _RESULT_H

#include <QVariant>
#include <QDomElement>
#include <QJsonArray>
#include "cantor_export.h"

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
     * return a Jupyter json object, containing the information of the result
     */
    virtual QJsonValue toJupyterJson() = 0;
    /**
     * saves this to a file
     * @param filename name of the file
     */
    virtual void save(const QString& filename) = 0;

    /**
     * This functions handle Jupyter metadata of
     */
    QJsonObject jupyterMetadata() const;
    void setJupyterMetadata(const QJsonObject&);

    /**
     * Allow to set execution result index, on this moment useful only for Jupyter
     * But maybe Cantor can use it too
     */
    int executionIndex() const;
    void setExecutionIndex(int index);

  private:
    ResultPrivate* d;
};

}

#endif /* _RESULT_H */
