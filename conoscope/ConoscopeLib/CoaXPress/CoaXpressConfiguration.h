#ifndef COAXPRESS_CONFIGURATION_H
#define COAXPRESS_CONFIGURATION_H


#define NUMBER_OF_FRAME 1

#define COAXPRESS_FRAME_AVERAGE

// when the capture is configured to be the average of more than one image
// standard deviation is also calculated
//#define STD_DEV_FILE
// the measurement of the standard deviation is stored in a float array
//#define STD_DEV_FLOAT

// packet sze for file transfer
#define FILE_TRANSFER_PACKET_SIZE_V10 168
#define FILE_TRANSFER_PACKET_SIZE_V11 1000
//#define FILE_TRANSFER_PACKET_SIZE 1024
#define FILE_TRANSFER_PACKET_ALIGNMENT 4

// this is for debug: pixel format is mono8: faster to display
//#define DISPLAY_FRAME_FORMAT

// configuration of the capture sequance
#define PROCESSING_TIME_US           100000
#define PROCESSING_SENSOR_LAG_US     1000
//#define PROCESSING_SENSOR_LAG_US   100000
#define PROCESSING_STD_DEV_PROC_US   200000

#define CAPTURE_TIMEOUT_FACTOR       10

#define MAX_ACQUISITION_FRAME_PERIOD 1000000

// #define QUIET_MODE

#endif // COAXPRESS_CONFIGURATION_H
