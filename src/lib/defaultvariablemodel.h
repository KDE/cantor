/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2010 Miha Čančula <miha.cancula@gmail.com>
*/

#ifndef CANTOR_DEFAULTVARIABLEMODEL_H
#define CANTOR_DEFAULTVARIABLEMODEL_H

#include <QAbstractTableModel>
#include "session.h"
#include "expression.h"

namespace Cantor {

class DefaultVariableModelPrivate;

/**
 * @brief
 * This DefaultVariableModel class is an implementation of QAbstractItemModel
 * that can be used with the Variable Manager plugin.
 *
 * For most uses the addVariable(), removeVariable() and clearVariables() methods are sufficient.
 * They can be used from session (directly or by connecting signals to them), or called from
 * a subclass.
 *
 * DefaultVariableModel uses the session to run expressions for changing variables, and it
 * gets the commands from the backend's VariableManagementExtension.
 * If you do not want this behavior, you can subclass it and reimplement data() and/or setData().
 *
 * @see Session::variableModel()
 */
class CANTOR_EXPORT DefaultVariableModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(Session* session READ session)

public:

    enum {DataRole = Qt::UserRole + 1};

    /**
     * A structure representing a variable.
     */
    struct Variable
    {
        Variable(): size(0) {}
        Variable(QString name, QString value, size_t size = 0): name(name), value(value), size(size) {}
        /**
         * The variable's name
         */
        QString name;
        /**
         * The variable's value, represented as a string
         */
        QString value;

        /**
         * Optional parameter. Size of variable in bytes
         */
        size_t size;
    };

    /**
     * Default constructor
     * If you are constructing a DefaultVariableModel without subclassing, the @p session must be valid
     * and its backends must support a VariableManagementExtension.
     *
     * This requirement can be avoided by reimplementing setData() in a subclass.
     *
     * @param session the session this Model belongs to, also becomes the Model's parent.
     */
    explicit DefaultVariableModel(Session* session);
    ~DefaultVariableModel() override;

    /**
     * Get the session which created this Model and whose variables it contains
     * @return the session
     */
    Session* session() const;

    /**
     * Returns variables, stored in this model, as @see Variable.
     */
    QList<Variable> variables() const;

    /**
     * Returns names of stored variables
     */
    QStringList variableNames() const;

    /**
     * Return functions, stored in this model
     */
    QStringList functions() const;

    //TODO: improve the description?
    /**
     * Starts updating variable model (variable lists, etc.). Usually executed after finished all user's commands
     */
    virtual void update() {};

public Q_SLOTS:
    /**
     * Adds a variable to the model.
     * If a variable with the same name already exists, it will be overwritten.
     * @param name the name of the variable
     * @param value the value of the variable
     */
    void addVariable(const QString& name, const QString& value);
    /**
     * Convenience method, equivalent to addVariable(variable.name, variable.value)
     * @param variable the variable to add
     */
    void addVariable(const Cantor::DefaultVariableModel::Variable& variable);
    /**
     * Remove the variable @p name from the model.
     * If a variable with the specified @p name doesn't exists, this method does nothing.
     * @param name the name of the variable to remove
     */
    void removeVariable(const QString& name);
    /**
     * Convenience method, equivalent to removeVariable(variable.name)
     * @param variable the variable to remove
     */
    void removeVariable(const Cantor::DefaultVariableModel::Variable& variable);
    /**
     * Clears all variables from the model
     */
    void clearVariables();

    /**
     * Clears all functions
     */
    void clearFunctions();


Q_SIGNALS:
    /**
     * Emitted after adding new variables
     * @param variables list of new variables
     */
    void variablesAdded(const QStringList& variables);

    /**
     * Emitted after variables removing
     * @param variables list of removed variables
     */
    void variablesRemoved(const QStringList& variables);

    /**
     * Similar to @c variablesAdded
     */
    void functionsAdded(const QStringList& names);

    /**
     * Similar to @c variablesRemoved
     */
    void functionsRemoved(const QStringList funcs);

protected:
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setVariables(const QList<DefaultVariableModel::Variable>& newVars);
    void setFunctions(const QStringList& newFuns);

    enum Column
    {
        NameColumn = 0,
        ValueColumn = 1,
        ColumnCount = 2
    };

private:

    DefaultVariableModelPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(DefaultVariableModel)
};

bool operator==(const Cantor::DefaultVariableModel::Variable& one, const Cantor::DefaultVariableModel::Variable& other);

}

#endif // CANTOR_DEFAULTVARIABLEMODEL_H
