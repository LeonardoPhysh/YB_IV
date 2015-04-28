/*
 * common.c - Define some common methodd used offenly in system
 * 
 * Author : Leonardo Physh  
 * Date   : 2014.8.25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#include "error.h"
#include "common.h"
#include "real_time.h"
#include "keyboard.h"
#include "input.h"


int get_anykey(void)
{
    clear_cache();
    return get_keycode();
}

/* format output debug message */
int uart_print(const char *format, ...)
{
	int ret;
    FILE *dev;

    dev = fopen(DEBUG_DEV, "w");
    if (dev == NULL)
        return -EFILE_OPEN_FAIL;

	va_list arg_ptr;	
	va_start(arg_ptr, format);
	ret = vfprintf(dev, format, arg_ptr);
	if (ret < 0)
		return FAIL;
		
	va_end(arg_ptr);
	
	return SUCCESS;
}

/* output debug message  */
int debug_msg(const char *format, ...)
{
#ifdef CONFIG_DEBUG
	int ret;
	va_list arg_ptr;		
	va_start(arg_ptr, format);
	ret = vprintf(format, arg_ptr);
	if (ret < 0)
		return FAIL;
		
	va_end(arg_ptr);
#else
	/*
	 * do nothing if this is a release version
	 */
#endif 

	return SUCCESS;
}

/*
 * End Cover (big-end <==> littel-end) 
 *  short - 16bits 
 */
int end_cover_short(uchar *dist, uchar *src)
{
    int size;
    uchar *pos;
    uchar tmp[10];

    pos = src;

    size = sizeof(short);
    pos += (size - 1);

    int i;
    for (i = 0; i < size; i++)
        tmp[i] = *pos--;
    
    memcpy(dist, tmp, size);

    return SUCCESS;
}

int end_cover_int(uchar *dist, uchar *src)
{
    int size;
    uchar *pos;
    uchar tmp[10];
    pos = src;

    size = sizeof(int);
    pos += (size - 1);

    int i;
    for (i = 0; i < size; i++)
        tmp[i] = *pos--;

    memcpy(dist, tmp, size);

    return SUCCESS;
}

int end_cover_long(uchar *dist, uchar *src)
{
    int size;
    uchar *pos;
    uchar tmp[10];

    pos = src;

    size = sizeof(long);
    pos += (size - 1);

    int i;
    for (i = 0; i < size; i++)
        tmp[i] = *pos--;

    memcpy(dist, tmp, size);

    return SUCCESS;
}

int end_cover_llong(uchar *dist, uchar *src)
{
    int size;
    uchar *pos;
    uchar tmp[10];

    pos = src;

    size = sizeof(long long);
    pos += (size - 1);

    int i;
    for (i = 0; i < size; i++)
        tmp[i] = *pos--;

    memcpy(dist, tmp, size);

    return SUCCESS;
}

/*
 * dec_to_bcd - data format 
 */
int dec_to_bcd(uint *dec_data, uchar *bcd_data)
{
    uint dec, tmp;
    uchar bcd;

    tmp = 0;
    dec = *dec_data;

    /* ensure dec can be transfer to bcd */
    if (dec >= 100)
        return FAIL;

    /* high 4bits */
    tmp = dec / 10;
    bcd = tmp; 
    bcd = bcd << 4;

    /* low 4bits */
    tmp = dec % 10;
    bcd += tmp;

    *bcd_data = bcd;

    return SUCCESS;
}

/*
 * bcd_to_dec - data format
 */
int bcd_to_dec(uchar *bcd_data, uint *dec_data)
{
    uint dec;
    uchar bcd, tmp; 

    tmp = 0;
    bcd = *bcd_data;

    /* ensure bcd can to transfer to dec */
    if (bcd > 0x99)
        return FAIL;

    /* high 4bits */
    tmp = bcd >> 4;
    dec = tmp;
    dec = dec * 10;

    /* low 4bits*/
    tmp = bcd & 0xF;
    dec += tmp;

    *dec_data = dec;

    return SUCCESS;
}

int bcd_to_str(const uchar *bcd, char *str, int len)
{
    uint i;
    uchar uc;

    for (i = 0; i < len; i++) {
        uc = bcd[i] & 0x0F;
        str[i * 2 + 1] = uc + 0x30;
        uc = (bcd[i] & 0xF0) / 16;
        str[i * 2] = uc + 0x30;
    }

    str[2 * i] = '\0';

    return SUCCESS;
}

/**
 * Converts a Gregorian date to a Julian day.
 * - http://pmyers.pcug.org.au/General/JulianDates.htm
 *  @year:  year
 *  @mon:  month of year 
 *  @day:  day of month
 */
