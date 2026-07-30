/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#ifndef WORKSHEETIMAGEITEM_H
#define WORKSHEETIMAGEITEM_H

#include <QPixmap>
#include <QGraphicsObject>

class Worksheet;
class QImage;
class QGraphicsSceneContextMenuEvent;
class QFocusEvent;
class QMenu;
class WorksheetImageResizeHandle;

class WorksheetImageItem : public QGraphicsObject
{
  Q_OBJECT
  public:
    explicit WorksheetImageItem(QGraphicsObject* parent);
    ~WorksheetImageItem() override;

    enum {Type = UserType + 101};

    int type() const override;

    bool imageIsValid();

    virtual qreal setGeometry(qreal x, qreal y, qreal w, bool centered=false);

    qreal height() const;
    qreal width() const;
    QSizeF size();
    void setSize(QSizeF size);
    QSize imageSize();
    void setResizable(bool resizable);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void setPdf(const QUrl &url);
    void setImage(const QImage& img);
    void setImage(const QImage& img, QSize displaySize);
    void setPixmap(const QPixmap& pixmap);
    QPixmap pixmap() const;

    virtual void populateMenu(QMenu* menu, QPointF pos);
    Worksheet* worksheet();

  Q_SIGNALS:
    void sizeChanged();
    void resizeStarted(bool fromTopCorner);
    void resizePreviewChanged(QSizeF size);
    void resizeFinished(QSizeF size);
    void menuCreated(QMenu*, QPointF);

  protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;

  private:
    friend class WorksheetImageResizeHandle;

    void beginResize(int position, const QPointF& scenePos);
    void continueResize(const QPointF& scenePos);
    void endResize();
    void updateResizeHandlePositions();

    QPixmap m_pixmap;
    QSizeF m_size;
    QList<WorksheetImageResizeHandle*> m_resizeHandles;
    QPointF m_resizeStartScenePos;
    QPointF m_resizeStartItemPos;
    QSizeF m_resizeStartSize;
    int m_resizePosition{0};
    bool m_resizable{false};
};

#endif //WORKSHEETIMAGEITEM_H
