#ifndef __COHORT_H__
#define __COHORT_H__

#include <traindef.h>
#include "user/trainstate.h"

// cohort follower self regulate speed

typedef struct {
    usize ahead_train;
    usize follower_train;
} CohortFollowerRegulate;

void cohort_follower_regulate();

// the ideal distance to leave between trains in this cohort
u32 cohort_follow_distance(usize ahead_train, usize ahead_train_speed);

#endif // __COHORT_H__
