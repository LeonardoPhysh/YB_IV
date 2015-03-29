/*
 * power.h - Head file power protect module
 *  provice APIs for other modules to detect power state 
 * 
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk> 
 * Date : 2014.9.25
 */

#ifndef __POEWR_H__
#define __POWER_H__

typedef int (*pm_check_t)(void);

struct power_state 
{
    pm_check_t check_power_state;
};

extern struct power_state * get_power_state(void);

#endif /* __POWER_H__ */
