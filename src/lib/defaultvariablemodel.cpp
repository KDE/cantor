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
#include <KDebug>
#include <KLocale>
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
        d->extension = dynamic_cast<Cantor::VariableManagementExtension*>(session->backend()->extension("VariableManagementExtension"));
    }
    kDebug() << d->session << d->extension;
}

DefaultVariableModel::~DefaultVariableModel()
{

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
        removeVariable(variable);
    }
    beginInsertRows(QModelIndex(), d->variables.size(), d->variables.size());
    d->variables.append(variable);
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
    beginRemoveRows(QModelIndex(), row, row);
    d->variables.removeAt(row);
    endRemoveRows();
}

void DefaultVariableModel::clearVariables()
{
    Q_D(DefaultVariableModel);
    beginResetModel();
    d->variables.clear();
    endResetModel();
}

Session* DefaultVariableModel::session() const
{
    Q_D(const DefaultVariableModel);
    return d->session;
}

}

bool operator==(const Cantor::DefaultVariableModel::Variable& one, const Cantor::DefaultVariableModel::Variable& other)
{
    return one.name == other.name;
}

#include  "defaultvariablemodel.moc"

