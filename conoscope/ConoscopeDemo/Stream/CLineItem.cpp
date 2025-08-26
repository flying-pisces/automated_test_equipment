#include "CLineItem.h"


CLineItem::CLineItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
//----------------------------------------------------------------------------
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  mTypeOfLine = CLineItem::t_Horizontal ;
  blnPositionChanged = true ;

  mXLeft = 0;
  mXRight = 0;
  mYTop = 0;
  mYBot = 0;
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

    qreal penWidth = 3;
    QColor penColor;

    switch ( mTypeOfLine )
    {
    case CLineItem::t_Horizontal:
    case CLineItem::t_Vertical:
        penWidth = 1;
        // penColor = Qt::green;
        penColor = QColor(134, 235, 52);
        break;

    case CLineItem::t_Roi:
        penWidth = 1;
        penColor = Qt::red;
        break;
    }

#ifdef REMOVED
    if(isSelected())
    {
        penColor = Qt::blue;
    }
    else
    {
        penColor = Qt::red;
    }
#endif

    QPen myPen(penColor, penWidth, Qt::SolidLine);
    painter->setPen(myPen);

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

    case CLineItem::t_Roi:
        {
        qreal sceneH = this->scene()->height();
        qreal sceneW = this->scene()->width();

        p.moveTo(sceneW * mXLeft,  sceneH * mYTop);
        p.lineTo(sceneW * mXRight, sceneH * mYTop); // horizontal top
        p.lineTo(sceneW * mXRight, sceneH * mYBot); // vertical   right
        p.lineTo(sceneW * mXLeft,  sceneH * mYBot); // horizontal bottom
        p.lineTo(sceneW * mXLeft,  sceneH * mYTop); // horizontal bottom
        }
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

void CLineItem::SetRoi(float XLeft, float XRight, float YTop, float YBot)
{
    mXLeft  = XLeft;
    mXRight = XRight;
    mYTop   = YTop;
    mYBot   = YBot;
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


