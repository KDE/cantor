/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#pragma once

#include <backendtest.h>

class TestJulia: public BackendTest
{
    Q_OBJECT
private Q_SLOTS:
    /**
     * Test simple one-line command. Check that last result is printed
     */
    void testOneLine();
    /**
     * Test one-line command returning `nothing`. No result is printed, except
     * what `print` does
     */
    void testOneLineWithPrint();
    /**
     * Test command, that emits exception
     */
    void testException();
    /**
     * Test command consisting of multiple lines, including comments.
     */
    void testMultilineCode();
    /**
     * Test command with malformed syntax
     */
    void testSyntaxError();
    /**
     * Test that results gathered before exception occured are shown
     */
    void testPartialResultOnException();

    /**
     * Tests that inline plot is shown
     */
    void testInlinePlot();
    /**
     * Tests that when exception occured and plotting is done, partial
     * text results shown to user
     */
    void testInlinePlotWithExceptionAndPartialResult();

    /**
     * Test registering new variables, when added by command
     */
    void testAddVariablesFromCode();
    /**
     * Test registering new variables, when added from variable manager
     */
    void testAddVariablesFromManager();
    /**
     * Test that removing variable unregisters it
     */
    void testRemoveVariables();

    /**
     * Test that auto completion provides expected results
     */
    void testAutoCompletion();

private:
    virtual QString backendName();
};
