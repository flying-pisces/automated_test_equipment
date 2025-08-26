#ifndef IMAGE_CONFIGURATION_H
#define IMAGE_CONFIGURATION_H

#include "Types.h"

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

    BIAS
    CORNER: TOP LEFT / TOP RIGTH / BOTTOM LEFT / BOTTOM RIGHT

    +----------------------------------------+
    |                                        |
    |   +--+    +----------------+    +--+   |
    |   |TL|    |                |    |TR|   |
    |   +--+    |                |    +--+   |
    |           |                |           |
    |           |                |           |
    |           |                |           |
    |   +--+    |                |    +--+   |
    |   |BL|    |                |    |BR|   |
    +---+--+- --+----------------+----+--+---+

*/

#define PIXEL_DATA_TYPE           int16
#define PIXEL_SIZE                sizeof(PIXEL_DATA_TYPE)

#define IMAGE_HORIZONTAL_OFFSET   0
#define IMAGE_VERTICAL_OFFSET     0
#define IMAGE_WIDTH               3584
#define IMAGE_HEIGHT              2528

// #define IMAGE                     IMAGE_HORIZONTAL_OFFSET, IMAGE_VERTICAL_OFFSET, IMAGE_WIDTH, IMAGE_HEIGHT

#define IMAGE_NB_PIXELS           IMAGE_WIDTH * IMAGE_HEIGHT
#define IMAGE_NB_BYTES            IMAGE_NB_PIXELS * PIXEL_SIZE

#define ACTIVE_HORIZONTAL_OFFSET  112
#define ACTIVE_VERTICAL_OFFSET    32
#define ACTIVE_WIDTH              3360
#define ACTIVE_HEIGHT             2496

// #define ACTIVE                    ACTIVE_HORIZONTAL_OFFSET, ACTIVE_VERTICAL_OFFSET, ACTIVE_WIDTH, ACTIVE_HEIGHT

#define TOP_HORIZONTAL_OFFSET     0
#define TOP_VERTICAL_OFFSET       0
#define TOP_WIDTH                 IMAGE_WIDTH
#define TOP_HEIGHT                ACTIVE_VERTICAL_OFFSET

// #define TOP                       TOP_HORIZONTAL_OFFSET, TOP_VERTICAL_OFFSET, TOP_WIDTH, TOP_HEIGHT

#define LEFT_HORIZONTAL_OFFSET    0
#define LEFT_VERTICAL_OFFSET      ACTIVE_VERTICAL_OFFSET
#define LEFT_WIDTH                ACTIVE_HORIZONTAL_OFFSET
#define LEFT_HEIGHT               ACTIVE_HEIGHT

// #define LEFT                      LEFT_HORIZONTAL_OFFSET, LEFT_VERTICAL_OFFSET, LEFT_WIDTH, LEFT_HEIGHT

#define RIGHT_HORIZONTAL_OFFSET   IMAGE_WIDTH - ACTIVE_HORIZONTAL_OFFSET
#define RIGHT_VERTICAL_OFFSET     ACTIVE_VERTICAL_OFFSET
#define RIGHT_WIDTH               ACTIVE_HORIZONTAL_OFFSET
#define RIGHT_HEIGHT              ACTIVE_HEIGHT

// #define RIGHT                     RIGHT_HORIZONTAL_OFFSET, RIGHT_VERTICAL_OFFSET, RIGHT_WIDTH, RIGHT_HEIGHT

#define BIAS_CORNER_AREA_WIDTH    32
#define BIAS_CORNER_AREA_HEIGHT   70

#define BIAS_CORNER_SIZE          (BIAS_CORNER_AREA_WIDTH * BIAS_CORNER_AREA_HEIGHT)

#define ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET  492
#define ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET    ACTIVE_VERTICAL_OFFSET
#define ACTIVE_CENTRAL_AREA_WIDTH              IMAGE_WIDTH - 2 * ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET
#define ACTIVE_CENTRAL_AREA_HEIGHT             ACTIVE_HEIGHT

class ImageConfiguration
{
public:
    int image_horizontal_offset;
    int image_vertical_offset;
    int image_width;
    int image_height;

    int image_nb_pixels;
    int image_nb_bytes;

    int active_horizontal_offset;
    int active_vertical_offset;
    int active_width;
    int active_height;

    int top_horizontal_offset;
    int top_vertical_offset;
    int top_width;
    int top_height;

    int left_horizontal_offset;
    int left_vertical_offset;
    int left_width;
    int left_height;

    int right_horizontal_offset;
    int right_vertical_offset;
    int right_width;
    int right_height;

    int bias_corner_area_width;
    int bias_corner_area_height;

    int bias_corner_size;

    int active_central_area_horizontal_offset;
    int active_central_area_vertical_offset;
    int active_central_area_width;
    int active_central_area_height;

    ImageConfiguration()
    {
        // default values
        image_horizontal_offset                = IMAGE_HORIZONTAL_OFFSET;
        image_vertical_offset                  = IMAGE_VERTICAL_OFFSET;
        image_width                            = IMAGE_WIDTH;
        image_height                           = IMAGE_HEIGHT;

        active_horizontal_offset               = ACTIVE_HORIZONTAL_OFFSET;
        active_vertical_offset                 = ACTIVE_VERTICAL_OFFSET;
        active_width                           = ACTIVE_WIDTH;
        active_height                          = ACTIVE_HEIGHT;

        bias_corner_area_width                 = BIAS_CORNER_AREA_WIDTH;
        bias_corner_area_height                = BIAS_CORNER_AREA_HEIGHT;

        active_central_area_horizontal_offset  = ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET;
        active_central_area_vertical_offset    = ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET;

        UpdateSettings();
    }

    ImageConfiguration(const ImageConfiguration& image)
    {
        // default values
        image_horizontal_offset                = image.image_horizontal_offset;
        image_vertical_offset                  = image.image_vertical_offset;
        image_width                            = image.image_width;
        image_height                           = image.image_height;

        active_horizontal_offset               = image.active_horizontal_offset;
        active_vertical_offset                 = image.active_vertical_offset;
        active_width                           = image.active_width;
        active_height                          = image.active_height;

        bias_corner_area_width                 = image.bias_corner_area_width;
        bias_corner_area_height                = image.bias_corner_area_height;

        active_central_area_horizontal_offset  = image.active_central_area_horizontal_offset;
        active_central_area_vertical_offset    = image.active_central_area_vertical_offset;

        UpdateSettings();
    }

    void UpdateSettings()
    {
        // update the settings depending on other settings
        image_nb_pixels             = image_width * image_height;
        image_nb_bytes              = image_nb_pixels * PIXEL_SIZE;

        top_horizontal_offset       = 0;
        top_vertical_offset         = 0;
        top_width                   = image_width;
        top_height                  = active_vertical_offset;

        left_horizontal_offset      = 0;
        left_vertical_offset        = active_vertical_offset;
        left_width                  = active_horizontal_offset;
        left_height                 = active_height;

        right_horizontal_offset     = image_width - active_horizontal_offset;
        right_vertical_offset       = active_vertical_offset;
        right_width                 = active_horizontal_offset;
        right_height                = active_height;

        bias_corner_size            = (bias_corner_area_width * bias_corner_area_height);

        active_central_area_width   = image_width - 2 * active_central_area_horizontal_offset;
        active_central_area_height  = active_height;
    }
};

#endif // IMAGE_CONFIGURATION_H
