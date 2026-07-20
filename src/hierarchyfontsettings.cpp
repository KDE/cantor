/*
    SPDX-FileCopyrightText: 2026 Nanhao Lv <nanhaolv@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hierarchyfontsettings.h"

#include "settings.h"

void migrateHierarchyFontSettings()
{
    if (Settings::hierarchyFontSettingsMigrated())
        return;

    const auto completeFont = [](QFont font, int pointSize, bool italic, bool bold)
    {
        if (pointSize > 0)
            font.setPointSize(pointSize);
        font.setItalic(italic);
        font.setBold(bold);
        return font;
    };

    Settings::setChapterFontFamily(
        completeFont(Settings::chapterFontFamily(), Settings::chapterFontSize(), Settings::chapterFontItalic(), Settings::chapterFontBold()));
    Settings::setSubchapterFontFamily(
        completeFont(Settings::subchapterFontFamily(), Settings::subchapterFontSize(), Settings::subchapterFontItalic(), Settings::subchapterFontBold()));
    Settings::setSectionFontFamily(
        completeFont(Settings::sectionFontFamily(), Settings::sectionFontSize(), Settings::sectionFontItalic(), Settings::sectionFontBold()));
    Settings::setSubsectionFontFamily(
        completeFont(Settings::subsectionFontFamily(), Settings::subsectionFontSize(), Settings::subsectionFontItalic(), Settings::subsectionFontBold()));
    Settings::setParagraphFontFamily(
        completeFont(Settings::paragraphFontFamily(), Settings::paragraphFontSize(), Settings::paragraphFontItalic(), Settings::paragraphFontBold()));
    Settings::setSubparagraphFontFamily(
        completeFont(Settings::subparagraphFontFamily(), Settings::subparagraphFontSize(), Settings::subparagraphFontItalic(), Settings::subparagraphFontBold()));

    Settings::setHierarchyFontSettingsMigrated(true);
    Settings::self()->save();
}
