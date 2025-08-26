#include "qEldViewer.h"

QEldViewer::QEldViewer(QWidget *parent) : QGraphicsView(parent)
//-------------------------------------------------------------
{

  // Create the scene
  mScene = new QGraphicsScene();


  // Allow mouse tracking even if no button is pressed
  this->setMouseTracking(true);

  // Add the scene to the QGraphicsView
  this->setScene(mScene);

  // Update all the view port when needed, otherwise, the drawInViewPort may experience trouble
  this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Add the default pixmap at startup
  mPixmapItem = mScene->addPixmap(mCurrentPixmap);

  // Set default zoom factors
  zoomFactor=DEFAULT_ZOOM_FACTOR;
  zoomCtrlFactor=DEFAULT_ZOOM_CTRL_FACTOR;

  mQVRawData.clear(); ;
  mXSizeRaw = 0 ;
  mYSizeRaw = 0 ;
  blnDisplayFalseColor = false ;

  mLineItem = new CLineItem () ;
  mLineItem->setZValue(1);
  mLineItem->setCursor(Qt::PointingHandCursor);
  mLineItem->setVisible(false);
  mScene->addItem(mLineItem);

  mWhiteLevel = 4095;
}

//-------------------------------------------------------------
QEldViewer::~QEldViewer()
//-----------------------
{

  delete mPixmapItem;
  delete mScene;
}


float QEldViewer::fCurrentResolution (short sImageXSize,short sImageYSize) {
//--------------------------------------------------------------------------

  float fXResolution = (float)sImageXSize/(float)(this->width()) ;
  float fYResolution = (float)sImageYSize/(float)(this->height()) ;

  if (fXResolution > fYResolution)  return (fXResolution) ;
  else                              return (fYResolution) ;
}




// Adjusts the image size to the resolution of the displayed window
void QEldViewer::UpdateImage() {
//------------------------------

   // SubSampling if needed
   if (fCurrentResolution (mXSizeRaw,mYSizeRaw) > 2.0) {

     mQVSubTemp = DataSubSampling (mQVRawData,mXSizeRaw,mYSizeRaw) ;

     while (fCurrentResolution (mXSizeSubSampling,mYSizeSubSampling) > 2.0f) {
      mQVSubTemp = DataSubSampling (mQVSubTemp,mXSizeSubSampling,mYSizeSubSampling) ;
     }
   }
   else {
       mQVSubSamplingData = mQVRawData ;
       mXSizeSubSampling = mXSizeRaw ;
       mYSizeSubSampling = mYSizeRaw ;
   }
   CreateBitmap() ;

   MyImage = QImage((unsigned char*)&(qBMPData.data()[0]),mXSizeSubSampling,mYSizeSubSampling,QImage::Format_RGBX8888);

   qDebug ("Image Size %dx%d" ,MyImage.width(),MyImage.height()) ;

   //bool blnResult =  MyImage.load("d:/qt.jpg") ;
   //qDebug () << blnResult ;

   // Update the pixmap in the scene
   mCurrentPixmap=QPixmap::fromImage(MyImage.mirrored(false,false));

   mPixmapItem->setPixmap(mCurrentPixmap);
   // Resize the scene (needed is the new image is smaller)
   mScene->setSceneRect(QRect (QPoint(0,0),MyImage.size()));

   fitImage () ;
}


void QEldViewer::SetRawData(QVector<uint16_t>& Source, short sXSize, short sYSize)
//-------------------------------------------------------------------------------
{
  mQVRawData = Source;
  mXSizeRaw = sXSize;
  mYSizeRaw = sYSize;
  UpdateImage ();
}

void QEldViewer::SetWhiteLevel(int whiteLevel)
{
    mWhiteLevel = whiteLevel;
}

