/*
 * ERROR CODE head file 
 *
 * Author: Leoanrdo Physh leonardo.yu@yeah.com
 * Data : 2014.6.28
 */

#ifndef __ERROR_HEAD__
#define __ERROR_HEAD__

/*
 * Global error number 
 */
extern int err_num;
extern char *get_err_msg(int err);

/* RTC */
#define ERT 	1
#define ERT_NUL_RT_OPS 	(ERT + 1)

/* UI */
#define EUI  		100
#define EUI_BACK 	(EUI + 1)
#define EUI_ESC 	(EUI + 2)
#define EUI_ENTER 	(EUI + 3)
#define EUI_TOTAL	(EUI + 4)

#define EUI_HANG 	(EUI + 6)
#define EUI_VIEW 	(EUI + 7)
#define EUI_MODIFY 	(EUI + 8)
#define EUI_SHIFT 	(EUI + 9)

#define EUI_BAD_UI_ID 		(EUI + 10)
#define EUI_BAD_DATE_FORMAT 	(EUI + 11)
#define EUI_BAD_TIME_FORMAT 	(EUI + 10)

/* Handler function */ 
#define EFUNC 		200
#define EFUNC_END_COVER_FAIL    (EFUNC + 1)


/* Display */
#define EDISPLAY 	300

/* Printer */
#define EPRINT 		400
#define EPRINT_NODEV     (EPRINT + 1)
#define EPRINT_INIT    	 (EPRINT + 2)
#define EPRINT_NOT_INIT	 (EPRINT + 4)
#define EPRINT_NOCMD     (EPRINT + 5)
#define EPRINT_NOMOVEUNIT     (EPRINT + 7)
#define EPRINT_NOPAPER        (EPRINT + 8)
#define EPRINT_NOMEM 	(EPRINT + 9)
#define EPRINT_INITFAIL (EPRINT + 10)
#define EPRINT_WR_ERR 	(EPRINT + 11)
#define EPRINT_RD_ERR 	(EPRINT + 12)

/* TAX */
#define ETAX        500
#define ETAX_NOMEM  		(ETAX + 0)
#define ETAX_NUL_RES_BUF 	(ETAX + 1)
#define ETAX_NUL_SEND_BUF 	(ETAX + 2)
#define ETAX_BAD_CARD_TYPE	(ETAX + 3)
#define ETAX_BAD_CMD_TYPE	(ETAX + 4)
#define ETAX_BADAPDU		(ETAX + 5)
#define ETAX_OPEN_CARD		(ETAX + 6)
#define ETAX_WR_CARD		(ETAX + 7)
#define ETAX_RD_CARD 		(ETAX + 8)
#define ETAX_BAD_RES_HEAD 	(ETAX + 9)
#define ETAX_BAD_CHKSUM 	(ETAX + 10)
#define ETAX_WR_FILENAME 	(ETAX + 11)
#define ETAX_BAD_SW 		(ETAX + 12)

#define ETAX_BAD_COLLECT_TYPE 	(ETAX + 14)
#define ETAX_UC_EMPTY		(ETAX + 15)
#define ETAX_FC_PIN_LOCK 	(ETAX + 16)
#define ETAX_CARD_PIN_LOCK 	(ETAX + 17)
#define ETAX_NO_CARD_FILE 	(ETAX + 18)
#define ETAX_MON_OUT_RANG 	(ETAX + 19)
#define ETAX_BAD_XOR 		(ETAX + 20)

#define ETAX_NUL_FISCAL_CARD 	(ETAX + 21)
#define ETAX_NUL_USER_CARD 		(ETAX + 22)
#define ETAX_TAXPAYER_NB_NOT_MATCH  (ETAX + 23)

