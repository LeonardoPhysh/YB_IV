/*
 * Head file of Tax File Operations
 *   define structure of content note
 *
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : 2014.8.12
 */

#ifndef __TAX_FILE_OP_HEAD__
#define __TAX_FILE_OP_HEAD__

#include "config.h"
#include "common.h"
#include "real_time.h"
#include "plu.h"

/*
 * fiscal control system local files 
 */
#define SYS_PATH  "/usr/sys/"

/*
 * user information file  
 *  username, password, level etc
 */
#define USER_FILE 	SYS_PATH"user.dat"
#define USER_FILE_MODE 	DELETE_AVAIL_MODE
#define USER_REC_NUM 	100

/*
 * machine information file 
 *  hw & sw version, machine number etc
 */
#define MACH_INFO_FILE 	SYS_PATH"machine.dat"
#define MACH_INFO_FILE_MODE 	DELETE_UNAVAIL_MODE
#define MACH_INFO_REC_NUM 		1

/*
 * system configuration file 
 *  system status, etc
 */
#define SYS_CFG_FILE 	SYS_PATH"sys_config.dat"
#define SYS_CFG_FILE_MODE 	DELETE_UNAVAIL_MODE
#define SYS_CFG_REC_NUM 	1

/*
 * fiscal configuration file 
 *   fiscal system status, ect
 */
#define FIS_FILE_PATH 	"/usr/fiscal/"
 
#define FIS_CFG_FILE 	FIS_FILE_PATH"fis_config.dat"
#define FIS_CFG_FILE_MODE  	DELETE_UNAVAIL_MODE
#define FIS_CFG_REC_NUM  	1

/* 
 * protect file 
 *  used while need to mount a roll 
 */
#define PROTECT_FILE 	FIS_FILE_PATH"protect.dat"
#define PROTECT_FILE_MODE 	DELETE_UNAVAIL_MODE
#define PROTECT_REC_NUM 	1

/* 
 * pin and origin PIN file 
 *  personal identification number
 */
#define PIN_FILE 	FIS_FILE_PATH"pin.dat"
#define PIN_FILE_MODE 	DELETE_UNAVAIL_MODE
#define PIN_REC_NUM 	1

#define ORIGIN_PIN_FILE 	FIS_FILE_PATH"origin_pin.dat"
#define ORIGIN_PIN_FILE_MODE 	DELETE_UNAVAIL_MODE
#define ORIGIN_PIN_REC_NUM 		1

/* 
 * application & history application information file 
 *  some records about fiscal system app info 
 */
#define APP_FILE 	FIS_FILE_PATH"app_info.dat"
#define APP_FILE_MODE 		DELETE_UNAVAIL_MODE
#define APP_FILE_REC_NUM	1

#define HISTORY_APP_FILE 	FIS_FILE_PATH"his_app_info.dat"
#define HISTORY_APP_FILE_MODE 	DELETE_UNAVAIL_MODE
#define HISTORY_APP_REC_NUM 	100

/* 
 * amount of this machine 
 *  amount of money that this machine issued
 */
#define AMOUNT_FILE 	FIS_FILE_PATH"amount.dat"
#define AMOUNT_FILE_MODE  		DELETE_UNAVAIL_MODE
#define AMOUNT_FILE_REC_NUM 	1

/* 
 * declare duty file 
 *  declare duty record, declare duty is the way to uplord local data
 *  to office.
 */
#define DECLARE_DUTY_FILE  	FIS_FILE_PATH"declare_duty.dat"
#define DECLARE_DUTY_FILE_MODE 		DELETE_UNAVAIL_MODE
#define DECLARE_DUTY_REC_NUM 		2000

/* 
 * buyed roll file 
 *  save the information of user-buyed roll 
 */
#define BUYED_ROLL_FILE  	FIS_FILE_PATH"buy_roll.dat"
#define BUYED_ROLL_FILE_MODE 	DELETE_UNAVAIL_MODE
#define BUYED_ROLL_REC_NUM 		1

