/*
 * Head file of Tax system 
 *  any definations we might used are defined here.
 * 
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : 2014.7.30
 */
 
#ifndef __TAX_SYSTEM_HEAD__
#define __TAX_SYSTEM_HEAD__

#include <stdio.h>

#include "common.h"
#include "real_time.h"
#include "tax_cmd.h"
#include "tax_file_op.h"
#include "tax_card_base.h"

/*
 * fiscal card, user card, check card - filesystem  
 */
#define TSAM01      "TSAM01"   //Fiscal card application directory
#define TID01       "TID01"    //User card application directory
#define CHECK01     "CHECK01"  //Check card application direcory 

#define EF01		0xEF01
#define EF02		0xEF02
#define EF03		0xEF03     	
#define EF04		0xEF04     	
#define EF05		0xEF05
#define EF06		0xEF06
#define EF07		0xEF07
#define EF08		0xEF08
#define EF09		0xEF09
#define EF10		0xEF10
#define	EF11		0xEF11

#define NORMAL_INVOICE 		0x01
#define RETURN_INVOICE 		0x02
#define SPOIL_INVOICE 		0x03

#define NORMAL_DECLARE	 	0
#define DOWNTIME_DECLARE  	1
#define MONTH_DECLARE 	 	2
#define RE_DECLARE 			3

#define INSPECT_INV_DETAIL  0
#define INSPECT_DAILY_SUM   1
#define INSPECT_DECLARE     2
#define INSPECT_MOD_TIME    3

#define BY_PLU_NUM  1
#define BY_BARCODE  2
/* 
 * Global varibal 
 *  below varibal will often been used
 */
extern struct tax_sys_app_info   g_sys_app_info;
extern struct tax_sys_pin_record g_pin;
extern struct tax_sys_pin_record g_origin_pin;
extern struct machine_info_record g_mach_info;
extern struct tax_sys_protect_record g_protect_record;
extern struct tax_sys_config_record  g_sys_config;
extern struct tax_sys_fis_config_record  g_fis_config;
extern struct tax_sys_buy_roll_record  g_cur_roll;
extern struct tax_sys_cur_roll_left_record  g_cur_roll_left;

#define get_pin()	 		(&g_pin)
#define get_origin_pin()	(&g_origin_pin)
#define get_mach_info()     (&g_mach_info) 
#define get_protect_record() (&g_protect_record)
#define get_sys_config()  	(&g_sys_config)
#define get_fis_config()  	(&g_fis_config)
#define get_sys_app_info()  (&g_sys_app_info)
#define get_cur_roll()      (&g_cur_roll)
#define get_cur_roll_left() (&g_cur_roll_left) 

/* fiscal system functions type defination */
typedef int (* dist_invoice_t)(struct tax_sys_invoice_roll_record *);
typedef int (* mount_roll_t)(struct tax_sys_invoice_roll_record *);
typedef int (* daily_collect_t)(void);
typedef int (* issue_invoice_t)(struct tax_sys_issue_invoice *, struct issue_invoice_res *);
typedef int (* declare_duty_t)(int, struct bcd_date *);
typedef int (* redeclare_duty_t)(struct bcd_date *, struct bcd_date *);
typedef int (* check_finish_tax_t)(uchar *card_nb);
typedef int (* update_control_t)(void);
typedef int (* update_taxpayer_t)(void);
typedef int (* power_on_check_t)(void);
typedef int (* transact_prepare_t)(void);
typedef int (* fiscal_init_t)(void);
typedef int (* get_invoice_nb_t)(uint *);
typedef int (* get_buy_roll_t)(int *, struct tax_sys_buy_roll_record *);
typedef int (* is_fiscal_init_t)(void);
typedef int (* get_last_dec_date_t)(struct bcd_date *);
typedef int (* check_dec_date_t)(struct bcd_date *);
typedef int (* card_init_t)(void);
typedef int (* get_check_info_t)(struct tax_sys_check_info *);
typedef int (* verify_check_card_t)(uchar *);
typedef int (* write_check_declare_detail_t)(int, int , int);
typedef int (* write_check_daily_detail_t)(int, int);
typedef int (* write_check_invoice_detail_t)(int, int, int);

struct tax_system
{
	is_fiscal_init_t is_fiscal_init;
	get_invoice_nb_t get_invoice_nb;
    get_last_dec_date_t get_last_declare_date;
	check_dec_date_t check_declare_date;
    get_buy_roll_t get_buy_roll;
	dist_invoice_t dist_invoice;
	mount_roll_t mount_roll;
	daily_collect_t daily_collect;
	issue_invoice_t issue_invoice;
	declare_duty_t declare_duty;
	redeclare_duty_t redeclare_duty;
    check_finish_tax_t check_finish_tax;
	update_control_t update_control;
    update_taxpayer_t update_taxpayer;
	power_on_check_t power_on_check;
	transact_prepare_t transact_prepare;
    
    card_init_t card_init;    
    fiscal_init_t fiscal_init;

    get_check_info_t get_check_info;
    verify_check_card_t verify_check_card;
    write_check_declare_detail_t write_check_declare_detail;
    write_check_daily_detail_t write_check_daily_detail;
    write_check_invoice_detail_t write_check_invoice_detail;
};

extern struct tax_system * get_tax_system(void);

#endif /* __TAX_SYSTEM_HEAD__ */
