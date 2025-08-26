#ifndef PIPELINETYPEFORM_H
#define PIPELINETYPEFORM_H

#include <math.h>
#include "Types.h"
#include "imageConfiguration.h"

#define SQUARE(Value) ((Value)*(Value))

// #define CUMULATIVE_AVERAGE

#define PIXEL_SATURATION    16380
#define HISTOGRAM_COUNT     256

class Histogram
{
public:
    int32 value[HISTOGRAM_COUNT];
    int32 count[HISTOGRAM_COUNT];

    void Reset()
    {
        for (int i = 0; i<HISTOGRAM_COUNT; i++)
        {
            // TODO please check this is possible
            value[i] = (int)((double)PIXEL_SATURATION/255*i);
            count[i] = 0;
        }
    }

    int byteSize()
    {
        return HISTOGRAM_COUNT * sizeof(int32);
    }

    void Add(int16 value)
    {
        int index = 0;

        if(value < 0)
        {
            index = 0;
        }
        else if(value >= PIXEL_SATURATION)
        {
            index = 255;
        }
        else
        {
            index = (int)floor((double)value / ((double)PIXEL_SATURATION/255))+1;
        }

        count[index] ++;
    }
};

#endif // PIPELINETYPEFORM_H