/*
 * distribute roll file 
 *  record detail of every time distribute a new roll 
 */
#define DIST_ROLL_FILE  	FIS_FILE_PATH"distribute.dat"
#define DIST_ROLL_FILE_MODE 	DELETE_AVAIL_MODE
#define DIST_ROLL_REC_NUM 		1000

/* 
 * last distribute roll file 
 *  information of last time distribute a roll, normally, used 
 *  to check the status of last time distribute a roll 
 */
#define LAST_DIST_FILE 	FIS_FILE_PATH"last_dist.dat"
#define LAST_DIST_FILE_MODE 	DELETE_AVAIL_MODE
#define LAST_DIST_REC_NUM 		1

/* 
 * curren roll file 
 *  information of current roll 
 */
#define CUR_ROLL_FILE 	FIS_FILE_PATH"cur_roll.dat"
#define CUR_ROLL_FILE_MODE 	DELETE_UNAVAIL_MODE
#define CUR_ROLL_REC_NUM 	1

/* 
 * current roll left invoice number 
 */
#define CRLN_FILE 	FIS_FILE_PATH"cur_left.dat"
#define CRLN_FILE_MODE 		DELETE_UNAVAIL_MODE
#define CRLN_REC_NUM 		1

/* 
 * second invoice file 
 *  for now, have no idear about this file use for
 */
#define SEC_INVOICE_FILE 	FIS_FILE_PATH"sec_invoice.dat"
#define SEC_INVOICE_FILE_MODE  	DELETE_UNAVAIL_MODE
#define SEC_INVOICE_REC_NUM 	1

/*
 * invoice detail information  
 *  as GB said, tax-printer needs to save at least 5 years invoice 
 *  detail information 
 */
#define INVOICE_DETAIL_PATH 	FIS_FILE_PATH"ID/"

/*
 * invoice config file 
 *  save file index, seems to be useless 
 */
#define INVOICE_ID_CFG_FILE 	INVOICE_DETAIL_PATH"inv_cfg.dat"
#define INVOICE_ID_CFG_FILE_MODE 	DELETE_UNAVAIL_MODE
#define INVOICE_ID_CFG_REC_NUM 		1

/* 
 * invoice detail file 
 *  save invoice detail record as GB said
 */
#define INVOICE_DETAIL_PREFIX 	"inv_detail"
#define INVOICE_DETAIL_SUFFIX	".dat"

#define MAX_ID_FILE_NUM 	10
#define INVOICE_DETAIL_FILE_MODE	DELETE_AVAIL_MODE
#define INVOICE_DETAIL_REC_NUM 		10000

/* 
 * invoice detail of today
 */
#define TODAY_ID_FILE	FIS_FILE_PATH"today_id.dat"
#define TODAY_ID_FILE_MODE 		DELETE_UNAVAIL_MODE
#define TODAY_ID_REC_NUM 		3000

/* 
 * daily collect file 
 *  save daily collect record, also called day sum
 */
#define DAILY_COLLECT_FILE 	FIS_FILE_PATH"daily_collect.dat"
#define DAILY_COLLECT_FILE_MODE   DELETE_UNAVAIL_MODE
#define DAILY_COLLECT_REC_NUM 	  3000

/*
 *  invoice detail of current roll 
 */
#define ROLL_ID_FILE 	FIS_FILE_PATH"roll_id.dat"
#define ROLL_ID_FILE_MODE 		DELETE_UNAVAIL_MODE
#define ROLL_ID_REC_NUM	 		1000

/* 
 *  invoice detail of used roll
 */
#define ROLL_COLLECT_FILE 	FIS_FILE_PATH"roll_collect.dat"
#define ROLL_COLLECT_FILE_MODE 	DELETE_UNAVAIL_MODE
#define ROLL_COLLECT_REC_NUM 		500


/*
 * --ATTENTION--
 *  file record note will not transfer to big-end while them been 
 *  write to flash, so when you read them out, you do not need to 
 *  transfer them.
 */

