/*
 * Real time control function 
 *  - implement functions about date and time 
 * 
 * Author : Leonardo Physh 
 * Date   : 2014.9.1 Rev01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "common.h"
#include "config.h"
#include "real_time.h"

struct bcd_date deadline  = {
	.year = DEADLINE_Y,
	.mon = DEADLINE_M,
	.day = DEADLINE_D,
};

/*
 * rt_cmp_greg_time - compare 2 time 
 *  @a_time : time 
 *  @b_time : another time 
 *  @return : compare result
 */
static int rt_cmp_greg_time(struct greg_time * a_time, struct greg_time *b_time) 
{
    int ret;

    /* hour */
    ret = a_time->hour - b_time->hour;
    if (ret != 0)
       return ret;
    
    ret = a_time->min - b_time->min;
    if (ret != 0)
        return ret;

    ret = a_time->sec - b_time->sec;
    if (ret != 0)
        return ret;

    return ret;
}

/*
 * rt_get_cur_time - get current time as hour minitu second 
 *  @time : receive buffer
 *  @return : status 
 */
static int rt_get_cur_time(struct greg_time *time)
{
    int retval,fd;
    struct rtc_time rtc_tm;
    
    fd = open(RTC_DEV, O_RDWR);    
    if(fd == -1) {
        printf("error: open /dev/rtc");
        return FAIL;

    }

    retval = ioctl(fd, RTC_RD_TIME,&rtc_tm);
    if(retval == -1) {
        printf("error : RTC_RD_TIME ioctl");
        goto fail;

    }
    
    time->hour = rtc_tm.tm_hour;
    time->min = rtc_tm.tm_min;
    time->sec = rtc_tm.tm_sec;
    
    retval = SUCCESS;

fail:
    close(fd);
    return retval;
}

/*
 * rt_set_cur_tiem - set current time 
 *  @time : setup time 
 *  @return : status  
 */
static int rt_set_cur_time(struct greg_time *time)
{
    int retval,fd;
    struct rtc_time rtc_tm;

    fd = open(RTC_DEV, O_RDWR);    
    if(fd == -1)
    {
        printf("error: open /dev/rtc");
        return FAIL;

    }

    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if(retval == -1)
    {
        printf("error : RTC_RD_TIME ioctl");
        goto fail;

    }

    rtc_tm.tm_hour = time->hour;
    rtc_tm.tm_min = time->min;
    rtc_tm.tm_sec = time->sec;

    retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
    if(retval == -1)
    {
        printf("error : RTC_SET_TIME ioctl");
        goto fail;

    }

    retval = SUCCESS;

fail:
    close(fd);
    return retval;
}

/*
 * rt_cmp_bcd_date - compare 2 date 
 *  @a_date : date 
 *  @b_date : another date 
 *  @return : compare result 
 */
static int rt_cmp_bcd_date(struct bcd_date * a_date, struct bcd_date * b_date)
{
    int ret;

    /* year */
    ret = memcmp(a_date->year, b_date->year, 2);
    if (ret != 0)
        return ret;

    /* mon */
    ret = memcmp(&a_date->mon, &b_date->mon, 1);
    if (ret != 0)
        return ret;

    /* day */
    ret= memcmp(&a_date->day, &b_date->day, 1);

    return ret;
}

/*
 * rt_set_cur_date - setup system date 
 *  @date :setup date 
 *  @return : status 
 */
static int rt_set_cur_date(struct bcd_date *date) 
{
    int retval,fd;
    struct rtc_time rtc_tm;

    uint year, mon, day;

    bcd_to_greg(date, &year, &mon, &day);
    
    fd = open(RTC_DEV, O_RDWR);    
    if(fd == -1)
    {
        debug_msg("error: open /dev/rtc0\n");
        return FAIL;

    }

    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if(retval == -1)
    {
        debug_msg("error : RTC_RD_TIME ioctl\n");
        goto fail;

    }

    rtc_tm.tm_year = year - 1900;
    rtc_tm.tm_mon = mon - 1;
    rtc_tm.tm_mday = day;

    retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
    if(retval == -1)
    {
        debug_msg("error : RTC_SET_TIME ioctl\n");
        goto fail;

    }

    retval = SUCCESS;

fail:
    close(fd);
    return retval;
}

