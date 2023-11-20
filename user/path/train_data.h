#ifndef __TRAINDATA_H__
#define __TRAIN_DATA_H__

#include <stdint.h>

#define TRAIN_DATA_TRAIN_COUNT 4
#define TRAIN_DATA_SPEED_COUNT 5

#define TRAIN_DATA_SHORT_MOVE_DIST_COUNT 17
#define TRAIN_DATA_SHORT_MOVE_TIME_INCREMENT 250
#define TRAIN_DATA_SHORT_MOVE_SPEED 8

#define TRAIN_SPEED_ROCK   2
#define TRAIN_SPEED_SNAIL  5
#define TRAIN_SPEED_LOW    8
#define TRAIN_SPEED_MED    11
#define TRAIN_SPEED_HIGH   14

uint32_t get_train_index(uint32_t train);

static const uint32_t TRAIN_DATA_TRAINS[TRAIN_DATA_TRAIN_COUNT] = {2, 47, 58, 77};
static const uint32_t TRAIN_DATA_SPEEDS[TRAIN_DATA_SPEED_COUNT] = {2, 5, 8, 11, 14};
static const uint32_t TRAIN_DATA_VEL[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {73, 224, 387, 554, 650},  // 2
    {75, 239, 390, 537, 578},  // 47
    {1, 66, 190, 371, 579},  // 58 (slowest speed is placeholder)
    {1, 99, 232, 408, 612},  // 77 (slowest speed is placeholder)
};
static const uint32_t TRAIN_DATA_STOP_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {60, 275, 525, 770, 971},  // 2
    {1, 284, 521, 709, 828},  // 47 (slowest speed is placeholder)
    {1, 115, 348, 716, 1254},  // 58 (slowest speed is placeholder)
    {1, 173, 447, 929, 1588},  // 77 (slowest speed is placeholder)
};
static const uint32_t TRAIN_DATA_SHORT_MOVE_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SHORT_MOVE_DIST_COUNT] = {
    {0, 10, 17, 33, 66, 97, 132, 186, 235, 305, 376, 475, 601, 725, 890, 992, 1118},  // 2
    {0, 8, 24, 39, 73, 105, 152, 203, 269, 334, 435, 555, 730, 873, 1002, 1110, 1175},  // 47
    {0, 10, 25, 72, 118, 192, 245, 290, 330, 384, 446, 477, 540, 575, 639, 672, 714},  // 58
    {0, 17, 58, 97, 154, 209, 300, 351, 405, 458, 515, 570, 634, 690, 745, 805, 849},  // 77
};

uint32_t train_data_vel(uint32_t train, uint32_t speed);
uint32_t train_data_stop_dist(uint32_t train, uint32_t speed);
uint32_t train_data_short_move_time(uint32_t train, uint32_t dist);

#endif // __TRAIN_DATA_H__
