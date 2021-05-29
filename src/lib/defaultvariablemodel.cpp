/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
*/

#include "defaultvariablemodel.h"
#include <QDebug>
#include <KLocalizedString>
#include "extension.h"
#include "backend.h"

namespace Cantor
{

class DefaultVariableModelPrivate
{
public:
    QList<DefaultVariableModel::Variable> variables;
    QStringList functions;

    Session* session;
    VariableManagementExtension* extension;
};

DefaultVariableModel::DefaultVariableModel(Session* session): QAbstractTableModel(session),
d_ptr(new DefaultVariableModelPrivate)
{
    Q_D(DefaultVariableModel);
    d->session = session;
    if (session)
    {
        d->extension = dynamic_cast<Cantor::VariableManagementExtension*>(session->backend()->extension(QStringLiteral("VariableManagementExtension")));
    }
    qDebug() << d->session << d->extension;
}

DefaultVariableModel::~DefaultVariableModel()
{
    Q_D(DefaultVariableModel);
    delete d;
}

int DefaultVariableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return ColumnCount;
}

int DefaultVariableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    else
    {
        Q_D(const DefaultVariableModel);
        return d->variables.size();
    }
}

QVariant DefaultVariableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole && orientation==Qt::Horizontal) {
        switch(section) {
            case NameColumn:
                return i18nc("@title:column", "Name");

            case ValueColumn:
                return i18nc("@title:column", "Value");
                break;
        }
    }
    return QVariant();
}

Qt::ItemFlags DefaultVariableModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant DefaultVariableModel::data(const QModelIndex& index, int role) const
{
    if ((role != Qt::DisplayRole && role != DataRole) || !index.isValid())
    {
        return QVariant();
    }

    Q_D(const DefaultVariableModel);
    switch (index.column())
    {
        case NameColumn:
            return QVariant(d->variables[index.row()].name);
        case ValueColumn:
        {
            const Variable& var = d->variables[index.row()];
            if (var.value.size() < 1000 || role == DefaultVariableModel::DataRole)
                return QVariant(var.value);
            else
            {
                if (var.size != 0)
                    return QVariant(i18n("<%1 bytes>", QString::number(var.size)));
                else
                    return QVariant(i18n("<too big variable>"));
            }
        }
        default:
            return QVariant();
    }
}

