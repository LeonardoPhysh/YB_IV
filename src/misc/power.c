/*
 * power.c - power protect main file 
 *  - implement the power detect function 
 *
 * Author : Leonardo Physh 
 * Date   : 2014.9.12
 */

#include "config.h"
#include "common.h"
#include "power.h"

/*
 *
 */
static int power_check_power_state()
{
#ifdef CONFIG_REV_BETA
    return POSITIVE; 
#else 
    /*
     * comming soon
     */
     return POSITIVE;
#endif 
}

struct power_state pm_base = {
    //...
    .check_power_state = power_check_power_state,
};

struct power_state * get_power_state(void)
{
    return &pm_base;
}
