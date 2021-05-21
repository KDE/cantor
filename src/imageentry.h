/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include "worksheetentry.h"
#include "imagesettingsdialog.h"

#include <QString>

class Worksheet;
class ActionBar;
class WorksheetImageItem;
class QFileSystemWatcher;

class ImageEntry : public WorksheetEntry
{
  Q_OBJECT

  public:
    explicit ImageEntry(Worksheet* worksheet);
    ~ImageEntry() override = default;

    enum {Type = UserType + 4};
    int type() const override;

    bool isEmpty() override;
    bool acceptRichText() override;
    void setContent(const QString& content) override;
    void setContent(const QDomElement& content, const KZip& file) override;
    void setContentFromJupyter(const QJsonObject & cell) override;
    QDomElement toXml(QDomDocument& doc, KZip* archive) override;
    QJsonValue toJupyterJson() override;
    QString toPlain(const QString& commandSep, const QString& commentStartingSeq, const QString& commentEndingSeq) override;

    QSizeF imageSize(const ImageSize& imgSize);

    void interruptEvaluation() override;

    void layOutForWidth(qreal entry_zone_x, qreal w, bool force = false) override;

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) override;
    void updateEntry() override;

    void populateMenu(QMenu* menu, QPointF pos) override;
    void startConfigDialog();
    void setImageData(const QString& path, const ImageSize& displaySize,
                      const ImageSize& printSize, bool useDisplaySizeForPrinting);

  protected:
    bool wantToEvaluate() override;
    bool wantFocus() override;
    qreal height();

    QString latexSizeString(const ImageSize& imgSize);
    void addActionsToBar(ActionBar* actionBar) override;

  private:
    QString m_imagePath;
    QString m_fileName;
    ImageSize m_displaySize;
    ImageSize m_printSize;
    bool m_useDisplaySizeForPrinting;
    WorksheetImageItem* m_imageItem;
    WorksheetTextItem* m_textItem;
    QFileSystemWatcher* m_imageWatcher;
};

#endif /* IMAGEENTRY_H */
