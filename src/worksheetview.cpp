
#include "worksheetview.h"

WorksheetView::WorksheetView(Worksheet* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    m_scale = 1;
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

WorksheetView::~WorksheetView()
{
}

void WorksheetView::resizeEvent(QResizeEvent * event)
{
    updateSceneSize();
}

void WorksheetView::updateSceneSize()
{
    scene()->setViewSize(event.width() / m_scale, event.height() / m_scale);
}

void WorksheetView::zoomIn()
{
    m_scale *= 1.1;
    scale(1.1, 1.1);
    updateSceneSize();
}

void zoomOut()
{
    m_scale /= 1.1
    scale(1/1.1, 1/1.1);
    updateSceneSize();
}

#include "worksheetview.moc"
