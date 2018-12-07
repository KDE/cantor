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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#pragma once

#include "expression.h"

/**
 * Expression of Julia language
 */
class JuliaExpression: public Cantor::Expression
{
    Q_OBJECT
public:
    /**
     * Creates new JuliaExpression
     *
     * @param session session to bound expression to
     * @param internal @see Cantor::Expression::Expression(Session*, bool)
     */
    explicit JuliaExpression(Cantor::Session *session, bool internal = false);
    ~JuliaExpression() override = default;

    /**
     * @see Cantor::Expression::evaluate
     */
    void evaluate() override;

    /**
     * @see Cantor::Expression::interrupt
     */
    void interrupt() override;

    QString internalCommand() override;

    /**
     * Call this function from session when JuliaServer ends evaluation of
     * this expression.
     *
     * This checks inline plots, exceptions and set appropriate result
     */
    void finalize();

private:
    /// If not empty, it's a filename of plot image file expression is awaiting
    /// to get
    QString m_plot_filename;


    /**
     * @return bool indicator if current expression contains command that
     *              shows plot
     */
    bool checkPlotShowingCommands();
};