#define ETAX_NO_CARD_REC    (ETAX + 24)
#define ETAX_UC_NOT_MATCH   (ETAX + 25)
#define ETAX_DATE_CONFUSE   (ETAX + 26)
#define ETAX_DIFF_MACH_NB   (ETAX + 27)
#define ETAX_FC_HAS_REG     (ETAX + 28)
#define ETAX_FC_NOT_READY   (ETAX + 29)
#define ETAX_MACH_LOCKED 	(ETAX + 30)
#define ETAX_FC_NOT_REG 	(ETAX + 31)
#define ETAX_INVOICE_FULL 	(ETAX + 32)
#define ETAX_UC_NOT_READY 	(ETAX + 33)
#define ETAX_INVOICE_MC_EMPTY 	(ETAX + 34)
#define ETAX_NODATA_TO_DECLARE 	(ETAX + 35)
#define ETAX_BAD_DECLARE_TYPE 	(ETAX + 36)
#define ETAX_UNKNOW_ERR		(ETAX + 37)

#define ETAX_ISSUE_LIMIT 		(ETAX + 38)
#define ETAX_FISCAL_NOT_INIT 	(ETAX + 39)
#define ETAX_INVOICE_EMPTY 		(ETAX + 40)
#define ETAX_INVOICE_NOT_MOUNT 	(ETAX + 41)
#define ETAX_AMT_TOTAL_LIMIT	(ETAX + 42)
#define ETAX_AMT_RETURN_LIMIT 	(ETAX + 43)

#define ETAX_USERNAME_IMPLICT	(ETAX + 44)
#define ETAX_CARD_NOT_INIT		(ETAX + 45)
#define ETAX_CARD_ERROR 		(ETAX + 46)
#define ETAX_CARD_NOT_IN 		(ETAX + 47)
#define ETAX_OUT_DECLARE_DATE   (ETAX + 48)
#define ETAX_SPOIL_INV          (ETAX + 49)
#define ETAX_RETURN_INV         (ETAX + 50)
#define ETAX_INV_HAS_RETURNED   (ETAX + 51)
#define ETAX_TAXPAYER_NAME_DISMATCH 	(ETAX + 52)

/* FILE SYS */
#define EFILE 	600
#define EFILE_OPEN_FAIL 	(EFILE + 1)
#define EFILE_SEEK_FAIL 	(EFILE + 2)
#define EFILE_EOF 			(EFILE + 3)
#define EFILE_RD_FAIL 		(EFILE + 4)
#define EFILE_WR_FAIL 		(EFILE + 5)
#define EFILE_OFFSET_OVER_FLOW 	(EFILE + 6)
#define EFILE_REC_HAS_DEL 		(EFILE + 7)
#define EFILE_FULL_DEL_RECORD 	(EFILE + 8)
#define EFILE_DEL_UNAVAIL 		(EFILE + 9)
#define EFILE_DEL_FAIL 		(EFILE + 10)
#define EFILE_REC_FULL 		(EFILE + 11)
#define EFILE_NO_REC        (EFILE + 12)

/* Key board */
#define EKEY 	700
#define EKEY_NO_DATA 		(EKEY + 1)
#define EKEY_ESC_CODE 		(EKEY + 2)
#define EKEY_BACK_CODE 		(EKEY + 3)
#define EKEY_ENTER_CODE 	(EKEY + 4)
#define EKEY_TOTAL_CODE 	(EKEY + 5)
#define EKEY_HANG 		(EKEY + 6)
#define EKEY_VIEW 		(EKEY + 7)
#define EKEY_MODIFY 	(EKEY + 8)
#define EKEY_SHIFT 		(EKEY + 9)

/* PLU */
#define EPLU 	800
#define EPLU_DPT_NOT_EMPTY 	(EPLU + 1)
#define EPLU_DPT_IS_FREE 	(EPLU + 2)
#define EPLU_DPT_IS_BUSY 	(EPLU + 3)
#define EPLU_NO_BARCODE 	(EPLU + 4)
#define EPLU_BARCODE_EXSIT 	(EPLU + 5)
#define EPLU_DEL_BARCODE 	(EPLU + 6)
#define EPLU_NAME_EXSIT 	(EPLU + 7)
#define EPLU_DEL_NAME 		(EPLU + 8)
#define EPLU_NO_NAME 		(EPLU + 9)
#define EPLU_BAD_DPT_NUM	(EPLU + 10)


/* error message */
#define DEFAUT_MSG  "未知错误，请联系厂商！"

#endif /* __ERROR_HEAD__*/