/* 
 * below files do not need to be created, them will fire to flash as rootfs
 * 	USER_FILE 
 * 	MACHINE_INFO_FILE
 * 	SYS_CFG_FILE 
 * -- 
 */
 
 /*
 * fiscal type, read from user card
 *  - fill up application information structure 
 */
struct tax_sys_fiscal_type
{
    uchar index;
    uchar item_code[4];

    ushort tax_rate; 

    char item_cn_name[20 + 1];
    char item_en_name[20 + 1];
};

/*
 * tax rate index item structure 
 */
struct tax_sys_rate_index
{
	uchar index;
	uint amount;
};

/*
 * used to issue a invoice 
 *  - used with ISSUE_INVOICE, after transfer
 */
#define ISSUE_INVOICE_SIZE 		(sizeof(struct tax_sys_issue_invoice))
struct tax_sys_issue_invoice
{
	struct bcd_date date;
	
	/*
	 * normal : 0x01
	 * return : 0x02
	 * spoild : 0x03
	 */
	uchar invoice_type;
	/*
	 * invoice number: used while process a normal invoice 
	 * origin_inv_num: used while process a return invoice
	 */ 
	uint invoice_num;
	uint origin_inv_num;
	
	struct tax_sys_rate_index item[MAX_USER_TAXRATE];
	uint amt_total;
};

/*
 * used for ensure the inspection
 *  - will store in file as invoice detail record 
 */
#define INSPECT_INVOICE_SIZE 	(sizeof(struct tax_inspect_invoice))
struct tax_sys_inspect_invoice
{
	struct bcd_date date;
	
	uchar type;
	
	uint invoice_num;
	uint origin_inv_num;
	
	uint amt_total;
	uchar fiscal_code[20 + 1];
	
	uchar reserved[2];
};

/*
 * commodity items
 */
#define COMM_ITEM_RECORD_SIZE 	 (sizeof(struct tax_sys_comm_item))
struct tax_sys_comm_item

{
	uint num;
	uint amount;

#define comm_plu_name 	plu_item.name	
#define comm_plu_num 	plu_item.plu_item
#define comm_tax_index 	plu_item.tax_index
#define comm_price 		plu_item.price
#define comm_stock 		plu_item.stock

	struct plu_item plu_item;
};

/*
 * used to hang up a transact and resume a transact 
 */
struct tax_sys_transact_items
{
	/*
	 * 1: plu number 
	 * 2: barcode  
	 */
	int type;
	//char payer_name[40];
	
	uint item_num;
	struct tax_sys_comm_item comm_items[10];
};
 
 /*
  * USER_FILE 
  */
#define USER_RECORD_SIZE 	(sizeof(struct user))
struct user 
{
	uchar id;
	uchar level;
	
	char name[USER_NAME_LEN + 1];
	char passwd[USER_PASSWD_LEN + 1];
};

/*
 *  MACH_INFO_FILE
 */
#define MACH_INFO_RECORD_SIZE   (sizeof(struct machine_info_record))
struct machine_info_record
{
    uchar machine_nb[8];

    /* date in produced */
    struct bcd_date produce_date;
    
    int print_nb;
    int cur_print;
    char hw_version[18 + 1];
    char sw_version[18 + 1];
};

/*
 * SYS_CFG_FILE 
 */
#define SYS_CONFIG_RECORD_SIZE  (sizeof(struct tax_sys_config_record))
struct tax_sys_config_record
{
    uchar is_lock;
    uchar is_init;
    
#ifdef 	CONFIG_MUL_BUSINESS
#define COMM_INVOICE_TYPE 	 	1
#define SERVICE_INVOICE_TYPE 	2
	/*
	 * 0 : not defined
	 * 1 : commercial invoice type
	 * 2 : service invoice type 
	 */ 
    uchar invoice_type;
#endif 

#ifdef 	CONFIG_MUL_PRINTER
#define CHGDP_PRINTER		1
#define OTHER_PRINTER 		2	
	/*
	 * it seems like we will just used one kind of printer 
	 * but save it in case of version update 
	 */
    uchar printer_type;
#endif 
	uchar reserved[10];
};


