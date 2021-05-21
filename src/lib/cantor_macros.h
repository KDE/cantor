/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _CANTOR_MACROS_H
#define _CANTOR_MACROS_H

#include <KPluginFactory>
#include <KPluginLoader>

/**
  Exports Backend plugin.
*/
#define K_EXPORT_CANTOR_PLUGIN(libname, classname) \
    K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
    K_EXPORT_PLUGIN(factory("cantor_" #libname)) \
    K_EXPORT_PLUGIN_VERSION(CANTOR_VERSION)

#endif /* _CANTOR_MACROS_H */