void QEldViewer::CreateBitmap()
//-----------------------------
{

    unsigned char   *ucBmpData  ;
    long            lIndex ;
    long            lLength ;
    unsigned char   ucCurrentValue ;
    uint16_t        intCurrentValue ;

    #define MAX_DATA_VALUE  4095

    lLength = mXSizeSubSampling* mYSizeSubSampling ;

    if ((4* lLength) != mQVSubSamplingData.length()) {
        qBMPData.resize (4* lLength) ;
    }

    if (blnDisplayFalseColor == false)
    {
        ucBmpData = (unsigned char*)&(qBMPData.data()[0]);

        int pixIndex = 0;

#pragma omp parallel num_threads(4)
        for (lIndex = 0 ; lIndex < lLength ; lIndex ++)
        {
            // ucCurrentValue = ((long)mQVSubSamplingData.at(lIndex)*255)/(MAX_DATA_VALUE) ;
            // ucCurrentValue = ((long)mQVSubSamplingData.at(lIndex)*255)/(mWhiteLevel);
            ucCurrentValue = (unsigned char)(mQVSubSamplingData.at(lIndex) >> 4);

            ucBmpData[pixIndex++] = ucCurrentValue;
            ucBmpData[pixIndex++] = ucCurrentValue;
            ucBmpData[pixIndex++] = ucCurrentValue;
            ucBmpData[pixIndex++] = 0;
        }
    }
    else
    {
        CreateCurrentPalette () ;
        /* ptrBmpData = (uint32_t *)&(qBMPData.data()[0]);
        for (lIndex = 0 ; lIndex < lLength ; lIndex ++) {
          intCurrentValue = mQVSubSamplingData.data()[lIndex] ;
          ptrBmpData [lIndex] = mQVCustomPalette.data()[intCurrentValue] ;
        }*/
        ucBmpData = (unsigned char*)&(qBMPData.data()[0]);

#pragma omp parallel num_threads(4)
        for (lIndex = 0 ; lIndex < lLength ; lIndex ++)
        {
            intCurrentValue = mQVSubSamplingData.data()[lIndex] ;
            ucBmpData [4*lIndex+0] = qRed (mQVCustomPalette.data()[intCurrentValue]) ;
            ucBmpData [4*lIndex+1] = qGreen (mQVCustomPalette.data()[intCurrentValue]) ;
            ucBmpData [4*lIndex+2] = qBlue (mQVCustomPalette.data()[intCurrentValue]) ;
            ucBmpData [4*lIndex+3] = 0 ;
        }
    }
}

void QEldViewer::CalculateDisplayMinMax()
//---------------------------------------
{
    long  lIndex ;
    long  lLength ;
    int   intValue ;

    lLength = mXSizeSubSampling* mYSizeSubSampling ;
    mMinValueDisplayed = 4096 ;
    mMaxValueDisplayed = 0 ;

    for (lIndex = 0 ; lIndex < lLength ; lIndex ++) {
        intValue = mQVSubSamplingData.at(lIndex) ;
        if (intValue < mMinValueDisplayed)
            mMinValueDisplayed = intValue ;

        if (intValue > mMaxValueDisplayed)
            mMaxValueDisplayed = intValue ;
    }
}


QRgb QEldViewer::QRGB_RainBowPalette (int  intValue1024) {
//--------------------------------------------------------


  if (intValue1024 < 256)
    return (qRgba(0,intValue1024, 255,0)) ;
  else
    if (intValue1024 < 512)
      return (qRgba(0,255,511 -intValue1024,0)) ;
    else
      if (intValue1024 < 768)
        return (qRgba(intValue1024-512,255,0,0)) ;
      else
        if (intValue1024 < 1024)
          return (qRgba(255,1023-intValue1024,0,0)) ;
        else
          return (qRgba(255,0,0,0)) ;
}

void QEldViewer::CreateCurrentPalette() {
//-----------------------------------------

  int   intIndex ;
  QRgb  oleColorTemp ;
  CalculateDisplayMinMax () ;

  mQVCustomPalette.resize( mMaxValueDisplayed);

  float fTemp = 1024.0f / (float)(mMaxValueDisplayed - mMinValueDisplayed) ;
  oleColorTemp = QRGB_RainBowPalette(0) ;

  for (intIndex = 0; intIndex < mMinValueDisplayed ; intIndex ++) {
    mQVCustomPalette.data()[intIndex] = oleColorTemp ;
  }

  for (intIndex = mMinValueDisplayed; intIndex <= mMaxValueDisplayed ; intIndex ++){
    mQVCustomPalette.data()[intIndex] = QRGB_RainBowPalette((float)(intIndex - mMinValueDisplayed) * fTemp) ;
  }

}