/* 
 * FIS_CFG_FILE 
 */
#define FIS_CONFIG_RECORD_SIZE  (sizeof(struct tax_sys_fis_config_record))
struct tax_sys_fis_config_record
{
    uchar declare_flag;
    uchar pin_lock_flag;
    uchar dist_err_flag;
};


/*
 * PIN record 
 */
#define PIN_RECORD_SIZE		(sizeof(struct tax_sys_pin_record))
struct tax_sys_pin_record 
{
	uchar pin[8];
};

/*
 * APP_FILE
 */
#define APP_INFO_RECORD_SIZE    (sizeof(struct tax_sys_app_info))
struct tax_sys_app_info
{
    uchar fiscal_card_nb[8];
    struct bcd_date app_start_date;
    struct bcd_date app_vaild_date;

    char taxpayer_name[40 + 1];
    uchar taxpayer_nb[8];   //taxpayer's number
    uchar taxpayer_id[20];  //taxpayer's identification

    uchar office_code[4];   //office code  
    uchar declare_mode;     //declare by user card

    struct bcd_date issue_limit_date;  //BCD
    /*
     * amount limit of single, total, return invoice 
     * for portable, avoid big-end and little-end issue 
     */
    uint single_invoice_limit;  
    uint total_invoice_limit;   
    uint return_invoice_limit;  

    uchar detail_mode;  //daclare detail mode, 0: doesn't declare 

    /*
     * number pf tax taxable items
     * index of tax taxable items 
     * number of items of business
     * detail of item of business 
     */
    uchar tax_item_nb;
    uchar tax_index[MAX_USER_TAXRATE];  //max to 6, 0 means no such tax item. 
    uchar fis_type_num;
    struct tax_sys_fiscal_type fis_type[MAX_USER_TAXRATE];

    /* fiscal initialize date */ 
    struct bcd_date init_date;
};

/* 
 * HISTORY_APP_FILE
 */
#define HIS_APP_INFO_RECORD_SIZE 	(sizeof(struct tax_sys_his_app_info))
struct tax_sys_his_app_info
{
    uchar fiscal_card_nb[8];
    
    char taxpayer_name[40 + 1];
    uchar taxpayer_nb[8];
    uchar taxpayer_id[20];
    uchar office_code[4];

    struct bcd_date end_date;
    
    uint used_count;
    uint return_count;
    uint spoil_count;

    uint amt_valid;
    uint amt_return;
};

/*
 * AMOUNT_FILE
 */
#define AMOUNT_INFO_RECORD_SIZE 		sizeof(struct tax_sys_amount_record)
struct tax_sys_amount_record
{
	uint amt_total_this;
	uint amt_return_this;
	uint amt_total_next;
	uint amt_return_next;
};

/*
 * DAILY_COLLECT_FILE
 */
#define DAILY_COLLECT_RECORD_SIZE   (sizeof(struct tax_sys_daily_collect_record)  
struct tax_sys_daily_collect_record 
{
    struct bcd_date cur_date;
    
    ushort valid_count;
    ushort return_count;
    ushort spoil_count;

    uchar tax_index[MAX_USER_TAXRATE];
    uint amt_valid[MAX_USER_TAXRATE];
    uint amt_return[MAX_USER_TAXRATE];
    
    /* card response data */
    uchar sign[128];
};

/*
 * DECLARE_DUTY_FILE
 */
#define DECLARE_DUTY_RECORD_SIZE  (sizeof(struct tax_sys_declare_duty_record))
struct tax_sys_declare_duty_record
{
	struct bcd_date cur_date;
	struct bcd_date	start_date;
	struct bcd_date	end_date;
	
	uint	valid_count;
	ushort 	return_count;
	ushort	spoil_count;
	