/*
 * rt_get_cur_date - get current date 
 *  @data : receive buffer 
 *  @return : status 
 */
static int rt_get_cur_date(struct bcd_date * date)
{
    int retval,fd;
    struct rtc_time rtc_tm;

    int year, mon, day;

    fd = open(RTC_DEV, O_RDWR);    
    if(fd == -1) {
        printf("error: open /dev/rtc0\n");
        return FAIL;

    }

    retval=ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if(retval == -1) {
        printf("error : RTC_RD_TIME ioctl\n");
        goto fail;

    }

    year = rtc_tm.tm_year + 1900;
    mon = rtc_tm.tm_mon + 1;
    day = rtc_tm.tm_mday;

    greg_to_bcd(date, year, mon, day);
 
    retval = SUCCESS;

fail:
    close(fd);
    return retval;
}

/*
 * rt_get_prev_date - get previous date 
 *  @date : input date & output date 
 *  @return : status 
 */
static int rt_get_prev_date(struct bcd_date * date)
{
    uint top_year;
    uint bot_year;

    uint year;
    uint mon;
    uint day;

    bcd_to_dec(&date->year[0], &top_year);
    bcd_to_dec(&date->year[1], &bot_year);

    bcd_to_dec(&date->mon, &mon);
    bcd_to_dec(&date->day, &day);

    year = top_year * 100 + bot_year;

    /* convert to julian day */
    uint jd;
    jd = greg_to_julian(year, mon, day);
    jd--;

    /* Convert back */
    julian_to_greg(jd, &year, &mon, &day);

    top_year = year / 100;
    bot_year = year - top_year * 100;

    dec_to_bcd(&top_year, &date->year[0]);
    dec_to_bcd(&bot_year, &date->year[1]);

    dec_to_bcd(&mon, &date->mon);
    dec_to_bcd(&day, &date->day);

    return SUCCESS;
}

/*
 * rt_get_next_date : get the next of date 
 *  @date : input date & output date 
 *  @return : status 
 */
static int rt_get_next_date(struct bcd_date * date)
{
    uint top_year;
    uint bot_year;

    uint year;
    uint mon;
    uint day;

    bcd_to_dec(&date->year[0], &top_year);
    bcd_to_dec(&date->year[1], &bot_year);

    bcd_to_dec(&date->mon, &mon);
    bcd_to_dec(&date->day, &day);

    year = top_year * 100 + bot_year;

    /* convert to julian day */
    uint jd;
    jd = greg_to_julian(year, mon, day);
    jd++;

    /* convert it back */
    julian_to_greg(jd, &year, &mon, &day);

    top_year = year / 100;
    bot_year = year - top_year * 100;

    dec_to_bcd(&top_year, &date->year[0]);
    dec_to_bcd(&bot_year, &date->year[1]);

    dec_to_bcd(&mon, &date->mon);
    dec_to_bcd(&day, &date->day);

    return SUCCESS;
}


#ifdef CONFIG_CHECK_DEADLINE
static int rt_check_deadline(struct bcd_date * today)
{
    //...
    return SUCCESS;
}
#else
static int rt_check_deadline(struct bcd_date * today)
{
    return SUCCESS;
}
#endif 

struct rt_operate g_rt_operate = {
    .cmp_bcd_date = rt_cmp_bcd_date,
    .get_cur_date = rt_get_cur_date,
    .set_cur_date = rt_set_cur_date,

    .get_next_date = rt_get_next_date,
    .get_prev_date = rt_get_prev_date,

    .cmp_greg_time = rt_cmp_greg_time,
    .get_cur_time = rt_get_cur_time,
    .set_cur_time = rt_set_cur_time,
};

/*
 * APIs to other modules
 */
struct rt_operate * get_rt_ops(void) 
{
    return &g_rt_operate;
}

#if 0
int main(void)
{
    int y, m, d;
    struct bcd_date date;

    printf("Input Y M D:");
    scanf("%d %d %d", &y, &m, &d);
    
    greg_to_bcd(&date, y, m, d);

    rt_get_next_date(&date);

    bcd_to_greg(&date, &y, &m, &d);

    printf("\nNext date: %d-%02d-%02d\n", y, m, d);

    return 0;
}
#endif 
