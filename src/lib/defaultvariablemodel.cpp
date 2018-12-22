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
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>
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
        d->extension = dynamic_cast<Cantor::VariableManagementExtension*>(session->backend()->extension(QLatin1String("VariableManagementExtension")));
    }
    qDebug() << d->session << d->extension;
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
    if (role != Qt::DisplayRole || !index.isValid())
    {
        return QVariant();
    }

    Q_D(const DefaultVariableModel);
    switch (index.column())
    {
        case NameColumn:
            return QVariant(d->variables[index.row()].name);
        case ValueColumn:
            return QVariant(d->variables[index.row()].value);
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
        return true;
    }
    else if(index.column() == NameColumn)
    {
        // Renaming => copy it, then delete the old one
        QString oldName = data(index).toString();
        QString variableValue = data(index.sibling(index.row(), ValueColumn)).toString();
        d->session->evaluateExpression(d->extension->addVariable(value.toString(), variableValue), Expression::DeleteOnFinish);
        d->session->evaluateExpression(d->extension->removeVariable(oldName), Expression::DeleteOnFinish);
        return true;
    }
    return false;
}

void DefaultVariableModel::addVariable(const QString& name, const QString& value)
{
    Variable v;
    v.name = name;
    v.value = value;
    addVariable(v);
}

void DefaultVariableModel::addVariable(const Cantor::DefaultVariableModel::Variable& variable)
{
    Q_D(DefaultVariableModel);
    if ( d->variables.contains(variable) )
    {
        // TODO: Why we remove variable here? Set value properly
        removeVariable(variable);
    }
    beginInsertRows(QModelIndex(), d->variables.size(), d->variables.size());
    d->variables.append(variable);
    emit variablesAdded(QStringList(variable.name));
    endInsertRows();
}

void DefaultVariableModel::removeVariable(const QString& name)
{
    Variable v;
    v.name = name;
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
    for (const Variable var: d->variables)
        names.append(var.name);

    d->variables.clear();
    endResetModel();

    emit variablesRemoved(names);
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
    for (const Variable newvar : newVars)
    {
        bool found = false;
        for (int i = 0; i < size; i++)
            if(d->variables[i].name == newvar.name)
            {
                found=true;
                if (d->variables[i].value != newvar.value)
                {
                    QModelIndex index = createIndex(i, ValueColumn);
                    QAbstractItemModel::setData(index, newvar.value);
                    d->variables[i].value = newvar.value;
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
    for (const Variable var: d->variables)
        names << var.name;
    return names;
}

bool operator==(const Cantor::DefaultVariableModel::Variable& one, const Cantor::DefaultVariableModel::Variable& other)
{
    return one.name == other.name;
}

}
