#ifndef IMAGE_CONFIGURATION_H
#define IMAGE_CONFIGURATION_H

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

//#define PIXEL_DATA_TYPE_           uint16
#define PIXEL_DATA_TYPE           unsigned short
#define PIXEL_SIZE                (int)sizeof(PIXEL_DATA_TYPE)

#define IMAGE_HORIZONTAL_OFFSET_x   0
#define IMAGE_VERTICAL_OFFSET_x     0
#define IMAGE_WIDTH_x               3584
#define IMAGE_HEIGHT_x              2528

// #define IMAGE                     IMAGE_HORIZONTAL_OFFSET, IMAGE_VERTICAL_OFFSET, IMAGE_WIDTH, IMAGE_HEIGHT

#define IMAGE_NB_PIXELS_x           IMAGE_WIDTH_x * IMAGE_HEIGHT_x
#define IMAGE_NB_BYTES_x            IMAGE_NB_PIXELS_x * PIXEL_SIZE

#define ACTIVE_HORIZONTAL_OFFSET_x  112
#define ACTIVE_VERTICAL_OFFSET_x    32
#define ACTIVE_WIDTH_x              3360
#define ACTIVE_HEIGHT_x             2496

// #define ACTIVE                    ACTIVE_HORIZONTAL_OFFSET, ACTIVE_VERTICAL_OFFSET, ACTIVE_WIDTH, ACTIVE_HEIGHT

#define TOP_HORIZONTAL_OFFSET_x     0
#define TOP_VERTICAL_OFFSET_x       0
#define TOP_WIDTH_x                 IMAGE_WIDTH_x
#define TOP_HEIGHT_x                ACTIVE_VERTICAL_OFFSET_x

// #define TOP                       TOP_HORIZONTAL_OFFSET, TOP_VERTICAL_OFFSET, TOP_WIDTH, TOP_HEIGHT

#define LEFT_HORIZONTAL_OFFSET_x    0
#define LEFT_VERTICAL_OFFSET_x      ACTIVE_VERTICAL_OFFSET_x
#define LEFT_WIDTH_x                ACTIVE_HORIZONTAL_OFFSET_x
#define LEFT_HEIGHT_x               ACTIVE_HEIGHT_x

// #define LEFT                      LEFT_HORIZONTAL_OFFSET, LEFT_VERTICAL_OFFSET, LEFT_WIDTH, LEFT_HEIGHT

#define RIGHT_HORIZONTAL_OFFSET_x   IMAGE_WIDTH_x - ACTIVE_HORIZONTAL_OFFSET_x
#define RIGHT_VERTICAL_OFFSET_x     ACTIVE_VERTICAL_OFFSET_x
#define RIGHT_WIDTH_x               ACTIVE_HORIZONTAL_OFFSET_x
#define RIGHT_HEIGHT_x              ACTIVE_HEIGHT_x

// #define RIGHT                     RIGHT_HORIZONTAL_OFFSET, RIGHT_VERTICAL_OFFSET, RIGHT_WIDTH, RIGHT_HEIGHT

#define BIAS_CORNER_AREA_WIDTH_x    32
#define BIAS_CORNER_AREA_HEIGHT_x   70

#define BIAS_CORNER_SIZE_x          (BIAS_CORNER_AREA_WIDTH_x * BIAS_CORNER_AREA_HEIGHT_x)

#define ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET_x  492
#define ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET_x    ACTIVE_VERTICAL_OFFSET_x
#define ACTIVE_CENTRAL_AREA_WIDTH_x              IMAGE_WIDTH_x - 2 * ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET_x
#define ACTIVE_CENTRAL_AREA_HEIGHT_x             ACTIVE_HEIGHT_x

#define IMAGE_DEF ImageConfiguration::Get()

// with parameters
#define IMAGE_HORIZONTAL_OFFSET   IMAGE_DEF->image_horizontal_offset
#define IMAGE_VERTICAL_OFFSET     IMAGE_DEF->image_vertical_offset
#define IMAGE_WIDTH               IMAGE_DEF->image_width
#define IMAGE_HEIGHT              IMAGE_DEF->image_height

#define IMAGE                     IMAGE_DEF->image_horizontal_offset, IMAGE_DEF->image_vertical_offset, IMAGE_DEF->image_width, IMAGE_DEF->image_height

