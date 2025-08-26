#include "CLineItem.h"


CLineItem::CLineItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
//----------------------------------------------------------------------------
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  mTypeOfLine = CLineItem::t_Horizontal ;
  blnPositionChanged = true ;
}

CLineItem::~CLineItem()
//--------------------
{

}

void CLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
//---------------------------------------------------------------------------------------------
{
  QPainterPath p;
  Q_UNUSED(option)
  Q_UNUSED(widget)

  painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);




  if (isSelected()) {
    QPen myPen(Qt::blue, 3, Qt::SolidLine);
    painter->setPen(myPen);
          //painter->setBrush(Qt::yellow);
  }
  else {
    QPen myPen(Qt::white, 3, Qt::SolidLine);
    painter->setPen(myPen);
          //painter->setBrush(Qt::green);
  }

  switch ( mTypeOfLine )
  {
    case CLineItem::t_Horizontal:
      // Horizontal
      p.moveTo(0,0);
      p.lineTo(this->scene()->width(),0);
      break;

    case CLineItem::t_Vertical:
      // Vertival
      p.moveTo(0,0);
      p.lineTo(0,this->scene()->height());
      break;

    default:
      break ;
  }

  setPath(p);

  painter->drawPath(path());
}

void CLineItem::SetTypeOfLine(CLineItem::eTypeOfLine mNewType)
//------------------------------------------------------------
{
  mTypeOfLine = mNewType ;
}

QPointF CLineItem::GetCursorPosition()
//-----------------------------------
{
    return (CurrentPosition) ;
}

bool CLineItem::isChanged()
//---------------------------
{
  if (blnPositionChanged) {
    blnPositionChanged = false ;
    return (true) ;
  }
  else {
    return (false) ;
  }
}

QVariant CLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
//----------------------------------------------------------------------------
{
  //Q_UNUSED(change);

  if (change == ItemPositionChange) {
        CurrentPosition = value.toPointF();
        qreal xV = 0;
        qreal yV = 0;

        switch ( mTypeOfLine )
        {
          case CLineItem::t_Horizontal:
            yV = round(CurrentPosition.y());
            if (yV < 0) yV = 0 ;
            if (yV > this->scene()->height()) xV = this->scene()->height() ;
            blnPositionChanged = true ;
            break;

          case CLineItem::t_Vertical:
            xV = round(CurrentPosition.x());
            if (xV < 0) xV = 0 ;
            if (xV > this->scene()->width()) xV = this->scene()->width() ;
            blnPositionChanged = true ;
            break;

          default:
            break ;
        }

        CurrentPosition.setX(xV) ;
        CurrentPosition.setY(yV) ;

        return (CurrentPosition) ;
    }
    else
        return QGraphicsItem::itemChange(change, value);
}