uint greg_to_julian(int y, int m, int d)
{
    uint c, ya, j;

    if (m > 2) {
        m = m - 3;
    } else {
        m = m + 9;
        y = y - 1;
    }

    c = y / 100;
    ya = y - 100 * c;    
    j = (146097 * c) / 4 + (1461 * ya) / 4 + (153 * m + 2) / 5 + d + 1721119;

    return j;
}


/**
 * Converts a Julian day to a Gregorian date. 
 * - http://pmyers.pcug.org.au/General/JulianDates.htm
 *  @julian : the Julian day 
 *  @year : return year 
 *  @mon : return month of year 
 *  @day : return day of month
 */
void julian_to_greg(uint jd, uint *year, uint *mon, uint *day)
{
    uint j = jd;
    uint y, m, d;

    j = j - 1721119 ;
    y = (4 * j - 1) / 146097 ; 
    j = 4 * j - 1 - 146097 * y ; 
    d = j / 4 ;
    j = (4 * d + 3) / 1461 ; 
    d = 4 * d + 3 - 1461 * j ; 
    d = (d + 4) / 4 ;
    m = (5 * d - 3) / 153 ; 
    d = 5 * d - 3 - 153 * m ; 
    d = (d + 5) / 5 ;
    y = 100 * y + j ;

    if (m < 10)
        m = m + 3;
    else {
        m = m - 9; 
        y = y + 1;
    }         

    *year = y;
    *mon = m;
    *day = d;
}

/*
 * greg_to_bcd - Converts Gregorian date and bcd date 
 *  @date : receive buffer 
 *  @year : year 
 *  @mon  : mon 
 *  @day  : day 
 *  @return : status 
 */
int greg_to_bcd(struct bcd_date *date, uint year, uint mon, uint day)
{
    uint top_year;
    uint bot_year;

    top_year = year / 100;
    bot_year = year - top_year * 100;

    dec_to_bcd(&top_year, &date->year[0]);
    dec_to_bcd(&bot_year, &date->year[1]);

    dec_to_bcd(&mon, &date->mon);
    dec_to_bcd(&day, &date->day);

    return SUCCESS;
}

/*
 * bcd_to_greg - Converts Gregorian date and bcd date 
 *  @date : date to Converts 
 *  @year : year 
 *  @mon  : mon 
 *  @day  : day 
 *  @return : status 
 */
int bcd_to_greg(struct bcd_date *date, uint *year, uint *mon, uint *day)
{
    uint top_year = 0;
    uint bot_year = 0;

    bcd_to_dec(&date->year[0], &top_year);
    bcd_to_dec(&date->year[1], &bot_year);

    bcd_to_dec(&date->mon, mon);
    bcd_to_dec(&date->day, day);

    *year = top_year * 100 + bot_year; 

    return SUCCESS;
}

/*
 * add_bcd_date - get bcd date after days 
 *  @days : days to add 
 *  @return : status
 */
int add_bcd_date(struct bcd_date *date, uint days)
{
    uint year;
    uint mon;
    uint day;

    bcd_to_greg(date, &year, &mon, &day);

    /* convert to julian day */
    uint jd;
    jd = greg_to_julian(year, mon, day);
    jd += days;

    /* Convert back */
    julian_to_greg(jd, &year, &mon, &day);

    greg_to_bcd(date, year, mon, day);

    return SUCCESS;
}

/*
 * add_bcd_date - get bcd date after days 
 *  @days : days to add 
 *  @return : status
 */
int sub_bcd_date(struct bcd_date *date, uint days)
{
    uint year;
    uint mon;
    uint day;

    bcd_to_greg(date, &year, &mon, &day);

    /* convert to julian day */
    uint jd;
    jd = greg_to_julian(year, mon, day);
    jd -= days;

    /* Convert back */
    julian_to_greg(jd, &year, &mon, &day);

    greg_to_bcd(date, year, mon, day);

    return SUCCESS;
}

/*
 * creat_crc16_table - creat crc16 table 
 */
static void creat_crc16_table(ushort *crc16_table)
{
    int i, j;
    ushort crc16;

    for (i = 0; i < 256; i++) {
        crc16 = i;
        for (j = 0; j < 8; j++) {
            if (crc16 & 0x1)
                crc16 = (crc16 >> 1) ^ 0xA001;
            else 
                crc16 >>= 1;
        }

        crc16_table[i] = crc16;
    }
}

