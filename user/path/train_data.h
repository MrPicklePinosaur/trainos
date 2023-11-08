#ifndef __TRAINDATA_H__
#define __TRAIN_DATA_H__

#include <stdint.h>

#define TRAIN_DATA_TRAIN_COUNT 2
#define TRAIN_DATA_SPEED_COUNT 4

#define TRAIN_SPEED_SNAIL  5
#define TRAIN_SPEED_LOW   8
#define TRAIN_SPEED_MED    11
#define TRAIN_SPEED_HIGH   14

static const uint32_t TRAIN_DATA_TRAINS[TRAIN_DATA_TRAIN_COUNT] = {2, 47};
static const uint32_t TRAIN_DATA_SPEEDS[TRAIN_DATA_SPEED_COUNT] = {5, 8, 11, 14};
static const uint32_t TRAIN_DATA_VEL[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {224, 387, 554, 650},  // 2
    {239, 390, 537, 578},  // 47
};
static const uint32_t TRAIN_DATA_STOP_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SPEED_COUNT] = {
    {275, 525, 770, 971},  // 2
    {284, 521, 709, 828},  // 47
};

uint32_t train_data_vel(uint32_t train, uint32_t speed);
uint32_t train_data_stop_dist(uint32_t train, uint32_t speed);

#endif // __TRAIN_DATA_H__
