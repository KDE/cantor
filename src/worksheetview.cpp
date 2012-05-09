
#include "worksheetview.h"
#include "worksheet.h"

WorksheetView::WorksheetView(Worksheet* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    m_scale = 1;
    m_worksheet = qobject_cast<Worksheet*>(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

WorksheetView::~WorksheetView()
{
}

void WorksheetView::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event);
    updateSceneSize();
}

qreal WorksheetView::scaleFactor()
{
    return m_scale;
}

void WorksheetView::updateSceneSize()
{
    m_worksheet->setViewSize(width() / m_scale, height() / m_scale);
}

void WorksheetView::zoomIn()
{
    m_scale *= 1.1;
    scale(1.1, 1.1);
    updateSceneSize();
}

void WorksheetView::zoomOut()
{
    m_scale /= 1.1;
    scale(1/1.1, 1/1.1);
    updateSceneSize();
}

#include "kdebug.h"

void WorksheetView::mousePressEvent(QMouseEvent* event)
{
    QPointF pos = mapToScene(event->pos());
    kDebug() << "Click at" << pos;
    if (QGraphicsItem* item = scene()->itemAt(pos))
	kDebug() << "item " << item;
}

#include "worksheetview.moc"