bool DefaultVariableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(role!=Qt::EditRole || !value.isValid() || !index.isValid())
    {
        return false;
    }

    Q_D(const DefaultVariableModel);
    if(index.column() == ValueColumn)
    {
        // Changing values
        QString name = data(index.sibling(index.row(), NameColumn)).toString();
        d->session->evaluateExpression(d->extension->setValue(name, value.toString()), Expression::DeleteOnFinish);
        emit dataChanged(index, index);
        return true;
    }
    else if(index.column() == NameColumn)
    {
        // Renaming => copy it, then delete the old one
        QString oldName = data(index).toString();
        QString variableValue = data(index.sibling(index.row(), ValueColumn)).toString();
        d->session->evaluateExpression(d->extension->addVariable(value.toString(), variableValue), Expression::DeleteOnFinish);
        d->session->evaluateExpression(d->extension->removeVariable(oldName), Expression::DeleteOnFinish);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void DefaultVariableModel::addVariable(const QString& name, const QString& value)
{
    Variable v(name, value);
    addVariable(v);
}

void DefaultVariableModel::addVariable(const Cantor::DefaultVariableModel::Variable& variable)
{
    Q_D(DefaultVariableModel);
    int index = d->variables.indexOf(variable);
    if (index != -1)
    {
        d->variables[index].value = variable.value;
        QModelIndex modelIdx = createIndex(index, ValueColumn);
        emit dataChanged(modelIdx, modelIdx);
    }
    else
    {
        beginInsertRows(QModelIndex(), d->variables.size(), d->variables.size());
        d->variables.append(variable);
        emit variablesAdded(QStringList(variable.name));
        endInsertRows();
    }
}

void DefaultVariableModel::removeVariable(const QString& name)
{
    Variable v(name, QString());
    removeVariable(v);
}

void DefaultVariableModel::removeVariable(const Cantor::DefaultVariableModel::Variable& variable)
{
    Q_D(DefaultVariableModel);
    int row = d->variables.indexOf(variable);
    if(row==-1)
        return;
    const QString& name = variable.name;
    beginRemoveRows(QModelIndex(), row, row);
    d->variables.removeAt(row);
    endRemoveRows();
    emit variablesRemoved(QStringList(name));
}

void DefaultVariableModel::clearVariables()
{
    Q_D(DefaultVariableModel);
    beginResetModel();

    QStringList names;
    for (const Variable& var: d->variables)
        names.append(var.name);

    d->variables.clear();
    endResetModel();

    emit variablesRemoved(names);
}

void DefaultVariableModel::clearFunctions()
{
    Q_D(DefaultVariableModel);
    QStringList names = d->functions;
    d->functions.clear();
    emit functionsRemoved(names);
}

void DefaultVariableModel::setVariables(const QList<DefaultVariableModel::Variable>& newVars)
{
    Q_D(DefaultVariableModel);

    QStringList addedVars;
    QStringList removedVars;

    // Handle deleted vars
    int i = 0;
    while (i < d->variables.size())
    {
        Variable var = d->variables[i];
        bool found = false;
        for (const Variable& newvar : newVars)
            if(var.name == newvar.name)
            {
                found=true;
                break;
            }

        if (!found)
        {
            removedVars << var.name;
            beginRemoveRows(QModelIndex(), i, i);
            d->variables.removeAt(i);
            endRemoveRows();
        }
        else
            i++;
    }

    // Handle added vars
    const int size = d->variables.size();
    for (const Variable& newvar : newVars)
    {
        bool found = false;
        for (int i = 0; i < size; i++)
            if(d->variables[i].name == newvar.name)
            {
                found=true;
                if (d->variables[i].value != newvar.value)
                {
                    QModelIndex index = createIndex(i, ValueColumn);
                    d->variables[i].value = newvar.value;
                    d->variables[i].size = newvar.size;
                    emit dataChanged(index, index);
                }
                break;
            }

        if (!found)
        {
            addedVars << newvar.name;
            beginInsertRows(QModelIndex(), d->variables.size(), d->variables.size());
            d->variables.append(newvar);
            endInsertRows();
        }
    }

    emit variablesAdded(addedVars);
    emit variablesRemoved(removedVars);
}

void DefaultVariableModel::setFunctions(const QStringList& newFuncs)
{
    Q_D(DefaultVariableModel);
    QStringList addedFuncs;
    QStringList removedFuncs;

    //remove the old variables
    int i = 0;
    while (i < d->functions.size())
    {
        //check if this var is present in the new variables
        bool found=false;
        for (const QString& func : newFuncs)
            if(d->functions[i] == func)
            {
                found=true;
                break;
            }

        if(!found)
        {
            removedFuncs<<d->functions[i];
            d->functions.removeAt(i);
        }
        else
            i++;
    }

    for (const QString& func : newFuncs)
    {
        if (!d->functions.contains(func))
        {
            addedFuncs<<func;
            d->functions.append(func);
        }
    }

    emit functionsAdded(addedFuncs);
    emit functionsRemoved(removedFuncs);
}

Session* DefaultVariableModel::session() const
{
    Q_D(const DefaultVariableModel);
    return d->session;
}

QList<DefaultVariableModel::Variable> DefaultVariableModel::variables() const
{
    Q_D(const DefaultVariableModel);
    return d->variables;
}

QStringList DefaultVariableModel::variableNames() const
{
    Q_D(const DefaultVariableModel);
    QStringList names;
    for (const Variable& var: d->variables)
        names << var.name;
    return names;
}

QStringList DefaultVariableModel::functions() const
{
    Q_D(const DefaultVariableModel);
    return d->functions;
}

bool operator==(const Cantor::DefaultVariableModel::Variable& one, const Cantor::DefaultVariableModel::Variable& other)
{
    return one.name == other.name;
}

}