void QEldViewer::fitImage()
//-------------------------
{
    // Get current scroll bar policy
    Qt::ScrollBarPolicy	currentHorizontalPolicy = horizontalScrollBarPolicy();
    Qt::ScrollBarPolicy	currentverticalPolicy = verticalScrollBarPolicy();

    // Disable scroll bar to avoid a margin around the image
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Fit the scene in the QGraphicsView
    this->fitInView(mScene->sceneRect(), Qt::KeepAspectRatio);

    // Restaure scroll bar policy
    setHorizontalScrollBarPolicy(currentHorizontalPolicy);
    setVerticalScrollBarPolicy(currentverticalPolicy);
}

void QEldViewer::wheelEvent(QWheelEvent *event)
//---------------------------------------------
{
    return;

    if (event->orientation() == Qt::Vertical)
    {
        double factor = (event->modifiers() & Qt::ControlModifier) ? zoomCtrlFactor : zoomFactor;
        if(event->delta() > 0)
            // Zoom in
            scale(factor, factor);
        else
            // Zooming out
            scale(1.0 / factor, 1.0 / factor);

       // When zooming, the view stay centered over the mouse
       sceneMousePos = this->mapToScene(event->pos());
       this->centerOn(sceneMousePos);
        // The event is processed
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void QEldViewer::mousePressEvent(QMouseEvent *event)
//--------------------------------------------------
{
    if(event->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction *QA_DisplayHorizontal = menu.addAction("Display Horizontal Cursor");
        QAction *QA_DisplayVertical = menu.addAction("Display Vertical Cursor");
        QAction *QA_HideCursor = menu.addAction("Hide Cursor");

        connect(QA_DisplayHorizontal, SIGNAL(triggered(bool)),this,SLOT(DisplayHorizontalCursor()));
        connect(QA_DisplayVertical, SIGNAL(triggered(bool)),this,SLOT(DisplayVerticalCursor()));
        connect(QA_HideCursor, SIGNAL(triggered(bool)),this,SLOT(HideCursor()));
        menu.exec(event->globalPos());
        event->accept();
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void QEldViewer::mouseMoveEvent(QMouseEvent *event)
//-------------------------------------------------
{
  float fSubSamplingRatio ;
  int   intXRaw ;
  int   intYRaw ;
  int   intValue ;

  fSubSamplingRatio = mXSizeRaw / mXSizeSubSampling ;

  sceneMousePos = this->mapToScene(event->pos());

  intXRaw = (int)(sceneMousePos.x()*fSubSamplingRatio) ;
  intYRaw = (int)(sceneMousePos.y()*fSubSamplingRatio) ;

  if ( (mXSizeRaw > 0) && (mXSizeRaw > 0)) {
      if ((intXRaw >= 0) && (intYRaw >= 0)) {
        intValue = mQVRawData.data()[intXRaw+intYRaw*mXSizeRaw] ;
        this->setToolTip(" x = " + QString::number (intXRaw) +
                         " , y = " + QString::number (intYRaw) + " , V = " + QString::number (intValue));



        MouseMoveOnImage(intXRaw,intYRaw,intValue);
        if ((mLineItem->isVisible()) &&(mLineItem->isChanged()) ) {
            CursorChange (mLineItem->GetCursorPosition().x()*fSubSamplingRatio,mLineItem->GetCursorPosition().y()*fSubSamplingRatio) ;
        }
      }
  }
  else {
    this->setToolTip("");
  }

  QGraphicsView::mouseMoveEvent(event);

}

void QEldViewer::DisplayVerticalCursor()
//--------------------------------------
{
    mLineItem->SetTypeOfLine(CLineItem::t_Vertical);
    mLineItem->setVisible(true);
    mLineItem->setPos(scene()->width()/2,0);
}

void QEldViewer::DisplayHorizontalCursor()
//----------------------------------------
{
    mLineItem->SetTypeOfLine(CLineItem::t_Horizontal);
    mLineItem->setVisible(true);
    mLineItem->setPos(0,scene()->height()/2);
}

void QEldViewer::HideCursor()
//---------------------------
{
     mLineItem->setVisible(false);
}

QVector<uint16_t>  QEldViewer::DataSubSampling(
        QVector<uint16_t>& Source,
        short sImgXSize,
        short sImgYSize)
//-------------------------------------------------------------------------------------------------------
{
    uint16_t shIndexCol, shIndexRow, shSum;
    uint16_t *ptrStart;

    uint16_t * ptrResult;
    uint16_t * ptrSource;

    if (sImgXSize % 2 == 0)  mXSizeSubSampling = sImgXSize / 2;
    else                     mXSizeSubSampling = (sImgXSize + 1) / 2;

    if (sImgYSize % 2 == 0)  mYSizeSubSampling = sImgYSize / 2;
    else                     mYSizeSubSampling = (sImgYSize + 1) / 2;

    mQVSubSamplingData.resize(mXSizeSubSampling*mYSizeSubSampling);

    ptrSource = &Source.data()[0] ;
    ptrResult = &mQVSubSamplingData.data()[0] ;

    //---- Compute
    for (shIndexRow = 0; shIndexRow < mYSizeSubSampling; shIndexRow ++)
    {
        ptrStart = ptrSource;

#pragma omp parallel num_threads(4)
        for (shIndexCol = 0; shIndexCol < mXSizeSubSampling; shIndexCol ++)
        {
            if ((shIndexRow != mYSizeSubSampling - 1) && (shIndexCol != mXSizeSubSampling - 1))
            {
                shSum = ( *ptrSource
                         + *(ptrSource + 1)
                         + *(ptrSource + sImgXSize)
                         + *(ptrSource + sImgXSize + 1)) / 4;
                *ptrResult = shSum;
            }
            else if ((shIndexRow != mYSizeSubSampling - 1) && (shIndexCol == mXSizeSubSampling - 1))
            {
                if (sImgXSize % 2 == 1)
                {
                    shSum = ( *ptrSource
                              + *(ptrSource + sImgXSize)) / 2;
                    *ptrResult = shSum;
                }
                else
                {
                    shSum = ( *ptrSource
                              + *(ptrSource + 1)
                              + *(ptrSource + sImgXSize)
                              + *(ptrSource + sImgXSize + 1)) / 4;
                    *ptrResult = shSum;
                }
            }
            else if ((shIndexRow == mYSizeSubSampling - 1) && (shIndexCol != mXSizeSubSampling - 1))
            {
                if (sImgYSize % 2 == 1)
                {
                    shSum = ( *ptrSource
                              + *(ptrSource + 1)) / 2;
                    *ptrResult = shSum;
                }
                else
                {
                    shSum = ( *ptrSource
                              + *(ptrSource + 1)
                              + *(ptrSource + sImgXSize)
                              + *(ptrSource + sImgXSize + 1)) / 4;
                    *ptrResult = shSum;
                }
            }
            else
            {
                if ((sImgXSize % 2 == 1) && (sImgYSize % 2 == 1))
                {
                  *ptrResult = *ptrSource;
                }
                else if ((sImgXSize % 2 == 1) && (sImgYSize % 2 != 1))
                {
                  shSum = ( *ptrSource
                            + *(ptrSource + sImgXSize)) / 2;
                  *ptrResult = shSum;
                }
                else if ((sImgXSize % 2 != 1) && (sImgYSize % 2 == 1))
                {
                  shSum = ( *ptrSource
                            + *(ptrSource + 1)) / 2;
                  *ptrResult = shSum;
                }
                else
                {
                  shSum = ( *ptrSource
                            + *(ptrSource + 1)
                            + *(ptrSource + sImgXSize)
                            + *(ptrSource + sImgXSize + 1)) / 4;
                  *ptrResult = shSum;
                }
            }
            ptrResult ++;
            ptrSource = ptrSource + 2;
        }
        ptrSource = ptrStart + 2 * sImgXSize;
    }

    return (mQVSubSamplingData) ;
}

void QEldViewer::SetDisplayFalseColor(bool blnValue)
//-------------------------------------------------
{
  blnDisplayFalseColor = blnValue ;
}


