/*
 * Head file of tax system 
 *   defined commands that used for the communication between CPU and IC card.
 * 
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : 2014.7.30
 */

#ifndef __TAX_CMD_HEAD__
#define __TAX_CMD_HEAD__

#include "config.h"
#include "common.h"

/* FISCAL_CARD */
#define FC_OS_CMD      0x50
#define FC_RESET       0x51
#define FC_POWER_ON    0x52
#define FC_POWER_OFF   0x53

/* USER CARD */
#define UC_OS_CMD       0xF0
#define UC_RESET        0xF1
#define UC_POWER_ON     0xF2
#define UC_POWER_OFF    0xF3


/* CMD Types */
#define CARD_CMD            20
#define CARD_OS_CMD         CARD_CMD + 1
#define CARD_RST_CMD        CARD_CMD + 2
#define CARD_PWR_ON_CMD     CARD_CMD + 3
#define CARD_PWR_OFF_CMD    CARD_CMD + 4

/*
 * -- DO NOT EDIT THIS STRUCTURE --
 * For Detail, Refer GB 18240.2-2003
 * 
 * -- ATTENTION HERE --
 * Some data in card is big-end, but they are little-end in ARM.
 * when you want to use these data, you should tranfer them carefully.
 */ 

/*
 * GET_REGISTER_NB & TERMINAL_REGISTER
 */
struct register_info
{
    uchar random[4];
    uchar card_nb[8];
    uchar mac1[4];
    uchar mac2[4];
    uchar pin[8];
};


struct rate_index {
	uchar index;
	uchar amount[4];
};

/*
 * ISSUE_INVOICE
 */
struct issue_invoice
{
    uchar date[4];
    
	/*
	 * normal : 0x01
	 * return : 0x02
	 * spoild : 0x03
	 */
    uchar invoice_type;
    uchar invoice_num[4];
    
    /* tax type information */
    struct rate_index tax_type[6];
    
    uchar amt_total[4];
    uchar xor_value;
};

/*
 * Inspect kind record 
 */
struct inspect_invoice
{
    uchar date[4];
    uchar invoice_type;
    uchar invoice_num[4];
    uchar amt_total[4];
    uchar fiscal_code[8];

    uchar ori_invoice_num[4];
    uchar reserved[2];
};


struct issue_invoice_res
{
    char half_top[4];
    char half_bot[4];
    char tax_num[20];
};

/*
 *  DECLARE_DUTY
 */
struct amount_item
{
	uchar amount[4];
};

struct declare_duty
{
    uchar start_date[4];
    uchar end_date[4];
    
    uchar valid_count[4];     
    uchar return_count[2];    
    uchar spoil_count[2];     
    
    uchar tax_index[MAX_USER_TAXRATE];
    /* AMT of normal count */
	struct amount_item amt_valid[MAX_USER_TAXRATE];
    
    /* AMT of return count */
	struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar xor_value;
};

struct inspect_declare
{
    uchar start_date[4];
    uchar end_date[4];

    uchar valid_count[4];     
    uchar return_count[2];    
    uchar spoil_count[2];     

    uchar tax_index[MAX_USER_TAXRATE];
    /* AMT of normal count */
    struct amount_item amt_valid[MAX_USER_TAXRATE];

    /* AMT of return count */
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar total_valid[4];
    uchar total_return[4];
    uchar status;
    uchar mac1[4];
};


struct declare_res
{
    uchar total_valid[4];
    uchar total_return[4];
    uchar status;
    uchar mac1[4];
    uchar sign[128];

    uchar xor_value;
};

/*
 * card with DAILY_COLLECT_SIGN
 */
#define DAILY_COLLECT  1
#define OTHER_COLLECT  0

struct daily_collect
{
    uchar cur_date[4];

    uchar valid_count[2];
    uchar return_count[2];
    uchar spoil_count[2];

    uchar tax_index[MAX_USER_TAXRATE];
    /* AMT of normal count */
    struct amount_item amt_valid[MAX_USER_TAXRATE];

    /* AMT of return count */
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar xor_value;
};

struct inspect_daily
{
    uchar cur_date[4];

    uchar valid_count[2];
    uchar return_count[2];
    uchar spoil_count[2];

    uchar tax_index[MAX_USER_TAXRATE];
    /* AMT of normal count */
    struct amount_item amt_valid[MAX_USER_TAXRATE];

    /* AMT of return count */
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar sign[128];
};

struct daily_collect_res
{
    uchar sign[128];
    uchar xor_value;
};

/*
 * card with DATA_COLLECT
 */
struct data_collect
{
    uchar card_num[8];

    uchar start_date[4];
    uchar end_date[4];

    uchar valid_count[4];
    uchar return_count[2];
    uchar spoil_count[2];

    uchar tax_index[MAX_USER_TAXRATE];
    /* AMT of normal count */
    struct amount_item amt_valid[MAX_USER_TAXRATE];

    /* AMT of return count */
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar total_valid[4];
    uchar total_return[4];

    uchar status;
    uchar mac1[4];
    uchar sign[128];

    uchar xor_value;
};

/*
 *  UPDATE_CONTROLS
 */
struct update_ctl_info
{
    uchar entrypt_id;
    uchar ciphertext[24];
    uchar mac2[4];
};

