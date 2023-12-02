#ifndef __COHORT_H__
#define __COHORT_H__

#include <traindef.h>

// cohort follower self regulate speed

#define FOLLOW_DISTANCE 200

typedef struct {
    usize ahead_train;
    usize follower_train;
} CohortFollowerRegulate;

void cohort_follower_regulate();


#endif // __COHORT_H__
