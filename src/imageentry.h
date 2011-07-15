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
    Copyright (C) 2011 martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef _IMAGEENTRY_H
#define _IMAGEENTRY_H

#include "worksheetentry.h"
#include "imagesettingsdialog.h"
#include <config-cantor.h>

#include <QObject>
#include <QFileSystemWatcher>

class Worksheet;
class WorksheetEntry;

class ImageEntry : public WorksheetEntry
{
  Q_OBJECT
  public:
    ImageEntry(QTextCursor position, Worksheet* parent);
    ~ImageEntry();

    enum {Type = 4};

    int type();

    bool isEmpty();
    
    QTextCursor closestValidCursor(const QTextCursor& cursor);
    QTextCursor firstValidCursorPosition();
    QTextCursor lastValidCursorPosition();
    bool isValidCursor(const QTextCursor& cursor);

    // Handlers for the worksheet input events affecting worksheetentries
    bool worksheetContextMenuEvent(QContextMenuEvent* event, const QTextCursor& cursor);

    bool acceptRichText();
    bool acceptsDrop(const QTextCursor& cursor);

    void setContent(const QString& content);
    void setContent(const QDomElement& content, const KZip& file);

    QDomElement toXml(QDomDocument& doc, KZip* archive);
    QString toPlain(QString& commandSep, QString& commentStartingSeq, QString& commentEndingSeq);

    void interruptEvaluation();
    bool worksheetKeyPressEvent(QKeyEvent* event, const QTextCursor& cursor);
    
    bool evaluate(bool current);

  public slots:
    void update();

    void startConfigDialog();
    void setImageData(const QString& path, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting);

  private:
#ifdef LIBSPECTRE_FOUND
    QTextImageFormat renderEps(const ImageSize& imageSize);
#endif
    void calculateImageSize(int imgWidth, int imgHeight, const ImageSize& imageSize, double& newWidth, double& newHeight);
    QString makeLatexSizeString(const ImageSize&);

  private:
    QString m_imagePath;
    ImageSize m_displaySize;
    ImageSize m_printSize;
    bool m_useDisplaySizeForPrinting;
    QFileSystemWatcher m_imageWatcher;
    ImageSettingsDialog* m_settingsDialog;

};

#endif /* _IMAGEENTRY_H */
