/*
 * lcm_api.h - Head file of LCM API module 
 *   defined APIs and data structures for LCM module 
 *
 *  Author : Leonardo Physh & Aningsk.Ning 
 *  Date   : 2014.10.15
 */
#ifndef __LCM_API_H
#define __LCM_API_H

#define LCM_CLEAR_CMD       _IO('L', 0x21)
#define LCM_CURSOR_CMD      _IO('L', 0x22)
#define LCM_REVERSE_CMD     _IO('L', 0x23)

#define ON  1
#define OFF 0

/*
 * lcm_data is the data format that conmucate
 * between user program and kernel driver.
 */
struct lcm_data 
{
    int row;
    int col;
    int size;
    char info[28];
};

extern int lcm_clear(void);
extern int lcm_printf(int row, int col, char *str);
extern int lcm_set_cursor(int row, int col, int cursor, int blink);
extern int lcm_rev_line(int num);

//obsolete API 
//extern int lcm_charset(char c);
//extern int lcm_rev_cursor(int row, int col);
#endif 

