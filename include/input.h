/*
 * Input head file 
 * 		defined some comon data structure. 
 *
 * Author: Leoanrdo Physh<leonardo.physh@yahoo.com.hk>
 * Data : 2014.6.28
 */

#ifndef __INPUT_HEAD__
#define __INPUT_HEAD__

#include <pthread.h>
#include "keyboard.h"

#define     INPUT_FUNC      0
#define     INPUT_UP_CASE   1
#define     INPUT_LOW_CASE  2
#define     INPUT_PINYIN    3
#define     INPUT_SIGN      4

extern int keyboard_stop(void);
extern pthread_t keyboard_init(void);

extern int get_keycode(void);
extern int clear_cache(void);
extern int get_ime_status(void);
extern int set_ime_status(int mode);

extern char * py_ime(char *str);

extern int get_barcode(int row, int col, char *barcode);

extern int get_greg_time(int row, int col, struct greg_time *time);
extern int get_bcd_date(int row, int col, struct bcd_date *date);
extern int get_chn_str(int row, int col, char *name);
extern int get_inter_num(int row, int col, int *num);
extern int get_hex_num(int row, int col, uchar *pin, int len);
extern int get_string(int row, int col, char *string);
extern int get_passwd(int row, int col, char *passwd);

#endif /* __INPUT_HEAD__ */


