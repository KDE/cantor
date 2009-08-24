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

#ifndef _MATHEMATIK_EXPORT_H
#define _MATHEMATIK_EXPORT_H

// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>

#ifndef MATHEMATIK_EXPORT
# if defined(MAKE_MATHEMATIKLIBS_LIB)
// We are building this library
#  define MATHEMATIK_EXPORT KDE_EXPORT
# else
// We are using this library
#  define MATHEMATIK_EXPORT KDE_IMPORT
# endif
#endif

# ifndef MATHEMATIK_EXPORT_DEPRECATED
#  define MATHEMATIK_EXPORT_DEPRECATED KDE_DEPRECATED MATHEMATIK_EXPORT
# endif

#endif /* _MATHEMATIK_EXPORT_H */
