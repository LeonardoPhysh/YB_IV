/*
 * Head file of real time module
 *   define API and structure used by real tme module
 *
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : Physh 2014.8.13
 */

#ifndef __REAL_TIME_HEAD__
#define __REAL_TIME_HEAD__

#include "common.h"
#include "config.h"

#define DEADLINE_Y 	{0x20, 0x06}
#define DEADLINE_M  0x02
#define DEADLINE_D 	0x01

/*
 * -- ATTENTION-- 
 *  NEVER TOUCH IT.
 *  Bcd format date
 */
struct bcd_date
{
    uchar year[2];
    uchar mon;
    uchar day;
};

struct greg_time 
{
    int hour;
    int min;
    int sec;
};

typedef int (*cmp_date_t)(struct bcd_date *, struct bcd_date *);
typedef int (*cur_date_t)(struct bcd_date *);

typedef int (*cmp_time_t)(struct greg_time *, struct greg_time *);
typedef int (*cur_time_t)(struct greg_time *);

struct rt_operate 
{
    cmp_date_t cmp_bcd_date;
    cur_date_t get_cur_date;
    cur_date_t set_cur_date;
    
    cmp_time_t cmp_greg_time;
    cur_time_t get_cur_time;
    cur_time_t set_cur_time;

    cur_date_t get_next_date;
    cur_date_t get_prev_date;
};

extern struct rt_operate * get_rt_ops(void);

#endif 	/* __REAL_TIME_HEAD__ */
