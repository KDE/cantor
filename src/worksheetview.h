#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

class WorksheetView : public QGraphicsView
{
  Q_OBJECT
  public:
    WorksheetView(Worksheet* scene, QWidget* parent);
    ~WorksheetView();
		    
    void resizeEvent(QResizeEvent* event);

    qreal scale();
		    
  public slots:
    void zoomIn();
    void zoomOut();
  private:
    void updateSceneSize();
  private:
    qreal m_scale;
};

#endif WORKSHEETVIEW_H
