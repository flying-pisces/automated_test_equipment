#ifndef QENHANCEDGRAPHICSVIEW_H
#define QENHANCEDGRAPHICSVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QtMath>
#include <QContextMenuEvent>
#include <QMenu>
#include <QGraphicsItem>
#include <QDebug>
#include <QGraphicsEffect>
#include "CLineItem.h"

// Default zoom factors
#define         DEFAULT_ZOOM_FACTOR             1.15
#define         DEFAULT_ZOOM_CTRL_FACTOR        1.01

class QEldViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit QEldViewer(QWidget *parent = nullptr);
    ~QEldViewer();


  void SetRawData (QVector<uint16_t> &Source, short sXSize, short sYSize , bool cleanCursor = false) ;
  void SetDisplayFalseColor (bool blnValue) ;
  void SetWhiteLevel(int whiteLevel);
  void SetAeRoi(int Width, int Height, int X, int Y);

protected:
    void wheelEvent(QWheelEvent *event);
    void setScale();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event) ;

signals:
    void MouseMoveOnImage(int Xpos,int Ypos,int Value);
    void CursorChange (int intXPos,int intYPos) ;

private slots:
    void DisplayCenter();
    void DisplayAeROI();
    void HideCursor () ;

private:

    QVector<uint16_t> DataSubSampling(QVector<uint16_t> &Source,
                                      short shNbCols,
                                      short shNbRows);

    void CreateBitmap();
    void fitImage();
    float fCurrentResolution(short sImageXSize, short sImageYSize);
    void UpdateImage();
    QRgb QRGB_RainBowPalette(int intValue1024);
    void CalculateDisplayMinMax();
    void CreateCurrentPalette();
//----------------------------------------------------------------------------------------------

    QPointF sceneMousePos;
    // Contains Image

    // Scene where the image is drawn
    QGraphicsScene*         mScene;

    // Pixmap item containing the image
    QGraphicsPixmapItem*    mPixmapItem;

    // Size of the current image
    QSize                   mImageSize;

    // Current pixmap
    QPixmap                 mCurrentPixmap;

    QVector<uint16_t>       mQVRawData ;

    uint16_t                mXSizeRaw ;
    uint16_t                mYSizeRaw ;

    QVector<uint16_t>       mQVSubSamplingData ;
    QVector<uint16_t>       mQVSubTemp ;
    uint16_t                mXSizeSubSampling ;
    uint16_t                mYSizeSubSampling ;
    QByteArray              qBMPData ;

    double                  zoomFactor;

    uint16_t                mMinValueDisplayed ;
    uint16_t                mMaxValueDisplayed ;
    QVector<QRgb>           mQVCustomPalette ;

    // Zoom factor when the CTRL key is pressed
    double                  zoomCtrlFactor;
    bool                    blnDisplayFalseColor ;


    QImage  MyImage  ;

    QVector<CLineItem*>     mLineList;
    int mWhiteLevel;

    int mAEMeasAreaWidth;
    int mAEMeasAreaHeight;
    int mAEMeasAreaX;
    int mAEMeasAreaY;

    int scaleWheelCount; // count the number of times the mouse wheel turned

};

#endif // QENHANCEDGRAPHICSVIEW_H