	uchar	tax_index[MAX_USER_TAXRATE];
	uint	amt_valid[MAX_USER_TAXRATE];
	uint	amt_return[MAX_USER_TAXRATE];
	
	/* responce data */
	uint	total_valid;
	uint	total_return;
	uchar	status;
	uchar	mac1[4];
	uchar	sign[128];
};

/*
 * BUYED_ROLL_FILE & CUR_ROLL_FILE
 */
#define BUY_ROLL_REROCD_SIZE  (sizeof(struct tax_sys_buy_roll_record))
struct tax_sys_buy_roll_record
{
	uchar	invoice_code[10];
	uint	start_num;
	uint	end_num;
	ushort	roll_num;
};

/*
 * DIST_ROLL_FILE
 */
#define DISTRIBE_ROLL_RECORD_SIZE 	(sizeof(struct tax_sys_invoice_roll_record))
struct tax_sys_invoice_roll_record
{
	uchar invoice_code[10];
	uint start_num;
	uint end_num;
	uchar mac[4];
};

/*
 * PROTECT FILE  
 */
#define PROTECT_RECORD_SIZE 	(sizeof(struct tax_sys_protect_record))
struct tax_sys_protect_record
{
#define PROTECT_TYPE_MOUNT  1
	uchar type;
	struct tax_sys_invoice_roll_record invoice_info;
	uchar reserved[12];
};

/* 
 * CRLN_FILE
 */
#define CUR_ROLL_LEFT_RECORD_SIZE    (sizeof(struct tax_sys_cur_roll_left_record))
struct tax_sys_cur_roll_left_record
{
    int cur_roll_left;  
};

/* 
 * INVOICE_DETAIL_CFG_FILE
 */
#define INVOICE_CFG_RECORD_SIZE 	sizeof(struct tax_sys_id_cfg_record)
struct tax_sys_id_cfg_record
{
	int id_index;
};

/* 
 * INVOICE_DETAIL_FILE
 */
#define INVOICE_DETAIL_RECORD_SIZE 		sizeof(struct tax_sys_invoice_detail_record)
struct tax_sys_invoice_detail_record
{
#define detail_date 		invoice.date 	
#define detail_type 		invoice.type
#define detail_inv_num 		invoice.invoice_num
#define detail_ori_inv_num 	invoice.origin_inv_num
#define detail_amt_total	invoice.amt_total
#define detail_fiscal_code  invoice.fiscal_code

	struct tax_sys_inspect_invoice invoice;
	
	char tax_payee[40 + 1];
	char payer_name[40 + 1];  			
	
	uchar item_num;
	struct tax_sys_comm_item item[10]; 	
	
	char drawer[USER_NAME_LEN + 1];  
	uchar register_num[8];
};

/* 
 * TODAY_ID_FILE
 */
#define TODAY_ID_RECORD_SIZE 	sizeof(struct tax_sys_today_id_record)
struct tax_sys_today_id_record
{
	struct bcd_date date;
	
	uchar type;
	struct tax_sys_rate_index item[MAX_USER_TAXRATE];
};

/* 
 * ROLL_ID_FILE
 */
#define CUR_ROLL_ID_RECORD_SIZE  	sizeof(struct tax_sys_cur_roll_id_record)
struct tax_sys_cur_roll_id_record
{
	struct bcd_date date;
	uchar type;
	uint amout_total;
};

/* 
 * USED_ROLL_ID_FILE
 */
#define USED_ROLL_ID_RECORD_SIZE  (sizeof(struct tax_sys_used_roll_id_record))
struct tax_sys_used_roll_id_record
{
    uchar invoice_code[10];
    uint start_num;
    uint end_num;

    uchar valid_count;
    uchar return_count;
    uchar spoil_count;

    uint amt_valid;
    uint amt_return;

    struct bcd_date start_date;
    struct bcd_date end_date;
};

#define PERIOD_COLLECT_RECORD_SIZE  (sizeof(struct tax_sys_period_collect_record))
struct tax_sys_period_collect_record 
{
    struct bcd_date start_date;
    struct bcd_date end_date;

