#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include <QGraphicsView>

class Worksheet;

class WorksheetView : public QGraphicsView
{
  Q_OBJECT
  public:
    WorksheetView(Worksheet* scene, QWidget* parent);
    ~WorksheetView();
		    
    void resizeEvent(QResizeEvent* event);

    qreal scaleFactor();

    void mousePressEvent(QMouseEvent* event);
		    
  public slots:
    void zoomIn();
    void zoomOut();
  private:
    void updateSceneSize();
  private:
    qreal m_scale;
    Worksheet* m_worksheet;
};

#endif //WORKSHEETVIEW_H
