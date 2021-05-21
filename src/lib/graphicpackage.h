/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Sirgienko Nikita <warquark@gmail.com>
*/

#ifndef _GRAPHICPACKAGE_H
#define _GRAPHICPACKAGE_H

#include <QList>
#include <QString>

#include "cantor_export.h"

namespace Cantor
{
class GraphicPackagePrivate;
class Session;
class Expression;

/**
 * This class represents of embedded graphic handler for certaion graphical package some of @c Backend
 * It provides access to native backend code (octave code for Octave backend, python code for Python backend, etc)
 * for few operations, which need for embedded graphics.
 *
 * @author Nikita Sirgienko
 */

class CANTOR_EXPORT GraphicPackage {
public:
    /// @c GraphicPackage can be only copied or load from disk via loadFromFile() function. Direct construction prohibited.
    GraphicPackage(const GraphicPackage&);
    ~GraphicPackage();

    /// This is id of graphical package. Must be unique, because the id used in search operations
    QString id() const;

    /// Name of package, which will be shown to user in some situation. Can be nonunique.
    QString name() const;

    /**
     * @brief This methor return @c Expression object, which will check requirements of the package.
     *
     * For example, using "matplotlib" graphic package for Python backend have sense only if Python module "matplotlib" installed.
     * So, the expression from @c isAvailable for "matplotlib" graphic package will check, if this module installed
     * @return Expression, which will have output @c "1" if requirements are fulfilled and "0" or just an error if they aren't fulfilled
     */
    Expression* isAvailable(Session*) const;

    /**
     * @brief This command should return code, which will enable capturing images.
     *
     * The command must be revertable, see disableSupportCommand().
     * @c additionalInfo This is additional parameter from backend, which go to @c "%1" template. This is optional.
     */
    QString enableSupportCommand(QString additionalInfo = QString()) const;

    /// This command should return code, which will disable capturing images.
    QString disableSupportCommand() const;

    /**
     * @brief The method return @c true if plot command empty and @c false otherwise
     *
     * The packages can have two realization of capturing.
     * 1. Need only enable/disable commands. Capturing is realising via plot function changing, etc.
     * 2. Need also additional run plot command after each entry with some arguments from @c Cantor::Expression object, see savePlotCommand()
     *
     * If the package is implemented in the first way, it must have empty plot command template (see savePlotCommand()), then isHavePlotCommand() will return @c true.
     * Otherwise, isHavePlotCommand() will return @c false.
     */
    bool isHavePlotCommand() const;

    /**
     * @brief This function return command for saving image result(s) from expression (if results are existed).
     * @param filenamePrefix Prefix of files with plots. Can be something like @c "/tmp/cantor_octave_2432_plot". Optional parameter.
     * @param plotNumber Currect plot number, should be used for full filename construction. Optional parameter.
     * @param additionalInfo This is additional parameter from backend, which go to @c "%3" template. Optional parameter.
     * @return Command which will save plot to certain file or empty string (see isHavePlotCommand())
     */
    QString savePlotCommand(QString filenamePrefix = QString(), int plotNumber = -1, QString additionalInfo = QString()) const;

    /**
     * Some graphic package can't capture plots correctly, for example, some packages can't test precense of created plot.
     * So, the package handling need some code for testing of plot command precense
     * This method return list of some strings, which should be in plot command.
     * @return List of strings, which should be in plot command or empty list.
     */
    const QStringList& plotCommandPrecentsKeywords() const;

    /**
     * @brief Load graphic packages from XML file.
     *
     * The file should have @c "GraphicPackages" root element with one or more @c "GraphicPackage" XML elements.
     * @code{.xml}
     * <GraphicPackages>
     *   <GraphicPackage>
     *     ..
     *   </GraphicPackage>
     *   ...
     * </GraphicPackages>
     * @endcode
     *
     * Scheme of the @c GraphicPackage element:
     * @code{.xml}
     * <GraphicPackage>
     *   <Id>...</Id>
     *   <Name>...</Name>
     *   <TestPresenceCommand>...</TestPresenceCommand>
     *   <EnableCommand>...</EnableCommand>
     *   <DisableCommand>...</DisableCommand>
     *   <ToFileCommandTemplate>...</ToFileCommandTemplate>
     * </GraphicPackage>
     * @endcode
     */
    static QList<GraphicPackage> loadFromFile(const QString& filename);

    /**
     * Some helper for searching @c package inside @c list of packages.
     */
    static int findById(const GraphicPackage& package, const QList<GraphicPackage>& list);

private:
    GraphicPackage();

    GraphicPackagePrivate* d;
};

}
#endif /* _GRAPHICPACKAGE_H */
