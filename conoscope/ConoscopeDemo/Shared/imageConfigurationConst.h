#ifndef IMAGECONFIGURATIONCONST_H
#define IMAGECONFIGURATIONCONST_H

/*

    IMAGE / ACTIVE

    +----------------------------------------+
    |IMAGE                                   |
    |   +--------------------------------+   |
    |   |ACTIVE                          |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    +---+--------------------------------+---+

    TOP / LEFT / RIGHT

    +----------------------------------------+
    |T                                       |
    +---+--------------------------------+---+
    |L  |                                |R  |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    |   |                                |   |
    +---+--------------------------------+---+

 */


#define _PIXEL_DATA_TYPE           qint16
#define _PIXEL_SIZE                sizeof(PIXEL_DATA_TYPE)

#define _IMAGE_HORIZONTAL_OFFSET   0
#define _IMAGE_VERTICAL_OFFSET     0
#define _IMAGE_WIDTH               3584
#define _IMAGE_HEIGHT              2528

#define _IMAGE                     _IMAGE_HORIZONTAL_OFFSET, _IMAGE_VERTICAL_OFFSET, _IMAGE_WIDTH, _IMAGE_HEIGHT

#define _IMAGE_NB_PIXELS           _IMAGE_WIDTH * _IMAGE_HEIGHT
#define _IMAGE_NB_BYTES            _IMAGE_NB_PIXELS * _PIXEL_SIZE

#define _ACTIVE_HORIZONTAL_OFFSET  112
#define _ACTIVE_VERTICAL_OFFSET    32
#define _ACTIVE_WIDTH              3360
#define _ACTIVE_HEIGHT             2496

#define _ACTIVE                    _ACTIVE_HORIZONTAL_OFFSET, _ACTIVE_VERTICAL_OFFSET, _ACTIVE_WIDTH, _ACTIVE_HEIGHT

#define _TOP_HORIZONTAL_OFFSET     0
#define _TOP_VERTICAL_OFFSET       0
#define _TOP_WIDTH                 _IMAGE_WIDTH
#define _TOP_HEIGHT                _ACTIVE_VERTICAL_OFFSET

#define _TOP                       _TOP_HORIZONTAL_OFFSET, _TOP_VERTICAL_OFFSET, _TOP_WIDTH, _TOP_HEIGHT

#define _LEFT_HORIZONTAL_OFFSET    0
#define _LEFT_VERTICAL_OFFSET      _ACTIVE_VERTICAL_OFFSET
#define _LEFT_WIDTH                _ACTIVE_HORIZONTAL_OFFSET
#define _LEFT_HEIGHT               _ACTIVE_HEIGHT

#define _LEFT                      _LEFT_HORIZONTAL_OFFSET, _LEFT_VERTICAL_OFFSET, _LEFT_WIDTH, _LEFT_HEIGHT

#define _RIGHT_HORIZONTAL_OFFSET   _IMAGE_WIDTH - _ACTIVE_HORIZONTAL_OFFSET
#define _RIGHT_VERTICAL_OFFSET     _ACTIVE_VERTICAL_OFFSET
#define _RIGHT_WIDTH               _ACTIVE_HORIZONTAL_OFFSET
#define _RIGHT_HEIGHT              _ACTIVE_HEIGHT

#define _RIGHT                     _RIGHT_HORIZONTAL_OFFSET, _RIGHT_VERTICAL_OFFSET, _RIGHT_WIDTH, _RIGHT_HEIGHT


#endif // IMAGECONFIGURATIONCONST_H
