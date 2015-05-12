/*
 * ERROR CODE Head File 
 *   all error code are defined here, you may add 
 *   your error code here.
 *
 * Author: Leoanrdo Physh <leonardo.physh@yahoo.com.hk> 
 * Data : 2014.6.28
 */

#ifndef __ERROR_HEAD__
#define __ERROR_HEAD__

/*
 * Global error number 
 */
extern int err_num;
extern char *get_err_msg(int err);

/* error message */
#define DEFAUT_MSG  "未知错误，请联系厂商！"

/* RTC */
enum rtc {
	ERT=1,
	ERT_NUL_RT_OPS,
};

/* UI */
enum err_ui {
	EUI = 100,
	EUI_BACK,
	EUI_ESC,
	EUI_ENTER,
	EUI_TOTAL,
	EUI_HANG,
	EUI_VIEW,
	EUI_MODIFY,
	EUI_SHIFT,
	EUI_BAD_UI_ID,
	EUI_BAD_DATE_FORMAT,
	EUI_BAD_TIME_FORMAT,
};

/* Handler function */
enum err_func { 
	EFUNC=200,
	EFUNC_END_COVER_FAIL,
    EFUNC_FIRST_BOOT,
};

/* Display */
enum err_display {
	EDISPLAY = 300,
    ELCM_OPEN_FAIL,
    ELCM_IOCTL_FAIL,
    ELCM_WR_FAIL,
};

/* Printer */
enum err_print {
	EPRINT = 400,
	EPRINT_NODEV,    
	EPRINT_INIT,    	
	EPRINT_NOT_INIT,	
	EPRINT_NOCMD,    
	EPRINT_NOMOVEUNIT,    
	EPRINT_NOPAPER,       
	EPRINT_NOMEM,
	EPRINT_INITFAIL, 
	EPRINT_WR_ERR, 	
	EPRINT_RD_ERR,
    EPRINT_NO_PRINTER,    
};

/* TAX */
enum err_tax {
	/*0*/ETAX = 500,
	/*1*/ETAX_NOMEM, 		
	/*2*/ETAX_NUL_RES_BUF,	
	/*3*/ETAX_NUL_SEND_BUF, 	
	/*4*/ETAX_BAD_CARD_TYPE,	
	/*5*/ETAX_BAD_CMD_TYPE,	
	/*6*/ETAX_BADAPDU,		
	/*7*/ETAX_OPEN_CARD,		
	/*8*/ETAX_WR_CARD,		
	/*9*/ETAX_RD_CARD, 		
	/*10*/ETAX_BAD_RES_HEAD, 	
	/*11*/ETAX_BAD_CHKSUM, 	
	/*12*/ETAX_WR_FILENAME, 	
	/*13*/ETAX_BAD_SW, 		
	/*14*/ETAX_BAD_COLLECT_TYPE,
	/*15*/ETAX_UC_EMPTY,	
	/*16*/ETAX_FC_PIN_LOCK,
	/*17*/ETAX_CARD_PIN_LOCK,
	/*18*/ETAX_NO_CARD_FILE,
	/*19*/ETAX_MON_OUT_RANG,
	/*20*/ETAX_BAD_XOR,
	/*21*/ETAX_NUL_FISCAL_CARD,
	/*22*/ETAX_NUL_USER_CARD,
    /*23*/ETAX_NUL_CHECK_CARD,
	/*24*/ETAX_CHK_CARD_DISMATCH,
    /*25*/ETAX_CHK_CARD_PAST_DUE,
    /*27*/ETAX_CHK_CARD_NOT_READY,
    /*28*/ETAX_AUTH_CHKC_FAIL,
    /*29*/ETAX_NEED_VF,
    /*30*/ETAX_TAXPAYER_NB_NOT_MATCH,
	/*31*/ETAX_NO_CARD_REC,    
	/*32*/ETAX_UC_NOT_MATCH,  
	/*33*/ETAX_DATE_CONFUSE,   
	/*34*/ETAX_DIFF_MACH_NB,   
	/*35*/ETAX_FC_HAS_REG,    
	/*36*/ETAX_FC_NOT_READY,   
	/*37*/ETAX_MACH_LOCKED, 	
	/*38*/ETAX_FC_NOT_REG, 
	/*39*/ETAX_INVOICE_FULL, 
	/*40*/ETAX_UC_NOT_READY,
	/*41*/ETAX_INVOICE_MC_EMPTY, 
	/*42*/ETAX_NODATA_TO_DECLARE, 
	/*43*/ETAX_BAD_DECLARE_TYPE, 	
	/*44*/ETAX_UNKNOW_ERR,
    /*45*/ETAX_ISSUE_LIMIT, 	
	/*46*/ETAX_FISCAL_NOT_INIT, 
	/*47*/ETAX_INVOICE_EMPTY, 	
	/*48*/ETAX_INVOICE_NOT_MOUNT, 
	/*49*/ETAX_AMT_TOTAL_LIMIT,
	/*50*/ETAX_AMT_RETURN_LIMIT, 
	/*51*/ETAX_USERNAME_IMPLICT,	
	/*52*/ETAX_CARD_NOT_INIT,		
	/*53*/ETAX_CARD_ERROR, 		
	/*54*/ETAX_CARD_NOT_IN, 		
	/*55*/ETAX_OUT_DECLARE_DATE,   
	/*56*/ETAX_SPOIL_INV,          
	/*57*/ETAX_RETURN_INV,         
	/*58*/ETAX_INV_HAS_RETURNED,   
	/*59*/ETAX_TAXPAYER_NAME_DISMATCH,	
};

/* FILE SYS */
enum err_file {
	EFILE = 600,
	EFILE_OPEN_FAIL, 
	EFILE_SEEK_FAIL, 
	EFILE_EOF, 		
	EFILE_RD_FAIL, 	
	EFILE_WR_FAIL, 	
	EFILE_OFFSET_OVER_FLOW, 
	EFILE_REC_HAS_DEL, 	
	EFILE_FULL_DEL_RECORD, 
	EFILE_DEL_UNAVAIL, 	
	EFILE_DEL_FAIL, 		
	EFILE_REC_FULL, 		
	EFILE_NO_REC,
};

/* Key board */
enum err_key {
	EKEY = 700,
	EKEY_NO_DATA, 	
	EKEY_ESC_CODE, 	
	EKEY_BACK_CODE, 	
	EKEY_ENTER_CODE, 
	EKEY_TOTAL_CODE, 
	EKEY_HANG, 	
	EKEY_VIEW, 	
	EKEY_MODIFY, 
	EKEY_SHIFT,	
};

/* PLU */
enum err_plu {
	EPLU = 800,
	EPLU_DPT_NOT_EMPTY, 
	EPLU_DPT_IS_FREE, 
	EPLU_DPT_IS_BUSY, 
	EPLU_NO_BARCODE, 
	EPLU_BARCODE_EXSIT,
	EPLU_DEL_BARCODE, 
	EPLU_NAME_EXSIT, 
	EPLU_DEL_NAME, 	
	EPLU_NO_NAME, 	
	EPLU_BAD_DPT_NUM,	
};

#endif /* __ERROR_HEAD__*/