    int valid_count;
    int return_count;
    int spoil_count;

    int amt_valid;
    int amt_return;
};

/*
 * Inspect helper data structure 
 */
#define CHECK_INFO_SIZE     (sizeof(struct tax_sys_check_info))
struct tax_sys_check_info
{
    uchar taxpayer_nb[8];
    uchar type;
    uchar level;
    struct bcd_date valid_date;
    uchar auth_code[8];
    uchar rec_num;
    uchar total_num;
};

#define CHECK_DAILY_IDX_SIZE    (sizeof(struct tax_sys_check_daily_idx))
struct tax_sys_check_idx
{
    struct bcd_date start_date;
    struct bcd_date end_date;

    int rec_num;
    int start_offset;
    int end_offset;
};

/* file operate methods */
extern int tax_file_save_mach_info(struct machine_info_record * mach_info);
extern int tax_file_read_mach_info(struct machine_info_record * mach_info);

extern int tax_file_save_sys_cfg(struct tax_sys_config_record * sys_cfg);
extern int tax_file_read_sys_cfg(struct tax_sys_config_record * sys_cfg);

extern int tax_file_save_fis_cfg(struct tax_sys_fis_config_record * fis_cfg);
extern int tax_file_read_fis_cfg(struct tax_sys_fis_config_record * fis_cfg);

extern int tax_file_save_protect(struct tax_sys_protect_record * protect_rec);
extern int tax_file_read_protect(struct tax_sys_protect_record * protect_rec);

extern int tax_file_save_pin(struct tax_sys_pin_record *pin);
extern int tax_file_read_pin(struct tax_sys_pin_record * pin);

extern int tax_file_save_origin_pin(struct tax_sys_pin_record *pin);
extern int tax_file_read_origin_pin(struct tax_sys_pin_record * origin_pin);

extern int tax_file_save_app_info(struct tax_sys_app_info * app_info);
extern int tax_file_read_app_info(struct tax_sys_app_info * app_info);

extern int tax_file_save_his_app_info(struct tax_sys_his_app_info * his_app_info);
extern int tax_file_read_his_app_info(struct  tax_sys_his_app_info * his_app_info);

extern int tax_file_save_amount(struct tax_sys_amount_record * amount_rec);
extern int tax_file_read_amount(struct tax_sys_amount_record * amount_rec);

extern int tax_file_save_last_dist(struct tax_sys_invoice_roll_record * dist_roll_rec);
extern int tax_file_read_last_dist(struct tax_sys_invoice_roll_record * dist_roll_rec);

extern int tax_file_save_cur_roll(struct tax_sys_buy_roll_record * cur_roll_record);
extern int tax_file_read_cur_roll(struct tax_sys_buy_roll_record * cur_roll_record);

extern int tax_file_save_crln(struct tax_sys_cur_roll_left_record *cur_roll_left);
extern int tax_file_read_crln(struct tax_sys_cur_roll_left_record * cur_roll_left);

extern int tax_file_save_invoice_cfg(struct tax_sys_id_cfg_record * invoice_cfg_rec);
extern int tax_file_read_invoice_cfg(struct tax_sys_id_cfg_record * invoice_cfg_rec);

extern int tax_file_save_sec_invoice();
extern int tax_file_read_sec_invoice();

extern int tax_file_append_user(struct user * new_user);
extern int tax_file_modify_user(int offset, struct user *user);
extern int tax_file_read_user(int offset, struct user * user);
extern int tax_file_find_user(char *username, struct user * user);
extern int tax_file_delete_user(int offset);

extern int tax_file_append_daily_collect(struct tax_sys_daily_collect_record * daily_collect_rec);
extern int tax_file_modify_daily_collect(int offset, struct tax_sys_daily_collect_record * daily_collect_rec);
extern int tax_file_read_daily_collect(int offset, struct tax_sys_daily_collect_record * daily_collect_rec);
extern int tax_file_find_daily_collect(struct bcd_date * date, struct tax_sys_daily_collect_record * daily_collect_rec);
extern int tax_file_find_chk_daily_idx(struct tax_sys_check_idx * chk_idx);

