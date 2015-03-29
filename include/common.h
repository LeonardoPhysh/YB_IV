 /*
 * Common head file of sytem 
 *      you can add your global flags or micro-defines here
 *
 * Author : Leonardo.Physh <leonardo.physh@yahoo.com.hk>
 * Data   : 2014.7.34
 */ 

#ifndef __COMMON_HEAD__
#define __COMMON_HEAD__

#include "real_time.h"

#define SUCCESS  0
#define FAIL    -1

#define POSITIVE    1
#define NEGATIVE    0

#define DELAY(time)    do{int i; for(i = 0; i < time * 100; i++);}while(0)
//#define DEADLY_BUG(x)   report_err(x) //we don't use it

/* common methods */
extern int end_cover_short(uchar *dist, uchar *src);
extern int end_cover_int(uchar *dist, uchar *src);
extern int end_cover_long(uchar *dist, uchar *src);
extern int end_cover_llong(uchar *dist, uchar *src);

extern int add_bcd_date(struct bcd_date *, uint);
extern int sub_bcd_date(struct bcd_date *, uint);

extern int dec_to_bcd(uint *dec_data, uchar *bcd_data);
extern int bcd_to_dec(uchar *bcd_data, uint * dec_data);
extern int bcd_to_str(const uchar *bcd, char *str, int len);

extern unsigned int greg_to_julian(int y, int m, int d);
extern void julian_to_greg(uint jd, uint *year, uint *mon, uint *day);

extern int greg_to_bcd(struct bcd_date *, uint , uint, uint);
extern int bcd_to_greg(struct bcd_date *, uint *, uint *, uint *);

extern int get_anykey(void);
extern int debug_msg(const char *format, ...);
extern int uart_print(const char *format, ...);

extern unsigned short crc16_chk(unsigned char *buf, unsigned int size);
extern unsigned int crc32_chk(unsigned char *buf, unsigned int size);

extern int is_num(int sw_code);
extern int is_ascii(int sw_code);
extern int is_func(int sw_code);
extern int is_printable(int sw_code);

extern void chinese_fee(float fee, char *desc);

/*
 * share from other modules, we put it here because 
 * other modules will often use it.
 */
extern int get_fis_type(void);
extern char * get_fis_type_name(int index);

#endif /* __CONFIG__HEAD__ */
