/*
 * Head file of key board function module 
 *  -define data structure that used by key board function 
 * 
 * Author : Leoanrdo Physh 
 * Date   : 2014.9.14 Rev01 
 */

#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

#include "config.h"

#define     HW_KEY_CODE_START   0x04
#define     HW_KEY_CODE_END     0x64

#define     KEY_VALUE_FUNC      300
#define     KEY_VALUE_VIEW      (KEY_VALUE_FUNC + 1)
#define     KEY_VALUE_HANG      (KEY_VALUE_FUNC + 2)
#define     KEY_VALUE_SHIFT     (KEY_VALUE_FUNC + 3)   
#define     KEY_VALUE_CANCLE    (KEY_VALUE_FUNC + 5)
#define     KEY_VALUE_BACKFW    (KEY_VALUE_FUNC + 6)
#define     KEY_VALUE_TOTAL     (KEY_VALUE_FUNC + 7)
#define     KEY_VALUE_ENTER     (KEY_VALUE_FUNC + 8)

#define     KEY_VALUE_PAPER_IN  (KEY_VALUE_FUNC + 9)
#define     KEY_VALUE_PAPER_OUT (KEY_VALUE_FUNC + 10)

#define     KEY_VALUE_ARCHER_UP     (KEY_VALUE_FUNC + 11)
#define     KEY_VALUE_ARCHER_DOWN   (KEY_VALUE_FUNC + 12)
#define     KEY_VALUE_ARCHER_LEFT   (KEY_VALUE_FUNC + 13)
#define     KEY_VALUE_ARCHER_RIGHT  (KEY_VALUE_FUNC + 14)
#define     KEY_VALUE_DOUBLE_ZERO   (KEY_VALUE_FUNC + 15)

#define     VIEW    KEY_VALUE_VIEW
#define     HANG    KEY_VALUE_HANG
#define     SHIFT   KEY_VALUE_SHIFT
#define     ESC     KEY_VALUE_CANCLE
#define     BACK    KEY_VALUE_BACKFW
#define     TOTAL   KEY_VALUE_TOTAL
#define     ENTER   KEY_VALUE_ENTER
#define     MODIFY  KEY_VALUE_SHIFT

#define     PAPER_IN    KEY_VALUE_PAPER_IN
#define     PAPER_OUT   KEY_VALUE_PAPER_OUT

#define     UP      KEY_VALUE_ARCHER_UP
#define     DOWN    KEY_VALUE_ARCHER_DOWN
#define     LEFT    KEY_VALUE_ARCHER_LEFT
#define     RIGHT   KEY_VALUE_ARCHER_RIGHT

#define     D_ZERO  KEY_VALUE_DOUBLE_ZERO

#ifndef __CONFIG__HEAD__
typedef unsigned char uchar; 
typedef unsigned int  uint;
#endif 

#define  	CACHE_SIZE  CONFIG_INPUT_CACHE_SIZE

struct key_code {
    volatile uint flag;
    volatile uint end;
    uint key_value[CACHE_SIZE];
};

#endif  /* __KEYBOARD_H_ */ 
