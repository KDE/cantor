
#ifndef WORKSHEET_TEXT_ITEM_H
#define WORKSHEET_TEXT_ITEM_H

class WorksheetTextItem : public QGraphicsTextItem
{
  Q_OBJECT
  public:
    WorksheetTextItem(QGraphicsItem* parent);
    ~WorksheetTextItem();

  signals:
    void leftmostValidPositionReached();
    void rightmostValidPositionReached();
    void topmostValidLineReached();
    void bottommostValidLineReached();
    void receivedFocus(QTextDocument*);
    
  protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);

  private:
    Cantor::Session* session();
};

#endif // WORKSHEET_TEXT_ITEM_H