/*
 * crc16 : crc 16 bits checking
 *  @buf : data 
 *  @size : size of data 
 *  @return : crc16 
 */
ushort crc16_chk(uchar *buf, uint size)
{
    static int init_flag = 0;
    static ushort crc16_table[256];

    int i;
    ushort crc16 = 0x0000;

    if (init_flag == 0) {
        creat_crc16_table(crc16_table);
        init_flag = 1;
    }

    for (i = 0; i < size; i++) {
        crc16 = (crc16 >> 8) ^ crc16_table[(crc16 & 0xFF) ^ buf[i]];
    } 

    return crc16;
}

/*
 * create_crc32_table - create crc32 table
 */
static void creat_crc32_table(uint *crc32_table)
{
    int i, j;
    uint crc32;

    for (i = 0; i < 256; i++) {
        crc32 = i;
        for (j = 0; j < 8; j++) {
            if (crc32 & 0x1)
                crc32 = (crc32 >> 1) ^ 0xEDB88320;
            else 
                crc32 >>= 1;
        }

        crc32_table[i] = crc32;
    }
}

/*
 * crc32 : crc 32bits checking 
 *  @buf : data 
 *  @size : size of data 
 *  @return : crc32 
 */
uint crc32_chk(uchar *buf, uint size)
{
    static int init_flag = 0;
    static uint crc32_table[256];

    int i;
    uint crc32 = 0xffffffff;

    if (init_flag == 0) {
        creat_crc32_table(crc32_table);
        init_flag = 1;
    }

    for (i = 0; i < size; i++) {
        crc32 = (crc32 >> 8) ^ crc32_table[(crc32 & 0xFF) ^ buf[i]];
    } 

    crc32 ^= 0xffffffff;
    return crc32;
}

/*
 * is_ascii - if sw_code is a ascii
 */
int is_ascii(int sw_code)
{
    if (sw_code >= 0 && sw_code <= 255)
        return POSITIVE;

    return NEGATIVE;
} 

/*
 *is_num - if sw_code is a number charactor
 */
int is_num(int sw_code)
{
    if (sw_code >= '0' && sw_code <= '9')
        return POSITIVE;

    return NEGATIVE;
}

/*
 * is_func - if sw_code is funcion keycode 
 */
int is_func(int sw_code)
{
    if (sw_code > KEY_VALUE_FUNC && sw_code <= KEY_VALUE_FUNC + 15) 
        return POSITIVE;

    return NEGATIVE;
}

/*
 * is_printable - if sw_code is a printable keycode
 */
int is_printable(int sw_code)
{
    if (sw_code >= 0x20 && sw_code < 0x7F)
        return POSITIVE;

    return NEGATIVE;
}

/*
 * chinese_fee - translate numer to chinese express 
 * 
 */
void chinese_fee(float fee, char * desc)
{
    float s, j;
    int i, q, g, pos = 0;

    char *a[] = {"壹","贰","叁","肆","伍","陆","柒","捌","玖"};
    char *b[] = {"元","十","百","千","万","十万","百万","千万","亿"};
    char *d[] = {"角","分"};

    s = fee;
    s = s + 0.003;

    for(i = 8; i > 0; i--)
    {
        j = s / pow(10,i);
        if (j != 0)
            break;
    }

    for(q = i-1; q >= 0; q--)
    {
        j = s / pow(10,q);
        g = (int)j % 10;
        if(g != 0) {
            sprintf(desc + pos, "%s%s", a[g-1], b[q]);
            pos += (strlen(a[g-1]) + strlen(b[q]));
        }
    }

    for(q = -1; q >= -2; q--)
    {
        j = s / pow(10,q);
        g=(int)j % 10;
        if(g != 0) {
            sprintf(desc + pos, "%s%s", a[g-1], d[-q-1]);
            pos += (strlen(a[g-1]) + strlen(d[-q-1]));
        }
    }
}

#if 0
int main(void)
{
    int ret;

    char des_s[2] = {0};
    char src_s[2] = {0x01, 0x02};

    char des_i[4] = {0};
    char src_i[4] = {0x01, 0x02, 0x03, 0x04};

    ret = end_cover_int(des_i, src_i);
    if (ret < 0)
        printf("FAIL!\n");

    ret = end_cover_short(des_s, src_s);
    if (ret < 0)
        printf("FAIL!\n");

    int i;
    for (i = 0; i < 2; i++)
        printf("des_s[] : %02x ", des_s[i]);

    printf("\n");

    for (i = 0; i < 4; i++)
        printf("des_i[] : %02x ", des_i[i]);

    printf("\n");

    return 0;
}
#endif 
