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

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <QObject>
#include <QDomElement>

#include "mathematik_export.h"

class KZip;

namespace MathematiK
{
class Session;
class Result;
class ExpressionPrivate;

class MATHEMATIK_EXPORT Expression : public QObject
{
  Q_OBJECT
  public:
    enum Status{ Computing, Done, Error, Interrupted};
    //Enum indicating how this Expression behaves on finishing
    //DoNotDelete means no special behavior
    //DeleteOnFinish means that the Object will delete itself when finished.
    //this is used for fire-and-forget commands
    enum FinishingBehavior { DoNotDelete, DeleteOnFinish };
    Expression( Session* session );
    virtual ~Expression();

    virtual void evaluate() = 0;
    virtual void interrupt() = 0;
    
    int id();
    void setId(int id);

    void setFinishingBehavior(FinishingBehavior behevior);
    FinishingBehavior finishingBehavior();

    void setCommand( const QString& cmd );
    QString command();

    virtual void addInformation(const QString& information);

    void setErrorMessage( const QString& cmd);
    QString errorMessage();

    Result *result();
    Status status();

    Session* session();

    /** returns an xml representation of this expression
     * used for saving the worksheet
     */
    QDomElement toXml(QDomDocument& doc);
    /** saves all the data, that can't be saved in xml 
     *  in an extra file in the archive. for Example 
     *  images of plots
     */
    void saveAdditionalData(KZip* archive);

  Q_SIGNALS:
    void idChanged();
    void gotResult();
    void statusChanged(MathematiK::Expression::Status status);
    void needsAdditionalInformation(const QString& question);

  //These are protected, because only subclasses will handle results/status changes
  protected:
    void setResult(Result* result);
    void setStatus(Status status);

  protected:
    //returns a string of latex commands, that is inserted into the header. 
    //used for example if special packages are needed
    virtual QString additionalLatexHeaders();
  private:
    void renderResultAsLatex();
  private Q_SLOTS:
    void convertToPs();
    void latexRendered();

  private:
    ExpressionPrivate* d;
};

}
#endif /* _EXPRESSION_H */
