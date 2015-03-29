/*
 * lcm_api.h - Head file of LCM API module 
 *   defined APIs and data structures of LCM module 
 *
 *  Author : Aningsk.Ning 
 *  Last UpDate : 2014.10.15
 */
#ifndef __LCM_API_H
#define __LCM_API_H

#define LCM_CLEAR_CMD   _IO('L', 0x21)
#define LCM_CHAR_CHN    _IO('L', 0x22)
#define LCM_CHAR_ENG    _IO('L', 0x23)

#define LCM_CURSOR_ON_IC1   _IO('L', 0x24)
#define LCM_CURSOR_ON_IC2   _IO('L', 0x33)

#define LCM_CURSOR_OFF_IC1  _IO('L', 0x25)
#define LCM_CURSOR_OFF_IC2  _IO('L', 0x34)

#define LCM_REV_CURSOR  _IO('L', 0x26)

#define LCM_REV_LINE_1  _IO('L', 0x27)
#define LCM_REV_LINE_2  _IO('L', 0x28)
#define LCM_REV_LINE_3  _IO('L', 0x29)
#define LCM_REV_LINE_4  _IO('L', 0x30)

#define LCM_DB5_H       _IO('L', 0x31)
#define LCM_DB5_L       _IO('L', 0X32)

#define ON  1
#define OFF 0

struct lcm_data 
{
    int y;
    int x;
    int size;
    char info[28];
};

extern int lcm_clear(void);
extern int lcm_charset(char c);
extern int lcm_printf(int row, int col, char *str);
extern int lcm_set_cursor(int row, int col, int flag);

extern int lcm_rev_cursor(int row, int col);
extern int lcm_rev_line(int num);

#endif 