/*
 *  INPUT_INVOICE_NB & DISTRIBUTE_INVOICE_NB
 */
struct invoice_roll_info
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
    uchar mac[4];
};

/*
 * equal to fis_ef05_record
 */
struct input_invoice_info
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
    uchar roll_num[2];
};

/*
 *  BUY ROLL INFO
 */
struct buy_roll_info
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
    uchar roll_num[2];
};

/*
 * Invoice usd record  
 */
struct inv_use_record
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];

    uchar valid_count;
    uchar return_count;
    uchar spoil_count;

    uchar amt_valid[4];
    uchar amt_spoil[4];

    uchar start_date[4];
    uchar end_date[4];
};

/*
 * common inspect record 
 */
struct inspect_record 
{
    uchar cur_date[4];
    uchar taxpayer_num[8];
    uchar machine_nb[8];
    uchar type;

    union {
        struct inspect_invoice inv_detail[7];
        struct inspect_declare declare_detail[2];
        struct inspect_daily daily_detail;
        uchar reserved[212];
    };
};


/*
 * FISCAL CARD FILE SYSTEM 
 */
/*
 * EF01 record  
 */
struct fis_ef01_record 
{
    uchar issue_limit_date[4];

    uchar single_invoice_limit[4];  
    uchar total_invoice_limit[4];   
    uchar return_invoice_limit[4];  

    uchar tax_index[6];
    uchar detail_mode;
};

/*
 * EF02 record 
 */
struct fis_ef02_record
{
    uchar card_type;
    uchar reg_flag;

    uchar fiscard_card_nb[8];
    uchar mach_num[8];
    uchar taxpayer_num[8];
    uchar taxpayer_id[20];

    uchar app_start_date[4];
    uchar app_vaild_date[4];

    uchar app_type_flag;
    uchar app_version;

    uchar fci_data;

    uchar taxpayer_name[40];
    uchar office_code[4];

    uchar declare_mode;
    uchar pin_mode;
};


/*
 * EF03 record
 */
struct fis_ef03_record
{
    uchar cur_date[4];

    uchar valid_count[2];     
    uchar return_count[2];    
    uchar spoil_count[2];   

    uchar tax_index[MAX_USER_TAXRATE];

    struct amount_item amt_valid[MAX_USER_TAXRATE];
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar pre_inv_num[4];
    uchar pre_fis_code[8];
};


/*
 * EF04 record  
 */
struct fis_ef04_record
{
    uchar start_date[4];
    uchar end_date[4];

    uchar valid_count[4];     
    uchar return_count[2];    
    uchar spoil_count[2];     

    uchar tax_index[MAX_USER_TAXRATE];
    struct amount_item amt_valid[MAX_USER_TAXRATE];
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar status;
    uchar mac1[4];
};

/*
 *  EFO5 record
 */
struct fis_ef05_record
{
    uchar no_invoice;
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
};

/*
 * USER CARD FILE SYSTEM 
 */
/*
 * EF01 record 
 */ 
struct usr_ef01_record
{
    uchar fiscard_card_nb[8];
    uchar encripty_id;

    uchar issue_limit_date[4];

    uchar single_invoice_limit[4];  
    uchar total_invoice_limit[4];   
    uchar return_invoice_limit[4];  

    uchar tax_index[6];
    uchar declar_flag:4;
    uchar fis_code:4;

    uchar reserved;

    uchar mac2[4];
};

/*
 * EF02 record
 */
struct usr_ef02_record
{
    uchar card_type;
    uchar reg_flag;

    /*
     * count of machine 
     */
    uchar mach_nb;
    uchar taxpayer_num[8];
    uchar taxpayer_id[20];

    uchar app_start_date[4];
    uchar app_vaild_date[4];

    uchar app_type_flag;
    uchar app_version;

    uchar fci_data;

    uchar taxpayer_name[40];
};

/*
 *  EF03 record
 */
struct usr_ef03_record
{
    uchar index;
    uchar item_code[4];

    uchar tax_rate[2]; 

    uchar item_cn_name[20];
    uchar item_en_name[20];
};

/*
 * ef04 record 
 */
struct usr_ef04_record
{
    uchar declare_flag;

    uchar fiscal_card_nb[8];

    uchar start_date[4];
    uchar end_date[4];

    uchar valid_count[4];     
    uchar return_count[2];    
    uchar spoil_count[2];     

    uchar tax_index[MAX_USER_TAXRATE];
    struct amount_item amt_valid[MAX_USER_TAXRATE];
    struct amount_item amt_return[MAX_USER_TAXRATE];

    uchar total_valid[4];
    uchar total_return[4];

    uchar status;
    uchar mac[4];
    uchar sign[128];
};

/*
 * EF05 record 
 */
struct usr_ef05_record
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
    uchar roll_num[2];
};

/*
 * EF06 record
 */
struct usr_ef06_record
{
    uchar mach_num[8];
    uchar fiscal_card_nb[8];
    uchar inv_use_record_tag[2];
};


/*
 * EF07 record  
 */
struct usr_ef07_record
{
    uchar invoice_code[10];
    uchar start_num[4];
    uchar end_num[4];
    uchar mac[4];
};

/*
 * EF08 record  
 */
struct usr_ef08_record
{
    //... no need
};

#endif /* __TAX_CMD_HEAD__ */
