
#ifndef WORKSHEET_TEXT_ITEM_H
#define WORKSHEET_TEXT_ITEM_H

class WorksheetTextItem : public QGraphicsTextItem
{
  Q_OBJECT
  public:
    WorksheetTextItem();
    ~WorksheetTextItem();

    void enableHighlighting(bool highlight);

  signals:
    void leftmostValidPositionReached();
    void rightmostValidPositionReached();
    void topmostValidLineReached();
    void bottommostValidLineReached();
    
  protected:
    bool keyPressEvent(QKeyEvent * event);

  private:
    Cantor::Session* session();

  private:
    QSyntaxHighlighter *m_highlighter;
};

#endif // WORKSHEET_TEXT_ITEM_H