extern int tax_file_find_period_collect(struct tax_sys_period_collect_record * collect_rec);

extern int tax_file_append_declare_duty(struct tax_sys_declare_duty_record * declare_duty_rec);
extern int tax_file_modify_declare_duty(int offset, struct tax_sys_declare_duty_record * declare_duty_rec);
extern int tax_file_read_declare_duty(int offset, struct tax_sys_declare_duty_record * declare_duty_rec);
extern int tax_file_find_declare_duty(struct bcd_date * date, struct tax_sys_declare_duty_record * declare_duty_rec);
extern int tax_file_find_chk_declare_idx(struct tax_sys_check_idx * chk_idx);

extern int tax_file_save_buyed_roll(struct tax_sys_buy_roll_record * buyed_roll);
extern int tax_file_read_buyed_roll(struct tax_sys_buy_roll_record * buyed_roll);

extern int tax_file_append_dist_roll(struct tax_sys_invoice_roll_record *dist_roll_rec);
extern int tax_file_modify_dist_roll(int index, struct tax_sys_invoice_roll_record* dist_roll_rec);
extern int tax_file_read_dist_roll(int index, struct tax_sys_invoice_roll_record* dist_roll_rec);

extern int tax_file_append_invoice_detail(struct tax_sys_invoice_detail_record * inv_detail_rec);
extern int tax_file_read_invoice_detail(int index, int offset, struct tax_sys_invoice_detail_record * inv_detail_rec);
extern int tax_file_find_invoice_detail(uint *inv_nb, struct tax_sys_invoice_detail_record * inv_detail_rec);
extern int tax_file_find_chk_invoice_idx(struct tax_sys_check_idx * chk_idx);

extern int tax_file_append_today_id(struct tax_sys_today_id_record * today_id_rec);
extern int tax_file_read_today_id(int offset, struct tax_sys_today_id_record * today_id_rec);
extern int tax_file_find_today_id(uint inv_num, struct tax_sys_today_id_record * today_id_rec);

extern int tax_file_append_cur_roll_id(struct tax_sys_cur_roll_id_record * cur_roll_id_rec);
extern int tax_file_modify_cur_roll_id(int offset, struct tax_sys_cur_roll_id_record * cur_roll_id_rec);
extern int tax_file_read_cur_roll_id(int offset, struct tax_sys_cur_roll_id_record * cur_roll_id_rec);
extern int tax_file_find_cur_roll_id(uint inv_num, struct tax_sys_cur_roll_id_record * cur_roll_id_rec);

extern int tax_file_append_used_roll_id(struct tax_sys_used_roll_id_record *used_roll_record);
extern int tax_file_modify_used_roll_id(int offset, struct tax_sys_used_roll_id_record *used_roll_record);
extern int tax_file_read_used_roll_id(int ofset, struct tax_sys_used_roll_id_record *used_roll_record);
extern int tax_file_find_used_roll_id(uint inv_num, struct tax_sys_used_roll_id_record *used_roll_record);

/* Non-Special Method */
extern int tax_file_creat_fiscal_file(void);

extern int tax_file_read_first_record(const char *filename, uchar *buf, int size);
extern int tax_file_read_last_record(const char *filename, uchar *buf, int size);
extern int tax_file_modify_last_record(const char *filename, uchar *buf, int size);
extern int tax_file_get_rec_num(const char *filename);

extern int tax_file_del_record(const char *filename, int offset);
extern int tax_file_clear(const char *filename);
extern int tax_file_term_clear(void);
extern int tax_file_is_empty(const char *filename);
extern int tax_file_is_full(const char *filename);

extern int tax_file_read_prev_record(FILE *fp, int offset, uchar *buf, int size);
extern int tax_file_read_next_record(FILE *fp, int offset, uchar *buf, int size);

#endif /* __TAX_FILE_OP_HEAD__ */


