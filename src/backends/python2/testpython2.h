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
    Copyright (C) 2013 Tuukka Verho <tuukka.verho@aalto.fi>
 */

#ifndef _TESTPYTHON2_H
#define _TESTPYTHON2_H

#include "backendtest.h"


class TestPython2 : public BackendTest
{
  Q_OBJECT

private Q_SLOTS:
    void testImportNumpy();

private:
    virtual QString backendName();
};

#endif /* _TESTPYTHON2_H */
