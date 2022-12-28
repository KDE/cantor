/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
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

    QString internalCommand() override;

    void parseOutput(const QString&) override {};
    void parseError(const QString&) override {};

    /**
     * Call this function from session when JuliaServer ends evaluation of
     * this expression.
     *
     * This checks inline plots, exceptions and set appropriate result
     */
    void finalize(const QString& output, const QString& error, bool wasException);

public:
    static const QStringList plotExtensions;

private:
    /// If not empty, it's a filename of plot image file expression is awaiting to get
    QString m_plot_filename;
};