#define IMAGE_NB_PIXELS           IMAGE_WIDTH * IMAGE_HEIGHT
#define IMAGE_NB_BYTES            IMAGE_NB_PIXELS * PIXEL_SIZE

#define ACTIVE_HORIZONTAL_OFFSET  IMAGE_DEF->active_horizontal_offset
#define ACTIVE_VERTICAL_OFFSET    IMAGE_DEF->active_vertical_offset
#define ACTIVE_WIDTH              IMAGE_DEF->active_width
#define ACTIVE_HEIGHT             IMAGE_DEF->active_height

#define ACTIVE                    IMAGE_DEF->active_horizontal_offset, IMAGE_DEF->active_vertical_offset, IMAGE_DEF->active_width, IMAGE_DEF->active_height

#define TOP_HORIZONTAL_OFFSET     IMAGE_DEF->top_horizontal_offset
#define TOP_VERTICAL_OFFSET       IMAGE_DEF->top_vertical_offset
#define TOP_WIDTH                 IMAGE_DEF->top_width
#define TOP_HEIGHT                IMAGE_DEF->top_height

#define TOP                       IMAGE_DEF->top_horizontal_offset, IMAGE_DEF->top_vertical_offset, IMAGE_DEF->top_width, IMAGE_DEF->top_height

#define LEFT_HORIZONTAL_OFFSET    IMAGE_DEF->left_horizontal_offset
#define LEFT_VERTICAL_OFFSET      IMAGE_DEF->left_vertical_offset
#define LEFT_WIDTH                IMAGE_DEF->left_width
#define LEFT_HEIGHT               IMAGE_DEF->left_height

#define LEFT                      IMAGE_DEF->left_horizontal_offset, IMAGE_DEF->left_vertical_offset, IMAGE_DEF->left_width, IMAGE_DEF->left_height

#define RIGHT_HORIZONTAL_OFFSET   IMAGE_DEF->right_horizontal_offset
#define RIGHT_VERTICAL_OFFSET     IMAGE_DEF->right_vertical_offset
#define RIGHT_WIDTH               IMAGE_DEF->right_width
#define RIGHT_HEIGHT              IMAGE_DEF->right_height

#define RIGHT                     IMAGE_DEF->right_horizontal_offset, IMAGE_DEF->right_vertical_offset, IMAGE_DEF->right_width, IMAGE_DEF->right_height

#define BIAS_CORNER_AREA_WIDTH    IMAGE_DEF->bias_corner_area_width
#define BIAS_CORNER_AREA_HEIGHT   IMAGE_DEF->bias_corner_area_height

#define BIAS_CORNER_SIZE          (BIAS_CORNER_AREA_WIDTH * BIAS_CORNER_AREA_HEIGHT)

#define ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET  IMAGE_DEF->active_central_area_horizontal_offset
#define ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET    IMAGE_DEF->active_central_area_vertical_offset
#define ACTIVE_CENTRAL_AREA_WIDTH              IMAGE_DEF->active_central_area_width
#define ACTIVE_CENTRAL_AREA_HEIGHT             IMAGE_DEF->active_central_area_height

#define BIAS_A       IMAGE_DEF->bias_a_horizontal_offset, IMAGE_DEF->bias_a_vertical_offset, IMAGE_DEF->bias_a_area_width, IMAGE_DEF->bias_a_height
#define BIAS_B       IMAGE_DEF->bias_b_horizontal_offset, IMAGE_DEF->bias_b_vertical_offset, IMAGE_DEF->bias_b_area_width, IMAGE_DEF->bias_b_height
#define BIAS_C       IMAGE_DEF->bias_c_horizontal_offset, IMAGE_DEF->bias_c_vertical_offset, IMAGE_DEF->bias_c_area_width, IMAGE_DEF->bias_c_height
#define BIAS_D       IMAGE_DEF->bias_d_horizontal_offset, IMAGE_DEF->bias_d_vertical_offset, IMAGE_DEF->bias_d_area_width, IMAGE_DEF->bias_d_height
#define BIAS_CENTER  IMAGE_DEF->bias_center_horizontal_offset, IMAGE_DEF->bias_center_vertical_offset, IMAGE_DEF->bias_center_width, IMAGE_DEF->bias_center_height

