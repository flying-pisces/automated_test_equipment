#ifndef CLineItem_H
#define CLineItem_H

#include <QGraphicsPathItem>
#include <QPen>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>


class QFocusEvent;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;


class CLineItem : public  QGraphicsPathItem
{

public:

  enum eTypeOfLine {
      t_Horizontal    = 0x0,
      t_Vertical      = 0x1,
  };

  CLineItem(QGraphicsItem *parent = 0);
  ~CLineItem () ;


  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  void SetTypeOfLine (CLineItem::eTypeOfLine mNewType)  ;
  QPointF  GetCursorPosition () ;
  bool  isChanged () ;

signals:
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    //void focusOutEvent(QFocusEvent *event) override;
    //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    eTypeOfLine mTypeOfLine ;
    QPointF CurrentPosition ;
    bool  blnPositionChanged ;
};

#endif // CLineItem_H
