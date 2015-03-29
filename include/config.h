/*
 * Configuration of sytem 
 *   all project special configuration are defined here.
 *   if you wana add a new CONFIG, be sure about that is 
 *   really need to do so.
 *
 * Author : Leonardo.Physh <leonardo.physh@yahoo.com.hk>
 * Data   : 2014.7.34 Rev 01
 */ 
 
#ifndef __CONFIG__HEAD__
#define __CONFIG__HEAD__

#define HW_VERSION  "DVT_REV1.2"
#define SW_VERSION  "Beta_0.1"
/*--------------------------------------*/

#define DEBUG_DEV  	 "/dev/console"
#define CARD_DEV 	 "/dev/ttySAC1"
#define KEYBD_DEV 	 "/dev/ttySAC2"
#define RTC_DEV      "/dev/rtc0"
#define PRINT_DEV 	 "/dev/printer0"
#define PRINT_SERIAL "/dev/prt_serial0"
#define LCM_DEVICE 	 "/dev/lcm19264"
/*--------------------------------------*/

#define MAX_CARD_TIMEOUT 	25
#define DEFAULT_BAUDRATE    9600

//#define CONFIG_HHP 	    1
#define CONFIG_REV_BETA 	1
#define CONFIG_DEBUG   		1

#ifdef 	CONFIG_DEBUG 		
#define CONFIG_CARD_DEBUG	1
#define CONFIG_TAX_DEBUG	1
#define CONFIG_PLU_DEBUG	1
#define CONFIG_PRINT_DEBUG	1
#define CONFIG_LCM_DEBUG	1
#endif 
/*--------------------------------------*/

//#define CONFIG_DETAIL_DECLARE 	1
#define CONFIG_CMDHEAD_4BYTE		1

#define CONFIG_DIR_MODE  	0777
#define CONFIG_FILE_MODE 	0666

#define CONFIG_MAX_REC_SIZE     1010
#define CONFIG_MAX_DPT 			20
#define CONFIG_MAX_PLU 			999

#define CONFIG_INPUT_CACHE_SIZE 	10
#define CONFIG_CHN_CHAR_SIZE    	2

/*-----------------------------*/

/* 5 years */
#define	MACHINE_LOCK_DATE 	1835

#define MAX_USER_TAXRATE 	6
#define MAX_BUY_ROLL_NUM 	10
#define MAX_CARD_DECNUM     10

#define MAX_COMM_ITEM 		10
#define MAX_STR_LEN  		24
#define USER_PASSWD_LEN 	10
#define USER_NAME_LEN       10

/* user level */
#define MAX_USER_NUM  10
enum user_level {
    NORMAL_USER = 1,
    SUPPER_USER, 	
    MANAGER_USER, 	
    INSPECT_USER, 	
    DEVELOP_USER,	

};
/*-----------------------------*/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int  uint; 
typedef unsigned long ulong;
typedef unsigned long long ullong;
/*-----------------------------*/

#endif /* __CONFIG_HEAD__ */

