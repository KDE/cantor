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

#include "session.h"
using namespace Cantor;

#include "backend.h"
#include "defaultvariablemodel.h"

#include <QTimer>
#include <QQueue>
#include <KMessageBox>
#include <KLocalizedString>

class Cantor::SessionPrivate
{
  public:
    SessionPrivate() : backend(nullptr), status(Session::Disable), typesettingEnabled(false), expressionCount(0), variableModel(nullptr), needUpdate(false)
    {
    }

    Backend* backend;
    Session::Status status;
    bool typesettingEnabled;
    int expressionCount;
    QList<Cantor::Expression*> expressionQueue;
    DefaultVariableModel* variableModel;
    bool needUpdate;
};

Session::Session( Backend* backend ) : QObject(backend), d(new SessionPrivate)
{
    d->backend=backend;
}

Session::Session( Backend* backend, DefaultVariableModel* model) : QObject(backend), d(new SessionPrivate)
{
    d->backend=backend;
    d->variableModel=model;
}

Session::~Session()
{
    delete d;
}

void Session::logout()
{
    if (d->status == Session::Running)
        interrupt();

    if (d->variableModel)
    {
        d->variableModel->clearVariables();
        d->variableModel->clearFunctions();
    }
    d->expressionCount = 0;
    changeStatus(Status::Disable);
}

QList<Expression*>& Cantor::Session::expressionQueue() const
{
    return d->expressionQueue;
}

void Session::enqueueExpression(Expression* expr)
{
    d->expressionQueue.append(expr);

    //run the newly added expression immediately if it's the only one in the queue
    if (d->expressionQueue.size() == 1)
    {
        changeStatus(Cantor::Session::Running);
        runFirstExpression();
    }
    else
        expr->setStatus(Cantor::Expression::Queued);
}

void Session::runFirstExpression()
{

}

void Session::finishFirstExpression(bool setDoneAfterUpdate)
{
    if (!d->expressionQueue.isEmpty())
        d->needUpdate |= !d->expressionQueue.takeFirst()->isInternal();

    if (d->expressionQueue.isEmpty())
        if (d->variableModel && d->needUpdate)
        {
            d->variableModel->update();
            d->needUpdate = false;

            // Some variable models could update internal lists without running expressions
            // So, if after update queue still empty, set status to Done
            // setDoneAfterUpdate used for compatibility with some backends, like R
            if (setDoneAfterUpdate && d->expressionQueue.isEmpty())
                changeStatus(Done);
        }
        else
            changeStatus(Done);
    else
        runFirstExpression();
}

Backend* Session::backend()
{
    return d->backend;
}

Cantor::Session::Status Session::status()
{
    return d->status;
}

void Session::changeStatus(Session::Status newStatus)
{
    d->status=newStatus;
    emit statusChanged(newStatus);
}

void Session::setTypesettingEnabled(bool enable)
{
    d->typesettingEnabled=enable;
}

bool Session::isTypesettingEnabled()
{
    return d->typesettingEnabled;
}

void Session::setWorksheetPath(const QString& path)
{
    Q_UNUSED(path);
    return;
}

CompletionObject* Session::completionFor(const QString& cmd, int index)
{
    Q_UNUSED(cmd);
    Q_UNUSED(index);
    //Return 0 per default, so Backends not offering tab completions don't have
    //to reimplement this. This method should only be called on backends with
    //the Completion Capability flag

    return nullptr;
}

SyntaxHelpObject* Session::syntaxHelpFor(const QString& cmd)
{
    Q_UNUSED(cmd);

    //Return 0 per default, so Backends not offering tab completions don't have
    //to reimplement this. This method should only be called on backends with
    //the SyntaxHelp Capability flag
    return nullptr;
}

QSyntaxHighlighter* Session::syntaxHighlighter(QObject* parent)
{
    Q_UNUSED(parent);
    return nullptr;
}

DefaultVariableModel* Session::variableModel() const
{
    //By default, there is variableModel in session, used by syntax higlighter for variable analyzing
    //The model store only variable names by default.
    //In backends with VariableManagement Capability flag, this model also used for Cantor variable doc panel
    return d->variableModel;
}

QAbstractItemModel* Session::variableDataModel() const
{
    return variableModel();
}

void Session::updateVariables()
{
    if (d->variableModel)
    {
        d->variableModel->update();
        d->needUpdate = false;
    }
}

void Cantor::Session::setVariableModel(Cantor::DefaultVariableModel* model)
{
    d->variableModel = model;
}

int Session::nextExpressionId()
{
    return d->expressionCount++;
}

QString Session::locateCantorFile(const QString& partialPath, QStandardPaths::LocateOptions options)
{
    QString file = QStandardPaths::locate(QStandardPaths::AppDataLocation, partialPath, options);

    if (file.isEmpty())
        file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("cantor/") + partialPath, options);

    return file;
}

QStringList Session::locateAllCantorFiles(const QString& partialPath, QStandardPaths::LocateOptions options)
{
    QStringList files = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, partialPath, options);

    if (files.isEmpty())
        files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("cantor/") + partialPath, options);

    return files;
}

void Cantor::Session::reportSessionCrash(const QString& additionalInfo)
{
    // Reporting about crashing backend in session without backend has not sense
    if (d->backend == nullptr)
        return;

    if (additionalInfo.isEmpty())
        KMessageBox::error(nullptr, i18n("%1 process has died unexpectedly. All calculation results are lost.", d->backend->name()), i18n("Error - Cantor"));
    else
        KMessageBox::error(nullptr, i18n("%1 process has died unexpectedly with message \"%2\". All calculation results are lost.", d->backend->name()), i18n("Error - Cantor"));
    logout();
}
