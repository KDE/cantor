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

#ifndef _RCALLBACKS_H
#define _RCALLBACKS_H

class RServer;
class Expression;

//R includes
#include <R.h>
#include <Rembedded.h>
#include <Rversion.h>
#include <Rdefines.h>
#define R_INTERFACE_PTRS
#include <R_ext/Parse.h>

//This File implements the necessary callbacks for R
//The information gathered will be pushed back to the RThread

void setupCallbacks(RServer* server);
void setCurrentExpression(Expression* expression);

void onWriteConsoleEx(const char* text, int size,int otype);
void onShowMessage(const char* text);
void onBusy(int which);
int  onReadConsole(const char* prompt, unsigned char* buf, int buflen, int hist);
int  onShowFiles(int nfile, const char** file, const char** headers, const char* wtitle, Rboolean del, const char* pager);

#endif /* _RCALLBACKS_H */
