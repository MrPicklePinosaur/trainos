#ifndef __TRAINDATA_H__
#define __TRAIN_DATA_H__

#include <traindef.h>

#define SPEED_SETTINGS 15

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

static const u32 TRAIN_DATA_TRAINS[TRAIN_DATA_TRAIN_COUNT] = {2, 47, 58, 77};
static const uint32_t TRAIN_DATA_SPEEDS[TRAIN_DATA_SPEED_COUNT] = {2, 5, 8, 11, 14};
static const u32 TRAIN_DATA_VEL[TRAIN_DATA_TRAIN_COUNT][SPEED_SETTINGS] = {
    //{0, 0, 0, 0, 0, 69, 109, 157, 210, 270, 331, 394, 461, 527, 602},  // 1
    {0, 0, 73, 0, 0, 224, 277, 333, 387, 441, 495, 554, 595, 642, 654},  // 2
    //{0, 0, 0, 0, 0, 78, 122, 168, 221, 282, 345, 411, 484, 550, 625}, // 24
    {0, 0, 75, 0, 0, 238, 294, 354, 391, 441, 484, 541, 587, 602, 608},  // 47
    {0, 0, 17, 0, 0, 66, 104, 147, 190, 252, 309, 371, 445, 512, 579},  // 58
    {0, 0, 25, 0, 0, 99, 134, 184, 232, 286, 344, 408, 473, 546, 612},  // 77
};
static const u32 TRAIN_DATA_STOP_DIST[TRAIN_DATA_TRAIN_COUNT][SPEED_SETTINGS] = {
    {0, 0, 60, 0, 0, 275, 334, 427, 525, 545, 627, 770, 799, 891, 970},  // 2
    {0, 0, 56, 0, 0, 287, 329, 417, 520, 538, 596, 707, 747, 815, 817},  // 47
    {0, 0, 14, 0, 0, 115, 0, 0, 348, 0, 0, 716, 0, 0, 1254},  // 58
    {0, 0, 20, 0, 0, 173, 0, 0, 447, 0, 0, 929, 0, 0, 1588},  // 77
};
static const u32 TRAIN_DATA_STOP_TIME[TRAIN_DATA_TRAIN_COUNT][SPEED_SETTINGS] = {
    {0, 0, 1642, 0, 0, 2455, 0, 0, 2713, 0, 0, 2780, 0, 0, 2988},  // 2
    {0, 0, 1493, 0, 0, 2377, 0, 0, 2672, 0, 0, 2641, 0, 0, 2865},  // 47
    {0, 0, 1647, 0, 0, 3485, 0, 0, 3663, 0, 0, 3860, 0, 0, 4332},  // 58
    {0, 0, 1600, 0, 0, 3495, 0, 0, 3853, 0, 0, 4543, 0, 0, 5190},  // 77
};
static const u32 TRAIN_DATA_SHORT_MOVE_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SHORT_MOVE_DIST_COUNT] = {
    {0, 10, 17, 33, 66, 97, 132, 186, 235, 305, 376, 475, 601, 725, 890, 992, 1118},  // 2
    {0, 8, 24, 39, 73, 105, 152, 203, 269, 334, 435, 555, 730, 873, 1002, 1110, 1175},  // 47
    {0, 10, 25, 72, 118, 192, 245, 290, 330, 384, 446, 477, 540, 575, 639, 672, 714},  // 58
    {0, 17, 58, 97, 154, 209, 300, 351, 405, 458, 515, 570, 634, 690, 745, 805, 849},  // 77
};
static const u32 TRAIN_DATA_ACCELERATION_DIST[TRAIN_DATA_TRAIN_COUNT][TRAIN_DATA_SHORT_MOVE_DIST_COUNT] = {
    {0, 0, 10, 0, 0, 318, 0, 0, 870, 0, 0, 1527, 0, 0, 2002},  // 2
    {0, 0, 10, 0, 0, 320, 0, 0, 830, 0, 0, 1524, 0, 0, 1617},  // 47
    {0, 0, 10, 0, 0, 30, 0, 0, 108, 0, 0, 554, 0, 0, 1427},  // 58
    {0, 0, 10, 0, 0, 15, 0, 0, 358, 0, 0, 880, 0, 0, 1656},  // 77
};

u32 train_data_vel(u32 train, u32 speed);
u32 train_data_stop_dist(u32 train, u32 speed);
u32 train_data_stop_time(u32 train, u32 speed);
u32 train_data_short_move_time(u32 train, u32 dist);
u32 train_data_acceleration_dist(u32 train, u32 speed);

u32 get_train_index(u32 train);
u32 get_safe_speed(u32 train, u32 velocity); // get a speed setting that is lower than given velocity

#endif // __TRAIN_DATA_H__
