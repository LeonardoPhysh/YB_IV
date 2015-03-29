/*
 * Head file of tax base subsystem 
 *   define APIs that used to communicate CPU and IC card.
 * 
 * Author : Leonardo Physh <Leonardo.physh@yahoo.com.hk>
 * Date   : 2014.7.30  Rev 01
 */
 
#ifndef __TAX_BASE_HEAD__
#define __TAX_BASE_HEAD__

#include "config.h"
#include "common.h"
#include "tax_cmd.h"

/*
 * --ATTENTION--
 *  Plese really attention this, data in IC card is stored as bid-end, 
 *  date in CPU is stored as littel-end, you need to transfer them 
 *  before using it.
 */

/*
 * There are some differences between user card & fiscal card,
 * however check card & user card ara same type.
 */
#define USER_CARD       0
#define CHECK_CARD      0
#define FISCAL_CARD     1

#define MAX_WRBUF_SIZE		300
#define MAX_CONDBODY_SIZE   300
#define MAX_RESBODY_SIZE    300

/* APDU sent buffer */
struct apdu_cmd_send
{
    uchar CLA;
    uchar INS;
    uchar P1;
    uchar P2;
    uchar cond_body[MAX_CONDBODY_SIZE];
}__attribute__((packed));

struct card_send_buf
{
#ifdef CONFIG_CMDHEAD_4BYTE
    uchar cmd_head[4];
#else
    uchar cmd_head[1];
#endif 

    uchar cmd_len;
    uchar cmd_type;
    struct apdu_cmd_send cmd_buf;
    uchar chksum;
}__attribute__((packed));

/* APDU revice buffer */
struct apdu_cmd_res
{
    uchar data[MAX_RESBODY_SIZE];
    uchar sw1;
    uchar sw2;
}__attribute__((packed));

struct card_res_buf
{
	uchar cmd_head;
    uchar cmd_len;
    uchar cmd_type;
    struct apdu_cmd_res res_buf;
    uchar chksum;
}__attribute__((packed));

/*
 * following elemetns will open access 
 */

typedef int (*POWER)(int);
typedef int (*SELECT_DF)(int, const char *);
typedef int (*SELECT_ID)(int, int);
typedef int (*READ_BINARY)(int, int, int, uchar *);
typedef int (*UPDATE_BINARY)(int, int, int, uchar *);
typedef int (*CARD_READ_RECORD)(int, uchar, int, uchar *);
typedef int (*UPDATE_RECORD)(int, uchar, int, uchar *);
typedef int (*CARD_VERIFY)(int, uchar, uchar, uchar *);
typedef int (*EXTERNAL_AUTH)(int, uchar, uchar *);
typedef int (*INTERNAL_AUTH)(int, uchar, uchar *, uchar *);
typedef int (*GET_CHALLENGE)(int, uchar, uchar *);
typedef int (*REGISETER)(struct register_info *);
typedef int (*ISSUE_INVOICE)(struct issue_invoice *, struct issue_invoice_res *);
typedef int (*DECLARE_DUTY)(struct declare_duty *, struct declare_res *);
typedef int (*UPTATE_CTL)(struct update_ctl_info *);
typedef int (*INPUT_INV_NB)(struct invoice_roll_info *);
typedef int (*VER_FIS_PIN)(uchar *, uchar *);
typedef int (*DAILY_COL_SIGN)(struct daily_collect *, struct daily_collect_res *);
typedef int (*REG_SIGN)(struct register_info *);
typedef int (*DATA_COL_SIGN)(struct data_collect *, struct daily_collect_res *);
typedef int (*DATA_COLLECT)(struct data_collect *);
typedef int (*DISTR_INV_NB)(struct invoice_roll_info *);
typedef int (*DEVICE_INIT)(void);

#ifdef CONFIG_CARD_DEBUG
typedef struct card_send_buf * (*send_buf_t)(void);
typedef	struct card_res_buf * (*res_buf_t)(void);
#endif 

struct fiscal_card {
    /* status of card*/
    int power_mark;
    
#ifdef CONFIG_CARD_DEBUG
	send_buf_t card_send_buf;
	res_buf_t card_res_buf;
#endif 
    
    DEVICE_INIT device_init;
    DEVICE_INIT device_stop;

    /* common function */
    POWER power_on;
    POWER power_off;
    POWER power_rst;
    SELECT_DF select_file_by_df;
    SELECT_ID select_file_by_id;
    READ_BINARY read_binary;
    UPDATE_BINARY update_binary;
    CARD_READ_RECORD read_record;
    UPDATE_RECORD update_record;
    CARD_VERIFY card_verify;
    EXTERNAL_AUTH external_auth;
    INTERNAL_AUTH internal_auth;
    GET_CHALLENGE get_challenge;
    
    /* fiscal card special */
    REGISETER get_register_nb;
    REGISETER terminal_register;
    ISSUE_INVOICE issue_invoice;
    DECLARE_DUTY declare_duty;
    UPTATE_CTL update_controls;
    INPUT_INV_NB input_invoice_nb;
    VER_FIS_PIN verify_fiscal_pin;
    DAILY_COL_SIGN daily_collect_sign;
    DATA_COL_SIGN data_collect_sign;
};

struct user_card {
    /* status of card*/
    int power_mark;
    
#ifdef CONFIG_CARD_DEBUG
	send_buf_t card_send_buf;
	res_buf_t card_res_buf;
#endif 
    
    DEVICE_INIT device_init;
    DEVICE_INIT device_stop;

    /* common function */
    POWER power_on;
    POWER power_off;
    POWER power_rst;
    SELECT_DF select_file_by_df;
    SELECT_ID select_file_by_id;
    READ_BINARY read_binary;
    UPDATE_BINARY update_binary;
    CARD_READ_RECORD read_record;
    UPDATE_RECORD update_record;
    CARD_VERIFY card_verify;
    EXTERNAL_AUTH external_auth;
    INTERNAL_AUTH internal_auth;
    GET_CHALLENGE get_challenge;
    
    /* user card special */
    REG_SIGN register_sign;
    DATA_COLLECT data_collect;
    DISTR_INV_NB distribute_invoice_nb;
};

struct check_card {
    /* status of card*/
    int power_mark;
    
#ifdef CONFIG_CARD_DEBUG
	send_buf_t card_send_buf;
	res_buf_t card_res_buf;
#endif 
    
    DEVICE_INIT device_init;
    DEVICE_INIT device_stop;

    /* common function */
    POWER power_on;
    POWER power_off;
    POWER power_rst;
    SELECT_DF select_file_by_df;
    SELECT_ID select_file_by_id;
    READ_BINARY read_binary;
    UPDATE_BINARY update_binary;
    CARD_READ_RECORD read_record;
    UPDATE_RECORD update_record;
    CARD_VERIFY card_verify;
    EXTERNAL_AUTH external_auth;
    INTERNAL_AUTH internal_auth;
    GET_CHALLENGE get_challenge;
    
    /* check card special */
};

/* API for others modules */
extern struct fiscal_card * get_fiscal_card(void);
extern struct user_card * get_user_card(void);
extern struct check_card * get_check_card(void);

#endif /* __TAX_BASE_HEAD__ */