/*
    CRITICAL LINK CAMERA return the complete sensor (8096*6048)

    +----------------------------------------+
    |   22 lines                             |
    +------------------------------------+   |
    |                                    |   |
    |                                    |   |
    |   6004 lines                       |   |
    |                                    |   |
    |                                    |   |
    |                                    |   |
    |                                    |   |
    +------------------------------------+   |
    |   22 lines                             |
    +----------------------------------------+

*/

#define CRITICAL_LINK_VERTICAL_OFFSET 22

class ImageConfiguration
{
private:
    static ImageConfiguration* mInstance;

    static ImageConfiguration* GetInstance()
    {
        if(mInstance == nullptr)
        {
            mInstance = new ImageConfiguration();
        }

        return mInstance;
    }

public:
    static ImageConfiguration* Get()
    {
        return GetInstance();
    }

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

    // bias settings
    // a top left
    // b top right
    // c bottom left
    // d bottom right
    // e center
    int bias_a_horizontal_offset;
    int bias_a_vertical_offset;
    int bias_a_area_width;
    int bias_a_height;

    int bias_b_horizontal_offset;
    int bias_b_vertical_offset;
    int bias_b_area_width;
    int bias_b_height;

    int bias_c_horizontal_offset;
    int bias_c_vertical_offset;
    int bias_c_area_width;
    int bias_c_height;

    int bias_d_horizontal_offset;
    int bias_d_vertical_offset;
    int bias_d_area_width;
    int bias_d_height;

    int bias_center_horizontal_offset;
    int bias_center_vertical_offset;
    int bias_center_width;
    int bias_center_height;

    ImageConfiguration()
    {
        // default values
        image_horizontal_offset                = IMAGE_HORIZONTAL_OFFSET_x;
        image_vertical_offset                  = IMAGE_VERTICAL_OFFSET_x;
        image_width                            = IMAGE_WIDTH_x;
        image_height                           = IMAGE_HEIGHT_x;

        active_horizontal_offset               = ACTIVE_HORIZONTAL_OFFSET_x;
        active_vertical_offset                 = ACTIVE_VERTICAL_OFFSET_x;
        active_width                           = ACTIVE_WIDTH_x;
        active_height                          = ACTIVE_HEIGHT_x;

        bias_corner_area_width                 = BIAS_CORNER_AREA_WIDTH_x;
        bias_corner_area_height                = BIAS_CORNER_AREA_HEIGHT_x;

        active_central_area_horizontal_offset  = ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET_x;
        active_central_area_vertical_offset    = ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET_x;

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

    void UpdateConfiguration()
    {
        // recalculate all settings after modification
        UpdateSettings();
    }

private:
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

        // bias area
        bias_a_horizontal_offset    = active_horizontal_offset;
        bias_a_vertical_offset      = active_vertical_offset;
        bias_a_area_width           = bias_corner_area_width;
        bias_a_height               = bias_corner_area_height;

        bias_b_horizontal_offset    = active_horizontal_offset + active_width - bias_corner_area_width;
        bias_b_vertical_offset      = active_vertical_offset;
        bias_b_area_width           = bias_corner_area_width;
        bias_b_height               = bias_corner_area_height;

        bias_c_horizontal_offset    = active_horizontal_offset;
        bias_c_vertical_offset      = active_vertical_offset + active_height - bias_corner_area_height;
        bias_c_area_width           = bias_corner_area_width;
        bias_c_height               = bias_corner_area_height;

        bias_d_horizontal_offset    = active_horizontal_offset + active_width - bias_corner_area_width;
        bias_d_vertical_offset      = active_vertical_offset + active_height - bias_corner_area_height;
        bias_d_area_width           = bias_corner_area_width;
        bias_d_height               = bias_corner_area_height;

        bias_center_width             = active_height;
        bias_center_height            = active_height;

        bias_center_horizontal_offset = active_horizontal_offset + (active_width - bias_center_width) / 2;
        bias_center_vertical_offset   = active_vertical_offset   + (active_height - bias_center_height) / 2;
    }
};

#endif // IMAGE_CONFIGURATION_H
