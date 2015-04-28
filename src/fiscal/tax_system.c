/*
 * Tax System Code
 *   -- Tax system core code
 * 
 * Author : Leonardo Physh 
 * Date   : 2014.7.30
 */

#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "config.h"
#include "error.h"
#include "power.h"
#include "tax_file_op.h"
#include "tax_card_base.h"
#include "tax_system.h"

/* 
 * Global varibals 
 *  below varibal will be used ofenly
 */
struct tax_sys_app_info  g_sys_app_info;
struct tax_sys_pin_record  g_pin;
struct tax_sys_pin_record  g_origin_pin;
struct machine_info_record g_mach_info;
struct tax_sys_config_record  g_sys_config;
struct tax_sys_fis_config_record  g_fis_config;
struct tax_sys_buy_roll_record g_cur_roll;
struct tax_sys_protect_record g_protect_record;
struct tax_sys_cur_roll_left_record  g_cur_roll_left;

/*
 * translate card used - convert data format between system and card 
 *  @sys_invoice : system format data 
 *  @card_invoice : card format data 
 *  @return : status 
 */
static int tax_trans_issue_invoice(int mode, struct tax_sys_issue_invoice * sys_invoice, 
        struct issue_invoice * card_invoice)
{
    uchar * ptr;
    uchar xor_value = 0;

    /*
     * MODE
     *   0 : system transfer to card 
     *   1 : card transfer to system 
     */
    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return FAIL;

sys_to_card:
    memcpy(card_invoice->date, &sys_invoice->date, 4);
    card_invoice->invoice_type = sys_invoice->invoice_type;
    end_cover_int(card_invoice->invoice_num, (uchar *)&sys_invoice->invoice_num);

    int i;
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        card_invoice->tax_type[i].index = sys_invoice->item[i].index;
        end_cover_int(card_invoice->tax_type[i].amount, (uchar *)&(sys_invoice->item[i].amount));    
    }

    end_cover_int(card_invoice->amt_total, (uchar *)&sys_invoice->amt_total);

    /* do xor sum */
    ptr = (uchar *)card_invoice;
    for (i = 0; i < sizeof(struct issue_invoice) - 1; i++) {
        xor_value ^= *ptr++;
    }

    card_invoice->xor_value = xor_value;

    return 0;

card_to_sys:
    memcpy(&sys_invoice->date, card_invoice->date, 4);
    sys_invoice->invoice_type = card_invoice->invoice_type;
    end_cover_int((uchar *)&sys_invoice->invoice_num, card_invoice->invoice_num);

    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        sys_invoice->item[i].index =card_invoice->tax_type[i].index;
        end_cover_int((uchar *)&(sys_invoice->item[i].amount), card_invoice->tax_type[i].amount);    
    }

    end_cover_int((uchar *)&sys_invoice->amt_total, card_invoice->amt_total);

    return 0;
}

/*
 * tax_trans_inspect_invoice - transalation helper function
 */
static int tax_trans_inspect_invoice(int mode, struct tax_sys_inspect_invoice *sys_inspect,
        struct inspect_invoice * card_inspect)
{
    /*
     * MODE
     *   0 : system transfer to card 
     *   1 : card transfer to system 
     */
    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return FAIL;

sys_to_card:
    card_inspect->invoice_type = sys_inspect->type;
    memcpy(card_inspect->date, &sys_inspect->date, 4);
    memcpy(card_inspect->fiscal_code, sys_inspect->fiscal_code, 20);
    end_cover_int(card_inspect->invoice_num, (uchar *)&sys_inspect->invoice_num);
    end_cover_int(card_inspect->ori_invoice_num, (uchar *)&sys_inspect->origin_inv_num);
    end_cover_int(card_inspect->amt_total, (uchar *)&sys_inspect->amt_total);

    return 0;

card_to_sys:
    sys_inspect->type = card_inspect->invoice_type;
    memcpy(&sys_inspect->date, card_inspect, 4);
    memcpy(sys_inspect->fiscal_code, card_inspect->fiscal_code, 20);
    end_cover_int((uchar*)&sys_inspect->invoice_num, card_inspect->invoice_num);
    end_cover_int((uchar*)&sys_inspect->origin_inv_num, card_inspect->ori_invoice_num);
    end_cover_int((uchar*)&sys_inspect->amt_total, card_inspect->amt_total);

    return 0;
}


/*
 * tax_trans_declare_duty - convert data format between system and card 
 *  @sys_declare_duty : system format data 
 *  @card_declare_duty : card format data 
 *  @res : card response data 
 *  @return : status   
 */
static int tax_trans_declare_duty(int mode, struct tax_sys_declare_duty_record * sys_declare_duty, 
        struct declare_duty * card_declare_duty, struct declare_res * res)
{
    uchar *ptr;
    uchar xor_value = 0;

    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return -1;

sys_to_card:
    memcpy(card_declare_duty->start_date, &sys_declare_duty->start_date, 4);
    memcpy(card_declare_duty->end_date, &sys_declare_duty->end_date, 4);

    end_cover_int(card_declare_duty->valid_count, (uchar *)&sys_declare_duty->valid_count);
    end_cover_short(card_declare_duty->return_count, (uchar *)&sys_declare_duty->return_count);
    end_cover_short(card_declare_duty->spoil_count, (uchar *)&sys_declare_duty->spoil_count);

    memcpy(card_declare_duty->tax_index, sys_declare_duty->tax_index, 6);

    int i;
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        end_cover_int(card_declare_duty->amt_valid[i].amount, (uchar *)&sys_declare_duty->amt_valid[i]);
        end_cover_int(card_declare_duty->amt_return[i].amount, (uchar *)&sys_declare_duty->amt_return[i]);
    }

    /* do xor sum */
    ptr = (uchar *)card_declare_duty;
    for (i = 0; i < sizeof(struct declare_duty) - 1; i++) {
        xor_value ^= *ptr++;
    }

    card_declare_duty->xor_value = xor_value;

    return 0;

card_to_sys:
    memcpy(&sys_declare_duty->start_date, card_declare_duty->start_date, 4);
    memcpy(&sys_declare_duty->end_date, card_declare_duty->end_date, 4);

    end_cover_int((uchar *)&sys_declare_duty->valid_count, card_declare_duty->valid_count);
    end_cover_short((uchar *)&sys_declare_duty->return_count, card_declare_duty->return_count);
    end_cover_short((uchar *)&sys_declare_duty->spoil_count, card_declare_duty->spoil_count);

    memcpy(sys_declare_duty->tax_index, card_declare_duty->tax_index, 6);

    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        end_cover_int((uchar *)&sys_declare_duty->amt_valid[i], card_declare_duty->amt_valid[i].amount);
        end_cover_int((uchar *)&sys_declare_duty->amt_return[i], card_declare_duty->amt_return[i].amount);
    }

    /* process res data */
    end_cover_int((uchar *)&sys_declare_duty->total_valid, res->total_valid);
    end_cover_int((uchar *)&sys_declare_duty->total_return, res->total_return);

    sys_declare_duty->status = res->status;
    memcpy(sys_declare_duty->mac1, res->mac1, 4);
    memcpy(sys_declare_duty->sign, res->sign, 128);

    return 0;
}

/*
 * tax_trans_data_collect - convert date between system and card 
 *  @system_date_collect : system format date 
 *  @card_date_collect : card format date 
 *  @return : status 
 */
static int tax_trans_data_collect(int mode, struct declare_duty * card_declare_duty,
        struct declare_res * card_declare_res,
        struct data_collect * card_data_collect)
{
    int i;
    uchar *ptr;
    uchar xor_value = 0;
    
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return -1;

sys_to_card:
    memcpy(card_data_collect->card_num, gp_app_info->fiscal_card_nb, 8);
    memcpy((void *)card_data_collect + 8, card_declare_duty, 70);
    memcpy((void *)card_data_collect + 78, card_declare_res, 141);

#if 0
    memcpy(card_data_collect->start_date, &sys_declare_duty->start_date, 4);
    memcpy(card_data_collect->end_date, &sys_declare_duty->end_date, 4);

    end_cover_int(card_data_collect->valid_count, (uchar *)&sys_declare_duty->valid_count);
    end_cover_short(card_data_collect->return_count, (uchar *)&sys_declare_duty->return_count);
    end_cover_short(card_data_collect->spoil_count, (uchar *)&sys_declare_duty->spoil_count);

    memcpy(card_data_collect->tax_index, sys_declare_duty->tax_index, 6);

    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        end_cover_int(card_data_collect->amt_valid[i].amount, (uchar *)&sys_declare_duty->amt_valid[i]);
        end_cover_int(card_data_collect->amt_return[i].amount, (uchar *)&sys_declare_duty->amt_return[i]);
    }
    
    card_data_collect->status = sys_declare_res->status;
    memcpy(card_data_collect->mac1, sys_declare_res->mac1, 4);
    memcpy(card_data_collect->sign, sys_declare_res->sign, 128);
#endif 

    /* do xor sum */
    ptr = (uchar *)card_data_collect;
    for (i = 0; i < sizeof(struct data_collect) - 1; i++) {
        xor_value ^= *ptr++;
    }

    card_data_collect->xor_value = xor_value;

    return 0;

card_to_sys:
    /*
     * don't need to convert data from card to system 
     */ 
    return 0;

}

/*
 * tax_trans_daily_collect - convert data format between system and card 
 *  @sys_daily_collect : system format data 
 *  @card_daily_collect : card format data 
 *  @res : data response data 
 *  @return : status   
 */
static int tax_trans_daily_collect(int mode, struct tax_sys_daily_collect_record * sys_daily_collect,
        struct daily_collect * card_daily_collect, struct daily_collect_res * res)
{
    uchar *ptr;
    uchar xor_value = 0;

    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return -1;

sys_to_card:
    memcpy(card_daily_collect->cur_date, &sys_daily_collect->cur_date, 4);
    end_cover_short(card_daily_collect->valid_count, (uchar *)&sys_daily_collect->valid_count);
    end_cover_short(card_daily_collect->return_count, (uchar *)&sys_daily_collect->return_count);
    end_cover_short(card_daily_collect->spoil_count, (uchar *)&sys_daily_collect->spoil_count);

    memcpy(card_daily_collect->tax_index, sys_daily_collect->tax_index, 6);

    int i;
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        end_cover_int(card_daily_collect->amt_valid[i].amount, (uchar *)&sys_daily_collect->amt_valid[i]);
        end_cover_int(card_daily_collect->amt_return[i].amount, (uchar *)&sys_daily_collect->amt_return[i]);
    }

    /* do xor sum */
    ptr = (uchar *)card_daily_collect;
    for (i = 0; i < sizeof(struct daily_collect)- 1; i++)
        xor_value ^= *ptr++;

    card_daily_collect->xor_value = xor_value;

    return 0;

card_to_sys:
    memcpy(&sys_daily_collect->cur_date, card_daily_collect->cur_date, 4);
    end_cover_short((uchar *)&sys_daily_collect->valid_count, card_daily_collect->valid_count);
    end_cover_short((uchar *)&sys_daily_collect->return_count, card_daily_collect->return_count);
    end_cover_short((uchar *)&sys_daily_collect->spoil_count, card_daily_collect->spoil_count);

    memcpy(sys_daily_collect->tax_index, card_daily_collect->tax_index, 6);

    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        end_cover_int((uchar *)&sys_daily_collect->amt_valid[i], card_daily_collect->amt_valid[i].amount);
        end_cover_int((uchar *)&sys_daily_collect->amt_return[i], card_daily_collect->amt_return[i].amount);
    }

    /* process res data */
    memcpy(sys_daily_collect->sign, res->sign, 128);

    return 0;
}

/*
 * tax_trans_invoice_roll - convert data format between system and card 
 *  @mode : convert mode 
 *  @sys_invoice_roll : system format data 
 *  @card_invoice_roll : card format data  
 *  @return : status 
 */
static int tax_trans_invoice_roll(int mode, struct tax_sys_invoice_roll_record * sys_invoice_roll,
        struct invoice_roll_info * card_invoice_roll)
{
    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return -1;

sys_to_card:
    memcpy(card_invoice_roll->invoice_code, sys_invoice_roll->invoice_code, 10);
    end_cover_int(card_invoice_roll->start_num, (uchar *)&sys_invoice_roll->start_num);
    end_cover_int(card_invoice_roll->end_num, (uchar *)&sys_invoice_roll->end_num);
    memcpy(card_invoice_roll->mac, sys_invoice_roll->mac, 4);

    return 0;

card_to_sys:
    memcpy(sys_invoice_roll->invoice_code, card_invoice_roll->invoice_code, 10);
    end_cover_int((uchar *)&sys_invoice_roll->start_num, card_invoice_roll->start_num);
    end_cover_int((uchar *)&sys_invoice_roll->end_num, card_invoice_roll->end_num);
    memcpy(sys_invoice_roll->mac, card_invoice_roll->mac, 4);

    return 0;
}

/*
 * tax_trans_buy_roll : convert data format between system and card 
 *  @sys_buy_roll : system format data 
 *  @card_buy_roll : card format data  
 *  @return : status 
 */
static int tax_trans_buy_roll(int mode, struct tax_sys_buy_roll_record * sys_buy_roll,
        struct buy_roll_info * card_buy_roll)
{
    if (mode == 0)
        goto sys_to_card;
    else if (mode == 1)
        goto card_to_sys;
    else 
        return -1;

sys_to_card:
    memcpy(card_buy_roll->invoice_code, sys_buy_roll->invoice_code, 10);
    end_cover_int(card_buy_roll->start_num, (uchar *)&sys_buy_roll->start_num);
    end_cover_int(card_buy_roll->end_num, (uchar *)&sys_buy_roll->end_num);
    end_cover_short(card_buy_roll->roll_num, (uchar *)&sys_buy_roll->roll_num);

    return 0;

card_to_sys:
    memcpy(sys_buy_roll->invoice_code, card_buy_roll->invoice_code, 10);
    end_cover_int((uchar *)&sys_buy_roll->start_num, card_buy_roll->start_num);
    end_cover_int((uchar *)&sys_buy_roll->end_num, card_buy_roll->end_num);
    end_cover_short((uchar *)&sys_buy_roll->roll_num, card_buy_roll->roll_num);

    return 0;        
}

/*
 *  is_fiscal_card_ready - Check fiscal is ready or not
 *   @return : card state
 */
static int is_fiscal_card_ready(void)
{
    int ret;
    uchar res;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 0, 1, &res);
    if (ret < 0)
        return ret;

    if (res == 0x00)
        return POSITIVE;

    return NEGATIVE;
}

/*
 *  is_fiscal_card_register - check fiscal card has been registered or not
 *   @return : card state
 */
static int is_fiscal_card_register(void)
{
    int ret;
    uchar res;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 1, 1, &res);
    if (ret < 0)
        return ret;

    if (res == 0x01)
        return NEGATIVE;

    return POSITIVE;
}

/*
 * fiscal_card_power_on - power on fiscal card and reset card 
 *  @return : status 
 */
static int fiscal_card_power_on(void)
{
    int ret; 
    struct fiscal_card *fiscal_card;

    fiscal_card = get_fiscal_card();

    ret = fiscal_card->power_on(FISCAL_CARD);
    if (ret < 0)
        return ret;

    ret = fiscal_card->power_rst(FISCAL_CARD);
    if (ret != SUCCESS) {
        fiscal_card->power_off(FISCAL_CARD);
        return ret;
    }

    return SUCCESS;
}

/*
 * Get the mechine number from fiscal card 
 *  @res : receive buffer 
 */
static int fiscal_card_get_machine_nb(uchar *res)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 10, 8, res);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_card_nb - read card number from fiscal card 
 *  @res : receive buffer
 *  @return : status 
 */
static int fiscal_card_get_card_nb(uchar *res)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 2, 8, res);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_taxpayer_nb - read taxpayer number from fiscal card
 *  @res : receive buffer 
 */
static int fiscal_card_get_taxpayer_nb(uchar *res)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 18, 8, res);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_invoice_info - read input invoice information
 *                                from fiscal card FS EFO5
 *   @inv_info : receive buffer 
 */
static int fiscal_card_get_invoice_info(struct input_invoice_info * inv_info)
{
    int ret;
    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF05);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 0, 19, (uchar *)inv_info);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_update_invoice_info - update binary content in card
 *  
 *  @inv_info : write-back data 
 *  @return : status 
 */
static int fiscal_card_update_invoice_info(struct input_invoice_info * inv_info)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF05);
    if (ret < 0)
        return ret;

    ret = fiscal_card->update_binary(FISCAL_CARD, 0, 19, (uchar *)inv_info);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_taxpayer_name - get taxpayer name from fiscal card 
 *  @buf : receive buffer
 *  @return : statsu 
 */
static int fiscal_card_get_taxpayer_name(uchar *buf)
{
    int ret;
    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 0x39, 40, buf);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_app_info - read application information 
 *                            from fiscal card.
 *  @app_info : receive buffer 
 */
static int fiscal_card_get_app_info(struct tax_sys_app_info * app_info)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card_get_card_nb(app_info->fiscal_card_nb);
    if (ret < 0)
        return ret;

    /*
     * EF02 : 
     *  refer to GB 18240.2-2003 appendix A.
     */
    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF02);
    if (ret < 0)
        return ret;

    /* application start date */
    ret = fiscal_card->read_binary(FISCAL_CARD, 0x2e, 4, (uchar *)&app_info->app_start_date);
    if (ret < 0)
        return ret;

    /* application vailable date */
    ret = fiscal_card->read_binary(FISCAL_CARD, 0x32, 4, (uchar *)&app_info->app_vaild_date);
    if (ret < 0)
        return ret;

    /* taxpayer name */
    ret = fiscal_card->read_binary(FISCAL_CARD, 0x39, 40, (uchar *)app_info->taxpayer_name);
    if (ret < 0)
        return ret;
    app_info->taxpayer_name[40] = '\0';

    /* taxpayer number*/
    ret = fiscal_card->read_binary(FISCAL_CARD, 18, 8, app_info->taxpayer_nb);
    if (ret < 0)
        return ret;

    /* taxpayer id*/ 
    ret = fiscal_card->read_binary(FISCAL_CARD, 26, 4, app_info->taxpayer_id);
    if (ret < 0)
        return ret;

    /* tax office code */
    ret = fiscal_card->read_binary(FISCAL_CARD, 97, 4, app_info->office_code);
    if (ret < 0)
        return ret;

    /* declare mode */
    ret = fiscal_card->read_binary(FISCAL_CARD, 101, 1, &app_info->declare_mode);
    if (ret < 0)
        return ret;

    /*
     * EF01 
     */
    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF01);
    if (ret < 0)
        return ret;

    /* issue invoice limit date */ 
    ret = fiscal_card->read_binary(FISCAL_CARD, 0, 4, (uchar *)&app_info->issue_limit_date);
    if (ret < 0)
        return ret;

    /* amount limit of single invoice */ 
    ret = fiscal_card->read_binary(FISCAL_CARD, 4, 4, (uchar *)&app_info->single_invoice_limit);
    if (ret < 0)
        return ret;

    /* need to end-cover*/
    ret = end_cover_int((uchar*)&app_info->single_invoice_limit, (uchar *)&app_info->single_invoice_limit);
    if (ret < 0)
        return ret;

    /* amount limit of total invoice */
    ret = fiscal_card->read_binary(FISCAL_CARD, 8, 4, (uchar *)&app_info->total_invoice_limit);
    if (ret < 0)
        return ret;

    /* need to end-cover */
    ret = end_cover_int((uchar *)&app_info->total_invoice_limit, (uchar *)&app_info->total_invoice_limit);
    if (ret < 0)
        return ret;

    /* amount limit of returen invoices */
    ret = fiscal_card->read_binary(FISCAL_CARD, 12, 4, (uchar *)&app_info->return_invoice_limit);
    if (ret < 0)
        return ret;

    /* need to end-cover */
    ret = end_cover_int((uchar *)&app_info->return_invoice_limit, (uchar *)&app_info->return_invoice_limit);
    if (ret < 0)
        return ret;

    /* detail mode */
    ret = fiscal_card->read_binary(FISCAL_CARD, 22, 1, &app_info->detail_mode);
    if (ret < 0)
        return ret;

    /* tax index number */
    ret = fiscal_card->read_binary(FISCAL_CARD, 16, MAX_USER_TAXRATE, app_info->tax_index);
    if (ret < 0)
        return ret;

    /*
     * move non-zero tax rate index to front of array
     */
    int i = 0, j = 0;
    uchar index_tmp[MAX_USER_TAXRATE];
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        if (app_info->tax_index[i] != 0) {
            index_tmp[j++] = app_info->tax_index[i];
        }
    }

    app_info->tax_item_nb = j;
    memset(app_info->tax_index, 0, MAX_USER_TAXRATE);
    memcpy(app_info->tax_index, index_tmp, app_info->tax_item_nb);

    /* Here don't initialize fis_type_num and fis_type */

    /* Done */
    return SUCCESS;
}

/*
 * fiscal_card_get_tax_index - read fiscal card and get tax rate index 
 *  @tax_index : receive buffer 
 *  @return : status 
 */
static int fiscal_card_get_tax_index(uchar *tax_index)
{
    int ret;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 16, 6, tax_index);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * fiscal_card_get_new_ctl_data() - update fiscal control data 
 *  @new_app : receive buffer 
 *  @return : status 
 */
static int fiscal_card_get_new_ctl_data(struct tax_sys_app_info * new_app)
{
    int ret;
    struct fis_ef01_record fis_ef01_rec;

    struct fiscal_card * fiscal_card;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card) 
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card->select_file_by_df(FISCAL_CARD, TSAM01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->select_file_by_id(FISCAL_CARD, EF01);
    if (ret < 0)
        return ret;

    ret = fiscal_card->read_binary(FISCAL_CARD, 0, 23, (uchar *)&fis_ef01_rec);
    if (ret < 0)
        return ret;

    /* limit date */
    memcpy(&new_app->issue_limit_date, fis_ef01_rec.issue_limit_date, 4);

    /* single invoice limit */
    end_cover_int((uchar *)&new_app->single_invoice_limit, fis_ef01_rec.single_invoice_limit);

    /* total limit */
    end_cover_int((uchar *)&new_app->total_invoice_limit, fis_ef01_rec.total_invoice_limit);

    /* return limit */
    end_cover_int((uchar *)&new_app->return_invoice_limit, fis_ef01_rec.return_invoice_limit);

    /* tax index */
    memcpy(new_app->tax_index, fis_ef01_rec.tax_index, 6);

    /* declare detail */
    memcpy(&new_app->detail_mode, &fis_ef01_rec.detail_mode, 1);

    /* tax rate */
    int i = 0, j = 0;
    uchar tmp_index[MAX_USER_TAXRATE];
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        if (new_app->tax_index[i] != 0)
            tmp_index[j++] = new_app->tax_index[i];
    }

    new_app->tax_item_nb = j;
    memset(new_app->tax_index, 0, MAX_USER_TAXRATE);
    memcpy(new_app->tax_index, tmp_index, new_app->tax_item_nb);

    return SUCCESS;
}

/*
 * is_user_card_ready - check user card is ready or not 
 *  @return : card state 
 */
static int is_user_card_ready(void)
{
    int ret;
    uchar res;

    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = user_card->read_binary(USER_CARD, 0, 1, &res);
    if (ret < 0)
        return ret;

    if (res == 0x01)
        return POSITIVE;

    return NEGATIVE;
}


/*
 * user_card_power_on - power on user card and reset card 
 *  @return : status 
 */
static int user_card_power_on(void)
{
    int ret; 
    struct user_card *user_card;

    user_card = get_user_card();

    ret = user_card->power_on(USER_CARD);
    if (ret < 0)
        return ret;

    ret = user_card->power_rst(USER_CARD);
    if (ret != SUCCESS) {
        user_card->power_off(USER_CARD);
        return ret;
    }

    return SUCCESS;
}

/*
 * user_card_chk_taxpayer_nb - read taxpayer number from user card 
 *  @taxpayer_nb - receive buufer 
 */
static int user_card_chk_taxpayer_nb(uchar * taxpayer_nb)
{
    int ret; 
    uchar res[8];

    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = user_card->read_binary(USER_CARD, 2, 8, res);
    if (ret < 0)
        return ret;

    if(memcmp(res, taxpayer_nb, 8) != 0) {
        return -ETAX_TAXPAYER_NB_NOT_MATCH;
    }

    return SUCCESS;
}

/*
 * user_card_get_taxpayer_name - get tax payer name from User Card 
 *  @return : receive buffer 
 *  @return : status 
 */
static int user_card_get_taxpayer_name(uchar *buf)
{
    int ret; 
    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = user_card->read_binary(USER_CARD, 21, 40, buf);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * user_card_get_mach_nb - read the cout of machines 
 *                         that belong to taxpayer.
 *  @num : receive buffer 
 */
static int user_card_get_mach_nb(uchar * num)
{
    int ret; 

    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = user_card->read_binary(USER_CARD, 1, 1, num);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/* user_card_get_mach_info - get machine information from user card 
 *  @index : record index number 
 *  @res : receive buffer 
 *  @retur : status 
 */
static int user_card_get_mach_info(int index, uchar *res)
{
    int ret; 
    uchar tmp[18];

    uchar rc_index = (uchar)index;

    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF06);
    if (ret < 0)
        return ret;

    ret = user_card->read_record(USER_CARD, rc_index, 18, tmp);
    if (ret < 0)
        return ret;

    memcpy(res, tmp, 18);

    return SUCCESS;
}

/*
 * user_card_get_tmp_dist - read tmp distribute record (EF07)
 * 
 * @inv_roll_rec : receive buffer 
 */
static int user_card_get_tmp_dist(struct tax_sys_invoice_roll_record * inv_roll_rec)
{
    int ret; 

    struct invoice_roll_info card_inv_info;

    struct user_card * user_card;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF07);
    if (ret < 0)
        return ret;

    ret = user_card->read_record(USER_CARD, 0, 22, (uchar *)&card_inv_info);
    if (ret < 0)
        return ret;

    /*
     * end cover: card to sys 
     */ 
    ret = tax_trans_invoice_roll(1, inv_roll_rec, &card_inv_info);
    if (ret < 0)
        return ret;

    return SUCCESS;

}

/*
 * user_card_get_fis_type - read fiscal type from user card 
 *  @app_info : receive buffer
 *  @return : status  
 */
static int user_card_get_fis_type(struct tax_sys_app_info * app_info)
{
    int ret; 
    char * ptr;
    uchar res[47];

    struct user_card * user_card;
    struct tax_sys_fiscal_type * fis_type;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF03);
    if (ret < 0)
        return ret;

    int i, count = 0;
    fis_type = app_info->fis_type;

    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        ret = user_card->read_record(USER_CARD, i + 1, 47, res);
        if (ret == SUCCESS) {
            if (res[0] != 0) {
                fis_type[count].index = res[0];
                memcpy(fis_type[count].item_code, res + 1, 4);
                memcpy(&fis_type[count].tax_rate, res + 5, 2);
                memcpy(fis_type[count].item_cn_name, res + 7, 20);
                memcpy(fis_type[count].item_en_name, res + 27, 20);

                /* tax rate need end cover */
                ret = end_cover_short((uchar *)&fis_type[count].tax_rate, 
                        (uchar *)&fis_type[count].tax_rate);
                if (ret < 0) 
                    return ret;

                count++;
            }     
        } else {            
            if (ret == -ETAX_NO_CARD_REC) {
                app_info->fis_type_num = count;
                return SUCCESS;
            }
            return ret;
        }
    }
    app_info->fis_type_num = count;
    
    /*
     * try to fix too many spaces behand item_cn_name & item_en_name
     */ 
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        if (fis_type[i].index == 0)
            continue;

        ptr = fis_type[i].item_cn_name;
        while (*ptr != ' ')
            ptr++;

        if (*(ptr + 1) == ' ')
            *ptr = '\0';

        ptr = fis_type[i].item_en_name;
        while (*ptr != ' ')
            ptr++;

        if (*(ptr + 1) == ' ')
            *ptr = '\0';
    }

    return SUCCESS;
}

/*
 * user_card_get_ctl_data - read control data from user card 
 *  @card_nb : fiscal card number 
 *  @res : receive buffer
 *  @return : status 
 */
static int user_card_get_ctl_data(uchar *card_nb, struct usr_ef01_record * res)
{
    int i, ret;
    uchar num;
    struct usr_ef01_record tmp_res;

    struct user_card * user_card = get_user_card();
    if (user_card == NULL)
        return -ETAX_NUL_USER_CARD;

    ret = user_card_get_mach_nb(&num);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF01);
    if (ret < 0)
        return ret;

    for (i = 0; i < num; i++) {
        ret = user_card->read_record(USER_CARD, i + 1, 37, (uchar *)&tmp_res);
        if (ret < 0)
            return ret;

        if (memcmp(card_nb, tmp_res.fiscard_card_nb, 8) == 0) {
            *res = tmp_res;
            return SUCCESS;
        }
    }

    return -ETAX_UC_NOT_MATCH;
}

/*
 * user_card_get_declare_record - read declare record from user card 
 *  @card_nb - fiscal card number 
 *  @buf - receive buffer 
 *  @return : SUCCESS / FAIL
 */
int user_card_get_declare_record(uchar * card_nb, uchar *buf)
{
    int ret;
    int i;

    struct user_card * user_card = get_user_card();
    if (user_card == NULL)
        return -ETAX_NUL_USER_CARD;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        return ret;

    ret = user_card->select_file_by_id(USER_CARD, EF04);
    if (ret < 0)
        return ret;
    
    /* 10: maximun of card declare record */
    for (i = 0; i < 10; i++) {
        ret = user_card->read_record(USER_CARD, i + 1, 220, buf);
        if (ret < 0)
            return ret;

        if (memcmp(card_nb, buf + 1, 8) == 0) {
            return SUCCESS;
        }
    }

    return FAIL;
}

/*
 *  user_card_check_card - check user card match system or not 
 *   @machine_nb : current machine number 
 *   @fis_card_nb : fiscal card number 
 *   @return : status 
 */
static int user_card_check_card(uchar *taxpayer_nb, uchar * machine_nb, uchar * fis_card_nb)
{
    int i, ret;
    uchar num, res[18];

    ret = user_card_chk_taxpayer_nb(taxpayer_nb);
    if (ret != SUCCESS)
        return ret;

    ret = user_card_get_mach_nb(&num);
    if (ret != SUCCESS)
        return ret;

    for(i = 0; i < num; i++){
        ret = user_card_get_mach_info(i + 1, res);
        if(ret != SUCCESS){
            return  ret;
        }

        if (!memcmp(machine_nb, res, 8) && !memcmp(fis_card_nb, res + 8, 8))
            return SUCCESS;
    }

    return -ETAX_UC_NOT_MATCH;
}

/*
 * is_user_card_ready - check admin card is ready or not 
 *  @return : card state 
 */
static int is_check_card_ready(void)
{
    int ret;
    uchar res;

    struct check_card * check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = check_card->read_binary(CHECK_CARD, 0x00, 1, &res);
    if (ret < 0)
        return ret;

    if (res == 0x02)
        return POSITIVE;

    return NEGATIVE;
}

/*
 * check_card_power_on - power on check card and reset card 
 *  @return : status 
 */
static int check_card_power_on(void)
{
    int ret; 
    struct check_card *check_card;

    check_card = get_check_card();

    ret = check_card->power_on(CHECK_CARD);
    if (ret < 0)
        return ret;

    ret = check_card->power_rst(CHECK_CARD);
    if (ret != SUCCESS) {
        check_card->power_off(CHECK_CARD);
        return ret;
    }

    return SUCCESS;
}

/*
 * check_card_get_ctl_data - read check card control data from card 
 *  @ctl_data : receive buffer 
 *  @return : status 
 */
static int check_card_get_ctl_data(uchar *ctl_data)
{
    int ret; 
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF01);
    if (ret < 0)
        return ret;

    ret = check_card->read_binary(CHECK_CARD, 0x00, 10, ctl_data);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * check_card_get_date_code - get valid date and authority code  
 *                            of check card.
 *  @valid_date : application valid date 
 *  @auth_code : authority code
 */
static int check_card_get_date_code(struct bcd_date *valid_date, uchar *auth_code)
{
    int ret; 
    uchar res[60] = {0};
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF02);
    if (ret < 0)
        return ret;

    ret = check_card->read_binary(CHECK_CARD, 0x00, 60, res);
    if (ret < 0)
        return ret;
    
    memcpy(auth_code, res + 1, 8);
    memcpy(valid_date, res + 13, 4);

    return SUCCESS;
}

/*
 * check_card_get_chk_rec_num - get data record number in check card 
 *  @num : return value
 */
static int check_card_get_chk_rec_num(uchar *num)
{
    int ret; 
    uchar res;
    struct check_card *check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF03);
    if (ret < 0)
        return ret;

    int low, high, mid;
    low = 1, high = 255;
    while (low < high) {
        mid = (low + high) / 2;
        ret = check_card->read_record(CHECK_CARD, mid, 1, &res);
        if (ret < 0) {
            if (ret != ETAX_NO_CARD_REC)
                return ret;
            else {
                high = mid - 1;
            }
        } else {
            if (res == 0 || res == 0xFF)
                high = mid -1;
            else 
                low = mid + 1;
        } 
    }
    
    ret = check_card->read_record(CHECK_CARD, low, 1, &res);
    if (ret < 0) {
        if (ret != ETAX_NO_CARD_REC)
            return ret;
        else {
            *num = low - 1;
        }
    } else {
        if (res == 0 || res == 0xFF)
            *num = low - 1;
        else 
            *num = low;
    } 

    return SUCCESS;
}

/*
 * check_card_get_total_rec_num - get total record number of check card 
 *  @num : total number 
 *  @return : status  
 */
static int check_card_get_total_rec_num(uchar *num)
{
    int ret; 
    uchar res;
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF03);
    if (ret < 0)
        return ret;

    int low, high, mid;
    low = 1, high = 255;
    while (low < high) {
        mid = (low + high) / 2;
        ret = check_card->read_record(CHECK_CARD, mid, 1, &res);
        if (ret < 0) {
            if (ret != ETAX_NO_CARD_REC)
                return ret;
            else {
                high = mid - 1;
            }
        } else {
            low = mid + 1;
        } 
    }

    ret = check_card->read_record(CHECK_CARD, low, 1, &res);
    if (ret < 0) {
        if (ret != ETAX_NO_CARD_REC)
            return ret;
        else {
            *num = low - 1;
        }
    } else {
        *num = low;
    } 

    return SUCCESS;
}

/*
 * check_card_write_chk_rec() - write a check record to card 
 *                 we donot used securty protect, no need mac 
 *  @rec_num : offset internal card 
 *  @sbuf: content of record
 */
static int check_card_write_chk_rec(uchar rec_num, uchar *sbuf)
{
    int ret; 
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF03);
    if (ret < 0)
        return ret;

    ret = check_card->update_binary(CHECK_CARD, rec_num, 233, sbuf);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * check_card_read_chk_rec - read a record from check card 
 *  @rec_num : offset internal card 
 *  @rbuf : receive buffer 
 */
static int check_card_read_chk_rec(uchar rec_num, uchar *rbuf)
{
    int ret; 
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card->select_file_by_df(CHECK_CARD, CHECK01);
    if (ret < 0)
        return ret;

    ret = check_card->select_file_by_id(CHECK_CARD, EF03);
    if (ret < 0)
        return ret;

    ret = check_card->read_record(CHECK_CARD, rec_num, 233, rbuf);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/*
 * check_card_authority_fc - authority check card by fiscal card 
 *  @key_id: the ID of authority file 
 */
static int check_card_authority_fc(uchar key_id)
{
    int ret;
    uchar buf[8] = {0};
    uchar res[8] = {0};
    struct check_card *check_card = get_check_card();
    struct fiscal_card *fiscal_card = get_fiscal_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;
    
    ret = check_card->get_challenge(CHECK_CARD, 8, res);
    if (ret != SUCCESS) 
        return ret;

    ret = fiscal_card->select_file_by_df(CHECK_CARD, TSAM01);
    if (ret != SUCCESS)
        return ret;
    
    ret = fiscal_card->internal_auth(FISCAL_CARD, 1, res, buf);
    if (ret != SUCCESS)
        return ret;

    ret = check_card->external_auth(CHECK_CARD, key_id, buf);
    if (ret != SUCCESS)
        return ret;
    
    return SUCCESS;
}

/*
 * check_card_verify - Verify check card by input pin 
 *  @pin_id : the ID of pin file 
 *  @pin_len : length of pin 
 *  @pin : content of pin 
 */
static int check_card_verify(uchar pin_id, uchar pin_len, uchar *pin)
{
    int ret; 
    struct check_card *check_card;
    check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;
 
    ret = check_card->card_verify(CHECK_CARD, pin_id, pin_len, pin);
    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}


/*
 * tax_base_get_rate_nb - get index belong to which tax rate item 
 *  @index : tax rate index 
 *  @return : status 
 */
static int tax_base_get_rate_nb(int index)
{
    int rate_nb;
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    for (rate_nb = 0; rate_nb < MAX_USER_TAXRATE; rate_nb ++) {
        if (index == gp_app_info->tax_index[rate_nb])
            return rate_nb;
    }

    return FAIL;    
}

/*
 * tax_base_last_declare_date - get the next day to last time do declare 
 *
 *  @date : receive buffer
 *  @return : status 
 */
static int tax_base_get_last_declare_date(struct bcd_date * date)
{
    int ret;

    struct rt_operate * rt_ops = get_rt_ops();

    struct bcd_date next_date;
    struct tax_sys_declare_duty_record declare_rec;
    memset(&declare_rec, 0, sizeof(declare_rec));

    ret = tax_file_read_last_record(DECLARE_DUTY_FILE, (uchar *)&declare_rec, 
            sizeof(declare_rec));
    if (ret < 0)
        return ret;

    next_date = declare_rec.end_date;
    ret = rt_ops->get_next_date(&next_date);
    if (ret < 0)
        return 0;

    *date = next_date;

    return SUCCESS;
}

/* 
 * tax_base_check_date - check current date 
 *     app_start_date <= init_date <= issue_limit_date <= app_vaild_date
 *  @app_info : include date need to be check 
 *  @return : status 
 */
static int tax_base_check_date(struct tax_sys_app_info * app_info)
{
    int ret;
    struct rt_operate * rt_ops = get_rt_ops();

    /* application start date and init date */
    ret = rt_ops->cmp_bcd_date(&app_info->app_start_date, &app_info->init_date);
    if (ret > 0)
        return -(ETAX_DATE_CONFUSE);

    /* init date and limit date */
    ret = rt_ops->cmp_bcd_date(&app_info->init_date, &app_info->issue_limit_date);
    if (ret > 0)
        return -(ETAX_DATE_CONFUSE);

    /* limit date application valid date */
    ret = rt_ops->cmp_bcd_date(&app_info->issue_limit_date, &app_info->app_vaild_date);
    if (ret > 0)
        return -(ETAX_DATE_CONFUSE);

    return SUCCESS;
}

/**
 * tax_base_check_lock_date - check date, machine should be lock down 
 *                            after 5 years as GB said.
 *  @date : date need to check 
 *  @return : days between date and lock date 
 */
static int tax_base_check_lock_date(struct bcd_date *date)
{
    int ret;
    uint jd, jd_lock;
    uint year, mon, day;

    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    bcd_to_greg(date, &year, &mon, &day);
    jd = greg_to_julian(year, mon, day);

    bcd_to_greg(&gp_app_info->init_date, &year, &mon, &day);
    jd_lock = greg_to_julian(year, mon, day);
    jd_lock += MACHINE_LOCK_DATE; 

    ret = jd_lock - jd;

    return ret;           
}

/*
 * tax_base_check_declare_date - check date is available 
 *                               declare date or not, if 
 *                               yes, do nothing or return 
 *                               new available date 
 *  @date : checking date
 *  @return : status  
 */
static int tax_base_check_declare_date(struct bcd_date *date) 
{
    int ret;
    int days;

    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    ret = rt_ops->cmp_bcd_date(date, &gp_app_info->issue_limit_date);
    if (ret > 0)
        *date = gp_app_info->issue_limit_date;

    /*
     * if end_date after lock date, set end_date to date that 
     * one day befor lock date
     */
    days = tax_base_check_lock_date(date);
    if (days <= 0) {
        days = -days;
        sub_bcd_date(date, (uint)days + 1);  
    }

    return SUCCESS;
}

#ifdef CONFIG_CHECK_DEADLINE
/**
 * tax_base_check_deadline - check data, today should after produce date 
 *  @today : date of now 
 */
static int tax_base_check_deadline(struct bcd_date *today)
{
    return SUCCESS;
}
#endif 

/*
 * tax_base_invoice_check - check invoice left number 
 *                          and distribute information
 * @retur : system state 
 */
static int tax_base_invoice_check(void)
{ 
    int ret;
    int inv_num;

    struct tax_sys_protect_record prot_rec;

    struct tax_sys_cur_roll_left_record * gp_cur_roll_left = get_cur_roll_left();

    if (gp_cur_roll_left->cur_roll_left < 1) {
        ret = tax_file_is_empty(PROTECT_FILE);
        if (ret == POSITIVE) {
            return -ETAX_INVOICE_EMPTY;
        } else if (ret == NEGATIVE){
            memset(&prot_rec, 0, sizeof(prot_rec));

            ret = tax_file_read_protect(&prot_rec);
            if (ret < 0)
                return ret;

            inv_num = prot_rec.invoice_info.end_num - prot_rec.invoice_info.start_num;
            if (inv_num > 0)
                return -ETAX_INVOICE_NOT_MOUNT;
            else 
                return -ETAX_INVOICE_EMPTY;
        } else 
            return ret;
    }

    return SUCCESS;
}

/*
 * tax_base_verify_check_card - API for upper module to verify check card 
 *  @pin : input pin 
 *  @return : status
 */
static int tax_base_verify_check_card(uchar *pin)
{
    int ret;
    struct check_card * check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card_power_on();
    if (ret < 0) 
        return ret;

    ret = is_check_card_ready();
    if (ret != POSITIVE)
        goto err;

    ret = check_card_verify(0x01, 8, pin);
    if (ret != SUCCESS)
        goto err;
    
    ret = SUCCESS;
err:
    check_card->power_off(CHECK_CARD);
    return ret;
}


/*
 * tax_base_get_check_info - get fiscal check information 
 *  @chk_info : receive buffer
 */
static int tax_base_get_check_info(struct tax_sys_check_info * chk_info)
{
    int ret;
    struct check_card * check_card = get_check_card();

    if (!check_card)
        return -ETAX_NUL_CHECK_CARD;

    ret = check_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_check_card_ready();
    if (ret != POSITIVE)
        goto err;
    
    ret = check_card_get_ctl_data((uchar *)chk_info);
    if (ret < 0)
        goto err;
    
    ret = check_card_get_date_code(&chk_info->valid_date, chk_info->auth_code);
    if (ret < 0)
        goto err;

    ret = check_card_get_chk_rec_num(&chk_info->rec_num);
    if (ret < 0)
        goto err;

    ret = check_card_get_total_rec_num(&chk_info->total_num);
    if (ret < 0)
        goto err;

    ret = SUCCESS;
err:
    check_card->power_off(CHECK_CARD);
    return ret;
}


/*
 * tax_sys_is_fiscal_init - check fical function is initialzed 
 *                           or not. 
 *  @return : system state
 */
static int tax_sys_is_fiscal_init(void)
{
    int ret; 

    struct tax_sys_config_record sys_cfg_rec;

    FILE *fp;

    /* check SYS_CFG_FILE is exsit or note */
    fp = fopen(SYS_CFG_FILE, "rb");
    if (fp == NULL) {
        return NEGATIVE;
    }

    fclose(fp);

    memset(&sys_cfg_rec, 0, sizeof(sys_cfg_rec));

    ret = tax_file_read_sys_cfg(&sys_cfg_rec);
    if (ret != SUCCESS) {
        return FAIL;
    }

    if (sys_cfg_rec.is_init != POSITIVE) {
        return NEGATIVE;
    }

    return POSITIVE;
}

/*
 * tax_sys_get_invoice_nb - get current invoice number 
 *  @inv_nb : receive buffer 
 */
static int tax_sys_get_invoice_nb(uint *inv_nb)
{
    struct tax_sys_buy_roll_record * gp_cur_roll = get_cur_roll();
    struct tax_sys_cur_roll_left_record *gp_cur_roll_left = get_cur_roll_left();

    if (gp_cur_roll_left->cur_roll_left == 0)
        return -ETAX_INVOICE_MC_EMPTY;

    *inv_nb = gp_cur_roll->end_num - gp_cur_roll_left->cur_roll_left + 1;

    return SUCCESS;
}

/*
 * tax_base_get_buy_roll - get buyed invoice roll information
 *  @roll_num : count of roll 
 *  @roll_rec : detail information of roll 
 *  @return : status 
 */
static int tax_sys_get_buy_roll(int *roll_num, struct tax_sys_buy_roll_record * roll_rec)
{
    int ret;
    uint index;

    struct user_card *user_card = get_user_card(); 
    struct machine_info_record *gp_mach_info = get_mach_info();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    struct buy_roll_info card_buy_roll;
    memset(&card_buy_roll, 0, sizeof(card_buy_roll));

#ifdef CONFIG_DEBUG
    assert(user_card != NULL);
    assert(roll_num != NULL && roll_rec != NULL);
#endif

    ret = user_card_power_on();
    if (ret < 0)
        goto card_off;

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
        goto card_off;
    }

    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb, gp_app_info->fiscal_card_nb);
    if (ret < 0) {
        ret = -ETAX_UC_NOT_MATCH;
        goto card_off;
    }

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret < 0)
        goto card_off;

    ret = user_card->select_file_by_id(USER_CARD, EF05);
    if (ret < 0)
        goto card_off;

    *roll_num = 0;
    for (index = 0; index < 10; index ++) {
        ret = user_card->read_record(USER_CARD, index + 1, 
                sizeof(card_buy_roll), (uchar *)&card_buy_roll);
        if (ret < 0) {
            if (ret == -ETAX_NO_CARD_REC) { 
                ret = -ETAX_INVOICE_EMPTY; 
                break;
            } else 
                goto card_off;
        }

        if (card_buy_roll.roll_num[0] == 0 && card_buy_roll.roll_num[1] == 0) {
            ret = -ETAX_INVOICE_EMPTY;
            break;
        }

        ret = tax_trans_buy_roll(1, &roll_rec[index], &card_buy_roll);
        if (ret != SUCCESS)
            goto card_off;

        (*roll_num) += 1;
    }

    if (*roll_num > 0)
        ret = SUCCESS;

card_off:
    user_card->power_off(USER_CARD);

    return ret;    
}

/*
 * tax_sys_dist_invoice - distribute a invoice roll 
 *                        only distribute one roll once 
 *
 * @inv_roll_rec : receive buffer 
 */
static int tax_sys_dist_invoice(struct tax_sys_invoice_roll_record * inv_roll_rec)
{
    int ret;
    int err_flag; 

    struct fiscal_card *fiscal_card;
    struct user_card *user_card;

    struct tax_sys_invoice_roll_record tmp_inv_roll;
    struct tax_sys_invoice_roll_record last_dist_record;
    struct tax_sys_protect_record prot_rec;

    struct tax_sys_fis_config_record * gp_fis_cfg = get_fis_config();
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();

    memset(&tmp_inv_roll, 0, sizeof(tmp_inv_roll));
    memset(&last_dist_record, 0, sizeof(last_dist_record));
    memset(&prot_rec, 0, sizeof(prot_rec));

#if CONFIG_DEBUG
    assert(inv_roll_rec != NULL);
    debug_msg("DOING DISTRIBUTE INVOICE\n");
#endif 

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
        goto card_off;
    }  
    ret = user_card_check_card(gp_app_info->taxpayer_nb, 
            gp_mach_info->machine_nb, gp_app_info->fiscal_card_nb);
    if (ret < 0) {
        goto card_off;
    }

    err_flag = 1;

    /*
     * has error last time distribute? 
     */
    if (gp_fis_cfg->dist_err_flag == 1) {
        ret = tax_file_is_full(DIST_ROLL_FILE);
        if (ret == POSITIVE) {
            ret = -ETAX_INVOICE_FULL;
            goto card_off;
        }

        ret =  user_card_get_tmp_dist(&tmp_inv_roll);
        if (ret < 0)
            goto card_off;

        /*
         * something interrupt last time distribute progress
         */
        if (tmp_inv_roll.start_num != 0) {
            ret = tax_file_is_empty(LAST_DIST_FILE);
            if (ret == POSITIVE) {
                /* 
                 * distribute progress is normal, but machine did not save 
                 * info in time, then save information 
                 */
                ret = tax_file_save_last_dist(&tmp_inv_roll);
                if (ret < 0)
                    goto card_off;

                /* 
                 * 'cause just distribute 1 invoice roll 
                 * so distribute done in this case 
                 */
                *inv_roll_rec = tmp_inv_roll;  
                err_flag = 0; 

                goto dist_done;
            } else {
                /* 
                 * machine has saved info 
                 */
                ret = tax_file_read_last_dist(&last_dist_record);
                if (ret < 0)
                    goto card_off;
                /*
                 * info in local file is not correct, 
                 * save local file as info in card.
                 */
                ret = memcmp(&tmp_inv_roll, &last_dist_record, sizeof(tmp_inv_roll));
                if (ret != 0) {
                    ret = tax_file_save_last_dist(&tmp_inv_roll);
                    if (ret < 0)
                        goto card_off;

                    *inv_roll_rec = tmp_inv_roll;
                    err_flag = 0; 

                    goto dist_done;        
                } else {
                    /*
                     * last time distribute progress is normal
                     * clear error flag  
                     */
                    gp_fis_cfg->dist_err_flag = 0;
                    ret = tax_file_save_fis_cfg(gp_fis_cfg);
                    if (ret < 0)
                        goto card_off;
                }
            }
        } else {
            /* tmp_inv_roll.start_num ==0 */
            gp_fis_cfg->dist_err_flag = 0;
            ret = tax_file_save_fis_cfg(gp_fis_cfg);
            if (ret < 0)
                goto card_off; 
        }
    }

    /*
     * distribute progress 
     * NOTICE: only distribute 1 invoice roll here 
     */
#if CONFIG_PWR_OFF_PROTECT
    struct power_state *pm = get_power_state();
    pm->check_power_state();
#endif 

    ret = tax_file_is_full(DIST_ROLL_FILE);
    if (ret == POSITIVE) {
        ret =  -ETAX_INVOICE_FULL;
        goto dist_done;
    }
    else if (ret != NEGATIVE) 
        goto dist_done;

    struct invoice_roll_info card_invoice_roll;
    ret = user_card->distribute_invoice_nb(&card_invoice_roll);
    if (ret != SUCCESS) {
        gp_fis_cfg->dist_err_flag = 1;
        ret = tax_file_save_fis_cfg(gp_fis_cfg);
        if (ret < 0)
            goto card_off;
    } 

    /*
     * Done, update local file
     */
    ret = tax_trans_invoice_roll(1, inv_roll_rec, &card_invoice_roll);
    if (ret < 0) 
        goto dist_done;

    ret = tax_file_save_last_dist(inv_roll_rec);
    if (ret < 0)
        goto dist_done;

    ret = tax_file_append_dist_roll(inv_roll_rec);
    if (ret < 0)
        goto dist_done;

    err_flag = 0;

dist_done:
    gp_fis_cfg->dist_err_flag = err_flag;
    ret = tax_file_save_fis_cfg(gp_fis_cfg);
    if (ret < 0)
        goto card_off;

    if (ret >= 0)
        ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(FISCAL_CARD);

    return ret;
}


#if 0 
/**
 * tax_sys_dist_mul_invoice - distribute multiple invoice rolls 
 *  @count : count of invoice rolls 
 *  @inv_roll_recs : receive buffer  
 *  @return : count of rolls distributed 
 */
static int tax_sys_dist_mul_invoice(int count, struct tax_sys_invoice_roll_record * inv_roll_recs)
{
    int ret;

#if CONFIG_DEBUG
    assert(inv_roll_recs != NULL);
#endif  

    /*
     * you should ensure inv_roll_recs will not overflow 
     */
    int i;
    for (i = 0; i < count; i++) {
        ret = tax_sys_dist_invoice(inv_roll_recs);
        if (ret < 0) {
            err_num = ret;
            break;
        }

        inv_roll_recs ++; 
    }

    return i;
}
#endif 

/**
 * tax_sys_mount_roll - mount a roll of invoice 
 *  @inv_info : output of DISTRIBUTE_INVOICE_NB
 *  @return : status 
 */
static int tax_sys_mount_roll(struct tax_sys_invoice_roll_record * inv_roll_rec)
{
    int ret; 
    struct fiscal_card *fiscal_card;
    struct user_card * user_card;

    struct tax_sys_pin_record *gp_pin = get_pin();

#if CONFIG_DEBUG
    assert(inv_roll_rec != NULL);
    debug_msg("DOING MOUNT ROLL\n");
#endif 

    ret = SUCCESS;

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

#if CONFIG_PWR_OFF_PROTECT
    struct power_state * pm = get_power_state();
    pm->check_power_state();
#endif     

    /*
     * Verify PIN
     */
    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin((uchar *)gp_pin, (uchar *)&new_pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

    /*
     * Input invoice number 
     */
    struct invoice_roll_info card_roll_info;
    ret = tax_trans_invoice_roll(0, inv_roll_rec, &card_roll_info);
    if (ret < 0)
        return ret;


    ret = fiscal_card->input_invoice_nb(&card_roll_info);
    if (ret < 0)
        goto card_off;

#if 0
    /* 
     * Clear PROTECT_FILE in upper layer 
     */
    ret = tax_file_clear(PROTECT_FILE);
    if (ret < 0)
        goto card_off;
#endif 

    struct tax_sys_buy_roll_record new_cur_roll;
    memset(&new_cur_roll, 0, sizeof(new_cur_roll));

    memcpy(new_cur_roll.invoice_code, inv_roll_rec->invoice_code, 10);
    new_cur_roll.start_num = inv_roll_rec->start_num;
    new_cur_roll.end_num = inv_roll_rec->end_num;

    struct tax_sys_cur_roll_left_record new_crln; 
    new_crln.cur_roll_left = new_cur_roll.end_num - new_cur_roll.start_num;

    /*
     * Update CUR_ROLL_FILE & CRLN_FILE
     */
    ret = tax_file_save_cur_roll(&new_cur_roll);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_crln(&new_crln);
    if (ret < 0)
        goto card_off;

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}


/*
 * tax_sys_cur_roll_collect - current roll data collect 
 *  
 *  @return : status 
 */
static int tax_sys_cur_roll_collect(void)
{
    int ret;
    int rec_count;

    struct tax_sys_buy_roll_record *gp_cur_roll = get_cur_roll();

    struct tax_sys_cur_roll_id_record cur_roll_rec;
    struct tax_sys_used_roll_id_record used_roll_rec;

    memset(&cur_roll_rec, 0, sizeof(cur_roll_rec));
    memset(&used_roll_rec, 0, sizeof(used_roll_rec));

    /* initialzed */
    used_roll_rec.start_num = gp_cur_roll->start_num;
    used_roll_rec.end_num = gp_cur_roll->end_num;
    memcpy(used_roll_rec.invoice_code, gp_cur_roll->invoice_code, 10);

    ret = tax_file_read_cur_roll_id(1, &cur_roll_rec);
    if (ret < 0)
        return ret;

    used_roll_rec.start_date = cur_roll_rec.date;

    ret = tax_file_read_last_record(ROLL_ID_FILE, (uchar *)&cur_roll_rec,
            sizeof(cur_roll_rec));
    if (ret < 0)
        return ret;

    used_roll_rec.end_date = cur_roll_rec.date;

    rec_count = tax_file_get_rec_num(ROLL_ID_FILE);
    if (rec_count < 0)
        return ret;

    int i;
    for (i = 0; i < rec_count; i++) {
        ret = tax_file_read_cur_roll_id(i + 1, &cur_roll_rec);
        if (ret < 0)
            return ret;

        switch (cur_roll_rec.type) {
            case NORMAL_INVOICE:
                used_roll_rec.amt_valid += cur_roll_rec.amout_total;
                used_roll_rec.valid_count ++;
                break;

            case RETURN_INVOICE:
                used_roll_rec.amt_return += cur_roll_rec.amout_total;
                used_roll_rec.return_count ++;
                break;

            case SPOIL_INVOICE:
                used_roll_rec.spoil_count ++;
                break;

            default:
                return -ETAX_UNKNOW_ERR;
                break;
        }
    }

    ret = tax_file_append_used_roll_id(&used_roll_rec);
    if (ret < 0)
        return ret;

    ret = tax_file_clear(ROLL_ID_FILE);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/**
 * tax_sys_nul_daily_collects - do daily collect for assign day 
 *
 *  @date : date need to do daily collect 
 *  @return : status 
 */
static int tax_sys_nul_daily_collect(struct bcd_date * date)
{
    int ret;

    struct fiscal_card * fiscal_card;
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();
    struct tax_sys_pin_record *gp_pin = get_pin();
    struct rt_operate * rt_ops = get_rt_ops();

#ifdef CONFIG_DEBUG
    assert(date != NULL);
#else 
    if (date == NULL)
        return FAIL;
#endif 

    debug_msg("[TAX_SYSTEM]: null daily collect\n");

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    struct daily_collect card_dc_record;
    struct daily_collect_res card_dc_res;
    struct tax_sys_daily_collect_record dc_record;
    struct tax_sys_daily_collect_record last_dc_rec;

    memset(&card_dc_res, 0, sizeof(card_dc_res));
    memset(&card_dc_record, 0, sizeof(card_dc_record));
    memset(&dc_record, 0, sizeof(dc_record));

    if (tax_file_is_empty(DAILY_COLLECT_FILE) == NEGATIVE) {
        ret = tax_file_read_last_record(DAILY_COLLECT_FILE, (uchar *)&last_dc_rec, 
                sizeof(struct tax_sys_daily_collect_record));
        if (ret < 0)
            return ret;

        if (rt_ops->cmp_bcd_date(&last_dc_rec.cur_date, date) == 0)
            return SUCCESS;
    }

    dc_record.cur_date = *date;
    memcpy(dc_record.tax_index, gp_app_info->tax_index, MAX_USER_TAXRATE);

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state *pm = get_power_state();

    pm->check_power_state();
#endif
    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

    /* sys to card */
    ret = tax_trans_daily_collect(0, &dc_record, &card_dc_record, NULL);
    if (ret != SUCCESS)
        goto card_off;

#if CONFIG_PWR_OFF_PROTECT
    pm->check_power_state();
#endif 

    /* preverion send CMD for 5 times ?? */
    ret = fiscal_card->daily_collect_sign(&card_dc_record, &card_dc_res);
    if (ret < 0)
        goto card_off;

    /* card to sys */ 
    memset(&dc_record, 0, sizeof(dc_record));
    ret = tax_trans_daily_collect(1, &dc_record, &card_dc_record, &card_dc_res);
    if (ret < 0)
        goto card_off;

    ret = tax_file_append_daily_collect(&dc_record);
    if (ret < 0)
        goto card_off;

    debug_msg("[TAX_SYSTEM] : nul daily collect done\n");

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);

    return ret;
}

/*
 * tax_sys_collect_proc - do daily collect and save record 
 *  @dc_record : the data to collect and sava 
 *  @return : status 
 */
static int tax_sys_collect_proc(struct tax_sys_daily_collect_record * dc_record)
{
    int ret;
    int i, rate_nb;

    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct rt_operate *rt_ops = get_rt_ops();

    struct tax_sys_pin_record *gp_pin = get_pin();

    struct daily_collect card_dc_record;
    struct daily_collect_res card_dc_res;
    struct tax_sys_daily_collect_record last_dc_record;

    debug_msg("[TAX_SYSTEM]: Daily Collect Proc...\n");

    memset(&card_dc_record, 0, sizeof(card_dc_record));
    memset(&card_dc_res, 0, sizeof(card_dc_res));
    memset(&last_dc_record, 0, sizeof(last_dc_record));

    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    ret = tax_file_read_last_record(DAILY_COLLECT_FILE, (uchar *)&last_dc_record,
            sizeof(struct tax_sys_daily_collect_record));
    if (ret < 0)
        return ret;

    /* if today did daily collect before ?*/ 
    if (rt_ops->cmp_bcd_date(&dc_record->cur_date, &last_dc_record.cur_date) == 0) {
        /* merge daily collect data */ 
        dc_record->valid_count += last_dc_record.valid_count;
        dc_record->return_count += last_dc_record.return_count;
        dc_record->spoil_count += last_dc_record.spoil_count;

        for (i = 0; i < MAX_USER_TAXRATE; i++){
            if (last_dc_record.tax_index[i] != 0){
                rate_nb = tax_base_get_rate_nb(last_dc_record.tax_index[i]);
                dc_record->amt_valid[rate_nb] += last_dc_record.amt_valid[i];
                dc_record->amt_return[rate_nb] += last_dc_record.amt_return[i];
            }
        }
    }

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state * pm = get_power_state();
    pm->check_power_state();
#endif 

    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        return ret;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        return ret;

    ret = tax_trans_daily_collect(0, dc_record, &card_dc_record, NULL);
    if (ret != SUCCESS)
        return ret;

#ifdef CONFIG_PWR_OFF_PROTECT 
    pm->check_power_state();
#endif 

    ret = fiscal_card->daily_collect_sign(&card_dc_record, &card_dc_res);
    if (ret < 0)
        return ret;

    ret = tax_trans_daily_collect(1, dc_record, &card_dc_record, &card_dc_res);
    if (ret < 0)
        return ret;

    /* save local file record */
    if (rt_ops->cmp_bcd_date(&dc_record->cur_date, &last_dc_record.cur_date) == 0) {
        ret = tax_file_modify_last_record(DAILY_COLLECT_FILE, (uchar *)dc_record,
                sizeof(struct tax_sys_daily_collect_record));
        if (ret < 0)
            return ret;
    } else {
        ret = tax_file_append_daily_collect(dc_record);
        if (ret < 0)
            return ret;
    } 

    debug_msg("[TAX_SYSTEM]: Daily Collect Proc Done\n");

    return SUCCESS;
}

/**
 * tax_sys_dc_for_today - do daily collect for today 
 *
 *  @today : assign date to do daily collect(avoid to 23:59:59) 
 *  @return : status 
 */
static int tax_sys_dc_for_today(struct bcd_date * today)
{
    int ret;

    struct bcd_date tmp_date;
    struct fiscal_card * fiscal_card; 
    struct rt_operate *rt_ops = get_rt_ops();
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    struct tax_sys_today_id_record today_id;
    struct tax_sys_daily_collect_record dc_record;
    memset(&today_id, 0, sizeof(today_id));
    memset(&dc_record, 0, sizeof(dc_record));

#ifdef CONFIG_DEBUG
    assert(today != NULL);
#else 
    if (today == NULL)
        return FAIL; 
#endif

    debug_msg("[TAX_SYSTEM]: Daily Collect for today.\n");

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state *pm = get_power_state();

    pm->check_power_state();
#endif

    /* 
     * read all today invoice detail record 
     * from TODAY_ID_FILE.
     */
    /* first record */
    ret = tax_file_read_today_id(1, &today_id);
    if (ret < 0)
        goto card_off;

    tmp_date = today_id.date;
    dc_record.cur_date = tmp_date;
    memcpy(dc_record.tax_index, gp_app_info->tax_index, MAX_USER_TAXRATE);

    int rec_count;
    rec_count = tax_file_get_rec_num(TODAY_ID_FILE);
    if (rec_count < 0)
        goto card_off;

    int i, j, rate_nb;
    for (i = 0; i < rec_count; i++) {
        ret = tax_file_read_today_id(i + 1, &today_id);
        if (ret < 0)
            goto card_off; 

        ret = rt_ops->cmp_bcd_date(&today_id.date, &tmp_date);
        if (ret > 0) {
            ret = tax_sys_collect_proc(&dc_record);
            if (ret != SUCCESS)
                goto card_off;
#if 0
            ret = rt_ops->get_next_date(&tmp_date);
            if (ret < 0)
                goto card_off;
#endif 
            tmp_date = today_id.date;

            memset(&dc_record, 0, sizeof(dc_record));
            dc_record.cur_date = tmp_date;
            memcpy(dc_record.tax_index, gp_app_info->tax_index, MAX_USER_TAXRATE);
        }

        switch (today_id.type) {
            case NORMAL_INVOICE:
                dc_record.valid_count ++;
                for (j = 0; j < MAX_USER_TAXRATE; j++) {
                    if (today_id.item[j].index != 0) {
                        rate_nb = tax_base_get_rate_nb(today_id.item[j].index);
                        dc_record.amt_valid[rate_nb] += today_id.item[j].amount;
                    }    
                }
                break;

            case RETURN_INVOICE:
                dc_record.return_count ++;
                for (j = 0; j < MAX_USER_TAXRATE; j++) {
                    if (today_id.item[j].index != 0) {
                        rate_nb = tax_base_get_rate_nb(today_id.item[j].index);
                        dc_record.amt_return[rate_nb] += today_id.item[j].amount;
                    }    
                }
                break;

            case SPOIL_INVOICE:
                dc_record.spoil_count ++;
                break;

            default:
                break;
        }
    }

    ret = tax_sys_collect_proc(&dc_record);
    if (ret < 0)
        goto card_off;

    ret = tax_file_clear(TODAY_ID_FILE);
    if (ret < 0)
        goto card_off;

    debug_msg("[TAX_SYSTEM]: Daily Collect for today done.\n");

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);

    return ret;
}

/**
 * tax_sys_daily_collect - do daily collect 
 *  
 *  @return :status 
 */
static int tax_sys_daily_collect(void)
{
    int ret;

    struct rt_operate * rt_ops = get_rt_ops();

    struct bcd_date today;
    struct bcd_date tmp_date;

    struct bcd_date *last_dc_date = NULL;
    struct bcd_date *last_issue_date = NULL;

    struct tax_sys_daily_collect_record last_dc_rec;
    struct tax_sys_today_id_record last_issue_rec;

    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    debug_msg("[TAX SYSTEM] : daily collect start...\n");

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    if (tax_file_is_empty(DAILY_COLLECT_FILE) == NEGATIVE) {
        ret = tax_file_read_last_record(DAILY_COLLECT_FILE, (uchar *)&last_dc_rec, 
                sizeof(struct tax_sys_daily_collect_record));
        if (ret < 0)
            return ret;

        last_dc_date = &last_dc_rec.cur_date;
    }

    if (tax_file_is_empty(TODAY_ID_FILE) == NEGATIVE) {
        ret = tax_file_read_today_id(1, &last_issue_rec);
        if (ret < 0)
            return ret;

        last_issue_date = &last_issue_rec.date;
    }

    /*
     * if machine haven't do daily_collect since 
     * it been fiscal initialzed.
     */
    if (last_dc_date == NULL) {
        tmp_date = gp_app_info->init_date;
        while (1) {
            if (rt_ops->cmp_bcd_date(&tmp_date, &today) <= 0) {
                /*
                 * today might after lock date, once machine 
                 * lock down, machine cann't issue invoice. 
                 */
                ret = tax_base_check_lock_date(&tmp_date);
                if (ret <= 0)
                    return -ETAX_MACH_LOCKED;

                ret = tax_sys_nul_daily_collect(&tmp_date);
                if (ret < 0)
                    return ret;
            } else { 
                debug_msg("[TAX SYSTEM] : daily collect done\n");
                return SUCCESS;
            }

            rt_ops->get_next_date(&tmp_date);
        }
    } else {
        /*
         * if machine havn't do daily collect since last 
         * time do daily collect
         */
        if (last_issue_date == NULL) {
            tmp_date = *last_dc_date;
            rt_ops->get_next_date(&tmp_date);
            while(1) {
                if (rt_ops->cmp_bcd_date(&tmp_date, &today) <= 0) {
                    ret = tax_base_check_lock_date(&tmp_date);
                    if (ret <= 0)
                        return -ETAX_MACH_LOCKED;

                    ret = tax_sys_nul_daily_collect(&tmp_date);
                    if (ret < 0)
                        return ret;
                } else { 
                    debug_msg("[TAX SYSTEM] : daily collect done\n");
                    return SUCCESS;
                }

                rt_ops->get_next_date(&tmp_date);
            }
        } else {
            /* TODAY_ID_FILE is not empty */
            tmp_date = *last_dc_date;  
            while (rt_ops->cmp_bcd_date(&tmp_date, last_issue_date) < 0) {
                rt_ops->get_next_date(&tmp_date);

                ret = tax_sys_nul_daily_collect(&tmp_date);
                if (ret < 0)
                    return ret;
            }

            /*
             * daily collect for today will clear today_id.dat 
             * read last record out before call it.
             */
            ret = tax_file_read_last_record(TODAY_ID_FILE, (uchar *)&last_issue_rec,
                    sizeof(last_issue_rec));
            if (ret < 0)
                return ret;

            /* daily collect progress */
            ret = tax_sys_dc_for_today(last_issue_date);
            if (ret < 0)
                return ret;

            tmp_date = last_issue_rec.date;
            rt_ops->get_next_date(&tmp_date); 
            while (1) {
                if (rt_ops->cmp_bcd_date(&tmp_date, &today) <= 0) {
                    ret = tax_base_check_lock_date(&tmp_date);
                    if (ret <= 0)
                        return -ETAX_MACH_LOCKED;

                    ret = tax_sys_nul_daily_collect(&tmp_date);
                    if (ret < 0)
                        return ret;
                } else 
                    return SUCCESS;

                rt_ops->get_next_date(&tmp_date);
            }

            debug_msg("[TAX SYSTEM] : daily collect done!\n");

            return SUCCESS;
        }
    }

    /*
     * can not get here in normal case 
     */
    return FAIL;
}

/*
 * tax_sys_write_check_daily_detail - get inspect date structure and write 
 *                        it to adam card. 
 *  @cur_offset : assigned where to get detail from local file 
 *  @return : status 
 */
static int tax_sys_write_check_daily_detail(int cur_offset, int card_rec_nb)
{
    int ret;
    char buf[10] = {0};
    struct bcd_date today;
    struct inspect_record inspect_record;
    struct daily_collect card_daily_rec;
    struct tax_sys_daily_collect_record sys_daily_rec; 
    
    struct check_card *check_card = get_check_card();
    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct rt_operate *rt_ops = get_rt_ops();

    memset(&inspect_record, 0, sizeof(inspect_record));
    memset(&card_daily_rec, 0, sizeof(card_daily_rec));
    memset(&sys_daily_rec, 0, sizeof(sys_daily_rec));

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    ret = tax_file_read_daily_collect(cur_offset, &sys_daily_rec);
    if (ret < 0)
        return ret;
    
    ret = tax_trans_daily_collect(0, &sys_daily_rec, &card_daily_rec, NULL);
    if (ret < 0)
        return ret;
    
    /* header */
    inspect_record.type = INSPECT_DAILY_SUM;
    memcpy(inspect_record.cur_date, &today, 4);
    memcpy(inspect_record.taxpayer_num, gp_app_info->taxpayer_nb, 8);
    memcpy(inspect_record.machine_nb, gp_mach_info->machine_nb, 8);
    
    /* data body*/
    memcpy(&inspect_record.daily_detail, &card_daily_rec, 
            sizeof(card_daily_rec) - 1);
    memcpy(&inspect_record.daily_detail.sign, sys_daily_rec.sign, 128);
    
    ret = check_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_check_card_ready();
    if (ret != POSITIVE)
        return -ETAX_CHK_CARD_NOT_READY;
    
    ret = check_card_get_ctl_data((uchar *)buf);
    if (ret != SUCCESS)
        return ret;

    if (buf[9] != 0xff) {
        ret = fiscal_card_power_on();
        if (ret != SUCCESS) {
            check_card->power_off(CHECK_CARD);
            return ret;
        }

        ret = is_fiscal_card_ready();
        if (ret != POSITIVE) {
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_FC_NOT_READY;
        }

        ret = check_card_authority_fc(buf[9]);
        if (ret != SUCCESS) { 
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_AUTH_CHKC_FAIL;
        }

        fiscal_card->power_off(FISCAL_CARD);
    }

    ret = check_card_write_chk_rec((uchar)card_rec_nb, (uchar *)&inspect_record);
    check_card->power_off(CHECK_CARD);

    return ret;
}

/*
 * tax_sys_issue_invoice_proc - issue invoice 
 *  @issue_inv_rec : issue invoice data 
 *  @issue_inv_res : issue invoice response data 
 *  @return : status 
 */
static int tax_sys_issue_invoice_proc(struct tax_sys_issue_invoice * issue_inv_rec,
        struct issue_invoice_res * issue_inv_res)
{
    int ret;

    struct fiscal_card * fiscal_card;

    struct tax_sys_pin_record *gp_pin = get_pin();
    struct tax_sys_fis_config_record * gp_fis_cfg = get_fis_config();
    struct tax_sys_cur_roll_left_record * gp_cur_roll_left = get_cur_roll_left();

    struct issue_invoice card_issue_inv_rec;
    struct tax_sys_today_id_record today_id_rec;
    struct tax_sys_cur_roll_id_record cur_roll_id_rec;

#ifdef CONFIG_DEBUG
    assert(issue_inv_rec != NULL);
    assert(issue_inv_res != NULL);
#endif 

    debug_msg("DOING ISSUE INVOICE.\n");

    fiscal_card = get_fiscal_card();
    if (fiscal_card == NULL)
        return -ETAX_NUL_FISCAL_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state *pm = get_power_state();
    pm->check_power_state();
#endif 

    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

    ret = tax_trans_issue_invoice(0, issue_inv_rec, &card_issue_inv_rec);
    if (ret != SUCCESS)
        goto card_off;

    ret = fiscal_card->issue_invoice(&card_issue_inv_rec, issue_inv_res);
    if (ret < 0)
        goto card_off;

    /* update : current roll left num */
    gp_cur_roll_left->cur_roll_left --;
    ret = tax_file_save_crln(gp_cur_roll_left);
    if (ret < 0)
        goto card_off;

    /* update : amount */
    struct tax_sys_amount_record amt_rec; 
    ret = tax_file_read_amount(&amt_rec);
    if (ret < 0 && ret != -EFILE_NO_REC)
        goto card_off;

    if (issue_inv_rec->invoice_type == NORMAL_INVOICE) {
        amt_rec.amt_total_this += issue_inv_rec->amt_total;
        if ( gp_fis_cfg->declare_flag == POSITIVE )
            amt_rec.amt_total_next += issue_inv_rec->amt_total;
    } 
    else if (issue_inv_rec->invoice_type == RETURN_INVOICE) {
        amt_rec.amt_return_this += issue_inv_rec->amt_total;
        if ( gp_fis_cfg->declare_flag == POSITIVE )
            amt_rec.amt_return_next += issue_inv_rec->amt_total;
    } 

    ret = tax_file_save_amount(&amt_rec);
    if (ret < 0)
        goto card_off;

    /* update : current roll id */
    memset(&cur_roll_id_rec, 0, sizeof(cur_roll_id_rec));
    cur_roll_id_rec.date = issue_inv_rec->date;
    cur_roll_id_rec.type = issue_inv_rec->invoice_type;
    cur_roll_id_rec.amout_total = issue_inv_rec->amt_total;

    ret = tax_file_append_cur_roll_id(&cur_roll_id_rec);
    if (ret < 0)
        goto card_off;

    if (gp_cur_roll_left->cur_roll_left == 0) {
        ret = tax_sys_cur_roll_collect();
        if (ret < 0)
            goto card_off;
    }

    /* update : today id */
    memset(&today_id_rec, 0, sizeof(today_id_rec));
    today_id_rec.date = issue_inv_rec->date;
    today_id_rec.type = issue_inv_rec->invoice_type;

    int i;
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        today_id_rec.item[i].index = issue_inv_rec->item[i].index;
        today_id_rec.item[i].amount = issue_inv_rec->item[i].amount;
    }

    ret = tax_file_append_today_id(&today_id_rec);
    if (ret < 0)
        goto card_off;

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);

    return ret;
}

/*
 * tax_sys_write_check_invoice_detail - write a inspect_invoice record to card 
 *  @rec_num:
 *  @index_h:
 *  @card_rec_nb:
 */
static int tax_sys_write_check_invoice_detail(int rec_num, int index_h, int card_rec_nb)
{
    int ret;
    char buf[10] = {0};
    int index, offset;

    struct bcd_date today;
    struct issue_invoice card_invoice_rec;
    struct inspect_invoice inv_detail[7]; // 1 - 7
    struct inspect_record inspect_record;
    struct tax_sys_invoice_detail_record sys_invoice_rec;

    struct check_card *check_card = get_check_card();
    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct rt_operate *rt_ops = get_rt_ops();

    memset(&inspect_record, 0, sizeof(inspect_record));
    memset(&card_invoice_rec, 0, sizeof(card_invoice_rec));
    memset(&sys_invoice_rec, 0, sizeof(sys_invoice_rec));
    memset(inv_detail, 0, 7 * sizeof(struct inspect_invoice));

    if (rec_num > 7 || rec_num <= 0)
        return FAIL;

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    /* header */
    inspect_record.type = INSPECT_INV_DETAIL;
    memcpy(inspect_record.cur_date, &today, 4);
    memcpy(inspect_record.taxpayer_num, gp_app_info->taxpayer_nb, 8);
    memcpy(inspect_record.machine_nb, gp_mach_info->machine_nb, 8);

    int i = 0;
    index = (index_h >> 16) & 0xffff;
    offset = index_h & 0xffff;

    do {
        ret = tax_file_read_invoice_detail(index, offset, &sys_invoice_rec);
        if (ret < 0)
            return ret;

        ret = tax_trans_inspect_invoice(0, &sys_invoice_rec.invoice, &inv_detail[i]);
        if (ret < 0)
            return ret;
        
        /* this is the last record in file ?*/ 
        if (offset == INVOICE_DETAIL_REC_NUM) {
            offset = 1;
            if (index == 10)
                index = 1;
            else 
                index ++;
        } else 
            offset ++;

        i++;  //next buffer
    } while (--rec_num);
    
    inspect_record.reserved[0] = (uchar)rec_num;
    memcpy(inspect_record.reserved + 1, inv_detail, 7 * sizeof(inv_detail[0]));

    /* start to write... */
    ret = check_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_check_card_ready();
    if (ret != POSITIVE)
        return -ETAX_CHK_CARD_NOT_READY;

    ret = check_card_get_ctl_data((uchar *)buf);
    if (ret != SUCCESS)
        return ret;

    if (buf[9] != 0xff) {
        ret = fiscal_card_power_on();
        if (ret != SUCCESS) {
            check_card->power_off(CHECK_CARD);
            return ret;
        }

        ret = is_fiscal_card_ready();
        if (ret != POSITIVE) {
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_FC_NOT_READY;
        }

        ret = check_card_authority_fc(buf[9]);
        if (ret != SUCCESS) { 
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_AUTH_CHKC_FAIL;
        }

        fiscal_card->power_off(FISCAL_CARD);
    }

    ret = check_card_write_chk_rec((uchar)card_rec_nb, (uchar *)&inspect_record);

    check_card->power_off(CHECK_CARD);
    return ret;
}

/*
 * tax_sys_redeclare_duty - redo declare duty on user card 
 *                          redeclare won't update file re-
 *                          cord. 
 *  @return : status 
 */
static int tax_sys_redeclare_duty(struct bcd_date *start_date, struct bcd_date *end_date)
{
    int ret;

    struct tax_sys_daily_collect_record daily_collect_rec;
    struct declare_duty card_declare_rec;
    struct declare_res card_declare_res;

    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct user_card * user_card = get_user_card();

    struct tax_sys_pin_record * gp_pin = get_pin();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();

    memset(&card_declare_rec, 0, sizeof(card_declare_rec));
    memset(&card_declare_res, 0, sizeof(card_declare_res));
    memset(&daily_collect_rec, 0, sizeof(daily_collect_rec));

    if (fiscal_card == NULL)
        return -ETAX_NUL_FISCAL_CARD;

    if (user_card == NULL)
        return -ETAX_NUL_USER_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
    }

    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb, 
            gp_app_info->fiscal_card_nb);
    if (ret < 0)
        goto card_off;

    /*
     *  no need to do daily collect when do re-declare duty 
     */
#if 0
    ret = tax_sys_daily_collect();
    if (ret < 0)
        return ret;
#endif 

    struct tax_sys_declare_duty_record last_declare_rec;

    ret = tax_file_read_last_record(DECLARE_DUTY_FILE, (uchar *)&last_declare_rec,
            sizeof(last_declare_rec));
    if (ret < 0)
        goto card_off;

    *start_date = last_declare_rec.start_date;
    *end_date = last_declare_rec.end_date;

    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

    /*
     * Adjust tax date as the same as fiscal card
     *  - Actually, we don't need to adjust tax rate 
     *  here, but avoid the fiscal card is changed, 
     *  do that anyway.
     */
    uchar card_tax_index[6];
    uint card_amt_vaild[6];
    uint card_amt_return[6];

    ret = fiscal_card_get_tax_index(card_tax_index);
    if (ret < 0)
        goto card_off;

    int i, j; 
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (last_declare_rec.tax_index[j] == card_tax_index[i])
                break;
        }

        card_amt_vaild[i] = last_declare_rec.amt_valid[j];
        card_amt_return[i] =  last_declare_rec.amt_return[j];
    }

    memcpy(last_declare_rec.tax_index, card_tax_index, 6);
    memcpy((uchar *)last_declare_rec.amt_valid, (uchar *)card_amt_vaild, 24);
    memcpy((uchar *)last_declare_rec.amt_return, (uchar *)card_amt_return, 24);

    /*
     * covert to card date
     */
    ret = tax_trans_declare_duty(0, &last_declare_rec, &card_declare_rec, NULL);
    if (ret < 0)
        goto card_off;

    ret = fiscal_card->declare_duty(&card_declare_rec, &card_declare_res);
    if (ret < 0)
        goto card_off;

    ret = tax_trans_declare_duty(1, &last_declare_rec, &card_declare_rec, &card_declare_res);
    if (ret < 0)
        goto card_off;

    /*
     * fill-up date clloct struct 
     */
    struct data_collect card_date_collect;
    ret = tax_trans_data_collect(0, &card_declare_rec, &card_declare_res, &card_date_collect);
    if (ret < 0)
        goto card_off;

    ret = user_card->data_collect(&card_date_collect);
    if (ret < 0)
        goto card_off;

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_sys_declare_duty - declare fiscal data to user card 
 *  @flag : declare flag 
 *          0 - normal declare - assigned the end day, not include today
 *          1 - downtime declare - include today
 *          2 - month declare - end date of last month
 *  
 *  @end_date : assigned declare end date 
 *  @return : status  
 */ 
static int tax_sys_declare_duty(int flag, struct bcd_date * end_date)
{
    int ret;

    struct bcd_date tmp_date;
    struct tax_sys_declare_duty_record last_declare_rec;

    struct tax_sys_declare_duty_record declare_rec;
    struct tax_sys_daily_collect_record daily_collect_rec;
    struct declare_duty card_declare_rec;
    struct declare_res card_declare_res;

    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct user_card * user_card = get_user_card();
    struct rt_operate * rt_ops = get_rt_ops();

    struct tax_sys_pin_record * gp_pin = get_pin();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct tax_sys_fis_config_record *gp_fis_cfg = get_fis_config();

    memset(&declare_rec, 0, sizeof(declare_rec));
    memset(&card_declare_rec, 0, sizeof(card_declare_rec));
    memset(&card_declare_res, 0, sizeof(card_declare_res));
    memset(&daily_collect_rec, 0, sizeof(daily_collect_rec));
    memset(&last_declare_rec, 0, sizeof(last_declare_rec));

#ifdef CONFIG_DEBUG
    assert(flag >= 0 && flag <= 2);
#else 
    if (!(flag >= 0 && flag <= 2))
        return FAIL;
#endif

    debug_msg("DOING DECLARE DUTY...");

    /* check normal_declare/month_declare parament */
    if ((flag != 0 && end_date != NULL) || (flag == 0 && end_date == NULL))
        return -ETAX_BAD_DECLARE_TYPE;

    if (fiscal_card == NULL)
        return -ETAX_NUL_FISCAL_CARD;

    if (user_card == NULL)
        return -ETAX_NUL_USER_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
    }

    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb, 
            gp_app_info->fiscal_card_nb);
    if (ret < 0)
        goto card_off;

    /*
     * daily collect 
     * avoid declaring while machine did not power off for somedays
     */
    ret = tax_sys_daily_collect();
    if (ret < 0)
        return ret;

    ret = tax_file_is_empty(DAILY_COLLECT_FILE);
    if (ret == POSITIVE) {
        ret = -ETAX_NODATA_TO_DECLARE;
        goto card_off;
    }

    ret = tax_file_read_last_record(DECLARE_DUTY_FILE, (uchar *)&last_declare_rec,
            sizeof(last_declare_rec));
    if (ret < 0) {
        if (ret == -EFILE_NO_REC) {
            declare_rec.start_date = gp_app_info->init_date;
        } else 
            goto card_off;
    } else {
        if (gp_fis_cfg->declare_flag == 0) {
            /*
             * didn't declare before
             */
            tmp_date = last_declare_rec.end_date;
            ret = rt_ops->get_next_date(&tmp_date);
            if (ret < 0)
                goto card_off;

            declare_rec.start_date = tmp_date;
        } else {
            /*
             * during delare, need to declare from last_declare's start_date 
             */
            declare_rec.start_date = last_declare_rec.start_date;
        }
    }

    /* cur_date */
    ret = rt_ops->get_cur_date(&tmp_date);
    if (ret < 0)
        goto card_off;

    declare_rec.cur_date = tmp_date;

    /* end_date */
    int days;
    switch (flag) {
        case NORMAL_DECLARE:
            /* user assigned end_date */ 
            tmp_date = *end_date;
            break;

        case DOWNTIME_DECLARE:
            /* end date is today in downtime case */
            ret = rt_ops->get_cur_date(&tmp_date);
            if (ret < 0)
                goto card_off;
            break;

        case MONTH_DECLARE:
            /* the last day of last monty is end_date */
            bcd_to_dec(&tmp_date.day, (uint *)&days);
            sub_bcd_date(&tmp_date, (uint)days);

            ret = rt_ops->cmp_bcd_date(&tmp_date, &declare_rec.start_date);
            if (ret < 0)
                return -ETAX_NODATA_TO_DECLARE;

            break;

        default:
            ret = -ETAX_BAD_DECLARE_TYPE;
            goto card_off;
            break;        
    }

    /* 
     * check end date 
     */
    ret = rt_ops->cmp_bcd_date(&tmp_date, &gp_app_info->issue_limit_date);
    if (ret > 0)
        tmp_date = gp_app_info->issue_limit_date;

    /*
     * if end_date after lock date, set end_date to date that 
     * one day befor lock date
     */
    days = tax_base_check_lock_date(&tmp_date);
    if (days <= 0) {
        days = -days;
        sub_bcd_date(&tmp_date, (uint)days + 1);  
    }

    declare_rec.end_date = tmp_date;

    /* tax index & rate */
    memcpy(declare_rec.tax_index, gp_app_info->tax_index, MAX_USER_TAXRATE);

    /* read all record from daily collect file */
    int rec_count;
    rec_count = tax_file_get_rec_num(DAILY_COLLECT_FILE);
    if (rec_count < 0)
        goto card_off;

    int i, j, rate_nb;
    for (i = rec_count; i > 0; i--) {
        ret = tax_file_read_daily_collect(i, &daily_collect_rec);
        if (ret < 0)
            goto card_off;

        if (rt_ops->cmp_bcd_date(&daily_collect_rec.cur_date, 
                    &declare_rec.start_date) < 0)
            break;

        if (rt_ops->cmp_bcd_date(&daily_collect_rec.cur_date,
                    &declare_rec.end_date) > 0)
            continue;

        declare_rec.valid_count += daily_collect_rec.valid_count;
        declare_rec.return_count += daily_collect_rec.return_count;
        declare_rec.spoil_count += daily_collect_rec.spoil_count;

        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (daily_collect_rec.tax_index[j] != 0) {
                rate_nb = tax_base_get_rate_nb(daily_collect_rec.tax_index[j]);
                if (rate_nb > 0) {
                    declare_rec.amt_valid[rate_nb] += daily_collect_rec.amt_valid[j];
                    declare_rec.amt_return[rate_nb] += daily_collect_rec.amt_return[j];
                }
            }
        }
    }

    /* comfirm there is data to be declared */ 
    if (i == rec_count) {
        ret = -ETAX_NODATA_TO_DECLARE;
        goto card_off;
    }

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state * pm = get_power_state();
    pm->check_power_state();
#endif 

    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

    uchar card_tax_index[6] = {0};
    uint card_amt_vaild[6] = {0};
    uint card_amt_return[6] = {0};

    ret = fiscal_card_get_tax_index(card_tax_index);
    if (ret < 0)
        goto card_off;

    /*
     * Adjusting tax rate to as the same as fiscal card
     * tax rate data
     */ 
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (declare_rec.tax_index[j] == card_tax_index[i]) {
                card_amt_vaild[i] = declare_rec.amt_valid[j];
                card_amt_return[i] =  declare_rec.amt_return[j];
            }
        }
    }

    memcpy(declare_rec.tax_index, card_tax_index, 6);
    memcpy((uchar *)declare_rec.amt_valid, (uchar *)card_amt_vaild, 24);
    memcpy((uchar *)declare_rec.amt_return, (uchar *)card_amt_return, 24);

    ret = tax_trans_declare_duty(0, &declare_rec, &card_declare_rec, NULL);
    if (ret < 0)
        goto card_off;

    ret = fiscal_card->declare_duty(&card_declare_rec, &card_declare_res);
    if (ret < 0)
        goto card_off;

    ret = tax_trans_declare_duty(1, &declare_rec, &card_declare_rec, &card_declare_res);
    if (ret < 0)
        goto card_off;

    /*
     * fill-up date_clloct struct 
     */
    struct data_collect card_date_collect;
    ret = tax_trans_data_collect(0, &card_declare_rec, &card_declare_res, &card_date_collect);
    if (ret < 0)
        goto card_off;

    ret = user_card->data_collect(&card_date_collect);
    if (ret < 0)
        goto card_off;

    /* update declare file */
    ret = tax_file_append_declare_duty(&declare_rec);
    if (ret < 0)
        goto card_off;

    /*
     * update amount information 
     */ 
    rec_count = tax_file_get_rec_num(DAILY_COLLECT_FILE);
    if (rec_count < 0)
        goto card_off;

    uint new_amt_valid = 0;
    uint new_amt_return = 0;
    for (i = rec_count; i > 0; i--) {
        ret = tax_file_read_daily_collect(i, &daily_collect_rec);
        if (ret < 0)
            goto card_off;

        if (rt_ops->cmp_bcd_date(&daily_collect_rec.cur_date,
                    &declare_rec.end_date) < 0)
            break;

        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (daily_collect_rec.tax_index[j] != 0) {
                int rate_nb = tax_base_get_rate_nb(daily_collect_rec.tax_index[j]);
                if (rate_nb > 0) {
                    new_amt_valid += daily_collect_rec.amt_valid[j];
                    new_amt_return += daily_collect_rec.amt_return[j];
                }
            }
        }
    }

    struct tax_sys_amount_record amt_rec;
    ret = tax_file_read_amount(&amt_rec);
    if (ret < 0)
        goto card_off;

    amt_rec.amt_return_next = new_amt_return;
    amt_rec.amt_total_next = new_amt_valid;

    ret = tax_file_save_amount(&amt_rec);
    if (ret < 0)
        goto card_off;

    /*
     * update fiscal config record 
     */   
    gp_fis_cfg->declare_flag = 1;
    ret = tax_file_save_fis_cfg(gp_fis_cfg);
    if (ret < 0)
        goto card_off;

    debug_msg("Done !\n");

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_sys_write_check_declare_detail - write declare inspect date 
 *                    to admin card.
 *  @rec_num : number of record
 *  @cur_offset: where to get declare detail 
 *  @return : status 
 */
static int tax_sys_write_check_declare_detail(int rec_num, int cur_offset, int card_rec_nb)
{
    int ret;
    char buf[10] = {0};
    struct bcd_date today;
    struct declare_duty card_declare_rec;
    struct tax_sys_declare_duty_record sys_declare_rec; 
    struct inspect_record inspect_record;
    struct inspect_declare declare_detail[2];

    struct check_card *check_card = get_check_card();
    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct rt_operate *rt_ops = get_rt_ops();

    memset(&inspect_record, 0, sizeof(inspect_record));
    memset(&card_declare_rec, 0, sizeof(card_declare_rec));
    memset(&sys_declare_rec, 0, sizeof(sys_declare_rec));
    memset(declare_detail, 0, 2 * sizeof(struct inspect_declare));

    if (rec_num > 2 || rec_num <= 0)
        return FAIL;

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    /* header */
    inspect_record.type = INSPECT_DECLARE;
    memcpy(inspect_record.cur_date, &today, 4);
    memcpy(inspect_record.taxpayer_num, gp_app_info->taxpayer_nb, 8);
    memcpy(inspect_record.machine_nb, gp_mach_info->machine_nb, 8);

    int i = 0;
    do {
        ret = tax_file_read_declare_duty(cur_offset, &sys_declare_rec);
        if (ret < 0)
            return ret;

        ret = tax_trans_declare_duty(0, &sys_declare_rec, &card_declare_rec, NULL);
        if (ret < 0)
            return ret;

        /* data body*/
        memcpy(&declare_detail[i], &card_declare_rec, 
                sizeof(card_declare_rec) - 1);

        declare_detail[i].status = sys_declare_rec.status;
        memcpy(declare_detail[i].mac1, sys_declare_rec.mac1, 4);
        end_cover_int(declare_detail[i].total_valid, (uchar *)&sys_declare_rec.total_valid);
        end_cover_int(declare_detail[i].total_return, (uchar *)&sys_declare_rec.total_return);

        i++;  //next buffer
        cur_offset++; //next record 
    } while (--rec_num);

    /* read declare detail */
    inspect_record.reserved[0] = (uchar)rec_num;
    memcpy(inspect_record.reserved + 1, declare_detail, 2 * sizeof(declare_detail[0]));

    /* start to write... */
    ret = check_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_check_card_ready();
    if (ret != POSITIVE)
        return -ETAX_CHK_CARD_NOT_READY;

    ret = check_card_get_ctl_data((uchar *)buf);
    if (ret != SUCCESS)
        return ret;

    if (buf[9] != 0xff) {
        ret = fiscal_card_power_on();
        if (ret != SUCCESS) {
            check_card->power_off(CHECK_CARD);
            return ret;
        }

        ret = is_fiscal_card_ready();
        if (ret != POSITIVE) {
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_FC_NOT_READY;
        }

        ret = check_card_authority_fc(buf[9]);
        if (ret != SUCCESS) { 
            check_card->power_off(CHECK_CARD);
            fiscal_card->power_off(FISCAL_CARD);
            return -ETAX_AUTH_CHKC_FAIL;
        }

        fiscal_card->power_off(FISCAL_CARD);
    }

    ret = check_card_write_chk_rec((uchar)card_rec_nb, (uchar *)&inspect_record);

    check_card->power_off(CHECK_CARD);
    return ret;
}

/*
 * tax_sys_update_control - update fiscal control date 
 *  @return status 
 */
static int tax_sys_update_control(void)
{
    int ret;
    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct user_card * user_card = get_user_card();
    struct tax_sys_pin_record * gp_pin = get_pin();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct tax_sys_fis_config_record *gp_fis_cfg = get_fis_config();
    struct usr_ef01_record usr_ef01_rec;
    struct tax_sys_app_info new_app_rec;
    struct tax_sys_amount_record new_amt_rec;

    memset(&usr_ef01_rec, 0, sizeof(usr_ef01_rec));
    memset(&new_app_rec, 0, sizeof(new_app_rec));
    memset(&new_amt_rec, 0, sizeof(new_amt_rec));

#ifdef CONFIG_DEBUG
    debug_msg("DOING UPDATE CONTROL\n");
#endif 

    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
        goto card_off;
    }

    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb, 
            &gp_app_info->fis_type_num);
    if (ret < 0)
        goto card_off;

    ret = user_card_get_ctl_data(gp_app_info->fiscal_card_nb, &usr_ef01_rec);
    if (ret < 0)
        goto card_off;

    new_app_rec = *gp_app_info; 
    ret = user_card_get_fis_type(&new_app_rec);
    if (ret < 0)
        goto card_off;

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state *pm = get_power_state();
    pm->check_power_state();
#endif 

    struct tax_sys_pin_record new_pin;
    ret = fiscal_card->verify_fiscal_pin(gp_pin->pin, new_pin.pin);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0)
        goto card_off;

#ifdef CONFIG_PWR_OFF_PROTECT
    pm->check_power_state();
#endif    

    struct update_ctl_info card_ctl_rec;
    memcpy(&card_ctl_rec, &usr_ef01_rec.encripty_id, sizeof(card_ctl_rec));
    ret = fiscal_card->update_controls(&card_ctl_rec);
    if (ret < 0)
        goto card_off;

    ret = fiscal_card_get_new_ctl_data(&new_app_rec);
    if (ret < 0)
        goto card_off;

    gp_fis_cfg->declare_flag = 0;
    ret = tax_file_save_fis_cfg(gp_fis_cfg);
    if (ret < 0)
        goto card_off;

    ret = tax_file_save_app_info(&new_app_rec);
    if (ret < 0)
        goto card_off;

    ret = tax_file_read_amount(&new_amt_rec);
    if (ret < 0)
        goto card_off;

    new_amt_rec.amt_return_this = new_amt_rec.amt_return_next;
    new_amt_rec.amt_return_next = 0;

    new_amt_rec.amt_total_this = new_amt_rec.amt_total_next;
    new_amt_rec.amt_total_next = 0;

    ret = tax_file_save_amount(&new_amt_rec);
    if (ret < 0)
        goto card_off;

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

int tax_sys_update_taxpayer(void)
{
    int ret;
    uchar buf_uc[50] = {0};
    uchar buf_fc[50] = {0};
    struct fiscal_card * fiscal_card = get_fiscal_card();
    struct user_card * user_card = get_user_card();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();
    struct machine_info_record * gp_mach_info = get_mach_info();

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_UC_NOT_READY;
        goto card_off;
    }

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    ret = fiscal_card_get_machine_nb(buf_fc);
    if (ret != SUCCESS) 
        goto card_off;

    if (memcmp(gp_mach_info->machine_nb, buf_fc, 8) != 0) {
        ret = -ETAX_DIFF_MACH_NB;
        goto card_off;
    }

    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb,
            gp_app_info->fiscal_card_nb);
    if (ret != SUCCESS)
        goto card_off;

    ret = fiscal_card_get_taxpayer_name(buf_fc);
    if (ret != SUCCESS)
        goto card_off;

    ret = user_card_get_taxpayer_name(buf_uc);
    if (ret != SUCCESS)
        goto card_off;

    /*
     * taxpayer's name in FC&UC dismatched 
     */
    if (memcmp(buf_fc, buf_uc, 40) != 0) {
        ret = -ETAX_TAXPAYER_NAME_DISMATCH;
        goto card_off;
    }

    /*
     * check current taxpayer name as the same as fiscal card 
     */
    if (memcmp(buf_fc, gp_app_info->taxpayer_name, 40) == 0) {
        ret = SUCCESS;
        goto card_off;
    }

    memcpy(gp_app_info->taxpayer_name, buf_fc, 40);
    gp_app_info->taxpayer_name[40] = '\0';
    ret = tax_file_save_app_info(gp_app_info);
    if (ret != SUCCESS)
        goto card_off;

    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_sys_power_on_check - power on self check 
 *
 * NOTICE
 *   Everytime power on, should runing this for once.
 *   in this funtion, will do mount roll and day sum.
 */
static int tax_sys_power_on_check(void)
{
    int ret;

    struct bcd_date today;
    struct fiscal_card * fis_card;
    struct user_card * usr_card;
    struct rt_operate * rt_ops; 

    fis_card = get_fiscal_card();
    if (!fis_card)
        return -ETAX_NUL_FISCAL_CARD;

    usr_card = get_user_card();
    if (!usr_card)
        return -ETAX_NUL_USER_CARD;

    rt_ops = get_rt_ops();
    if (!rt_ops)
        return -ERT_NUL_RT_OPS;

    /* get current date */
    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

#ifdef CONFIG_CHECK_DEADLINE
    /*
     * today should after produce date 
     */
    ret = tax_base_check_deadline(&today);
    if (ret < 0)
        return -ETAX_DATE_CONFUSE;
#endif 

    /* read fiscal config file */
    struct tax_sys_fis_config_record * gp_fis_cfg = get_fis_config();
    ret = tax_file_read_fis_cfg(gp_fis_cfg);
    if (ret < 0)
        return ret;

    /* read application file */
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();
    ret = tax_file_read_app_info(gp_app_info);
    if (ret < 0)	
        return ret;

    /* read the current roll file */    
    struct tax_sys_buy_roll_record * gp_cur_roll = get_cur_roll();
    ret = tax_file_read_cur_roll(gp_cur_roll);
    if (ret < 0 && ret != -EFILE_OFFSET_OVER_FLOW)
        return ret;

    /* read the current roll left num */
    struct tax_sys_cur_roll_left_record * gp_cur_roll_left = get_cur_roll_left();
    ret = tax_file_read_crln(gp_cur_roll_left);
    if (ret < 0 && ret != -EFILE_OFFSET_OVER_FLOW)
        return ret;

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret = -ETAX_FC_NOT_READY;
        goto card_off;
    }

    /* check machine number */
    uchar res[8];
    ret = fiscal_card_get_machine_nb(res);
    if (ret < 0) 
        goto card_off;

    /* machine info*/
    struct machine_info_record * gp_mach_info = get_mach_info();
    ret = tax_file_read_mach_info(gp_mach_info);
    if (ret < 0)
        goto card_off;

    if (memcmp(res, gp_mach_info->machine_nb, 8) != 0) {
        ret = -ETAX_DIFF_MACH_NB;
        goto card_off;
    }

    /*
     * fiscal card susposed to be registered here 
     * otherwise, the fiscal system might not be initialzed 
     */
    ret = is_fiscal_card_register();
    if (ret != POSITIVE){
        if (ret == NEGATIVE)
            ret = -ETAX_FC_NOT_REG;

        goto card_off;
    }

    /* check pin lock */
    if (gp_fis_cfg->pin_lock_flag == POSITIVE) {
        ret = -ETAX_FC_PIN_LOCK;
        goto card_off;
    }

    /* check pin and save new pin */
    struct tax_sys_pin_record * pin = get_pin();
    ret = tax_file_read_pin(pin);
    if (ret < 0) 
        goto card_off;

#ifdef CONFIG_PWR_OFF_PROTECT
    struct power_state * pm = get_power_state();

    pm->check_power_state();
#endif

    /*
     * Verify fiscal card PIN
     */
    struct tax_sys_pin_record new_pin;
    ret = fis_card->verify_fiscal_pin(pin->pin, new_pin.pin);
    if (ret < 0) {
        if (ret == -ETAX_FC_PIN_LOCK) {
            gp_fis_cfg->pin_lock_flag = POSITIVE;
            tax_file_save_fis_cfg(gp_fis_cfg); 
        }

        goto card_off;    
    }

    ret = tax_file_save_pin(&new_pin);
    if (ret < 0) 
        goto card_off;

#if 0
    /* mount invoice roll if need */
    struct tax_sys_protect_record * prot_rec = get_protect_record();
    ret = tax_file_is_empty(PROTECT_FILE);
    if (ret == NEGATIVE) {
        /*
         * current roll has no left
         * and there is a roll has been distributed but hasn't been mounted,
         * then mount it 
         */
        if (gp_cur_roll_left->cur_roll_left < 1) {

            ret = tax_file_read_protect(prot_rec);
            if (ret < 0)
                goto card_off;

            if (prot_rec->type == PROTECT_TYPE_MOUNT) {

#ifdef CONFIG_PWR_OFF_PROTECT
                /*
                 * mount invoice roll must not be interrupt
                 */
                pm->check_power_state();
#endif      

                ret = tax_sys_mount_roll(&prot_rec->invoice_info);
                if (ret < 0)
                    return ret;
            } else {
                ret = tax_file_clear(PROTECT_FILE);
                if (ret < 0)
                    goto card_off;
            }
        }
    } else if (ret != POSITIVE) 
        goto card_off;
#endif 

    /* do day sum */
    ret = tax_sys_daily_collect();
    if (ret < 0)
        goto card_off;

    /* 
     * check lock date 
     * machine should be lock down after 5 years, said by GB
     */
    ret = tax_base_check_lock_date(&today);
    if (ret <= 0) {
        ret = -ETAX_MACH_LOCKED;
        goto card_off;
    }

    ret = SUCCESS;

card_off:
    fis_card->power_off(FISCAL_CARD);
    usr_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_sys_check_finish_tax - check user card is finished tax 
 *                            progress or not.
 *  @
 */
static int tax_sys_check_finish_tax(uchar * card_nb)
{
    int ret;
    uchar buf[220] = {0};
    struct user_card * user_card = get_user_card();
    struct machine_info_record * gp_mach_info = get_mach_info();
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    ret = user_card_power_on();
    if (ret != SUCCESS)
        return ret;

    ret = is_user_card_ready();
    if (ret != POSITIVE)
        goto fail;
    
    ret = user_card_check_card(gp_app_info->taxpayer_nb, gp_mach_info->machine_nb,
            gp_app_info->fiscal_card_nb);
    if (ret != SUCCESS)
        goto fail;

    ret = user_card->select_file_by_df(USER_CARD, TID01);
    if (ret != SUCCESS)
        goto fail;

    int i;
    for (i = 0; i < MAX_CARD_DECNUM; i++) {
        ret = user_card->read_record(USER_CARD, i + 1, 220, buf);
        if (ret != SUCCESS)
            goto fail;

        if (memcmp(card_nb, buf + 1, 8) == 0)
            break;
    }

    if (i == MAX_CARD_DECNUM)
        ret = NEGATIVE;
    else 
        ret = POSITIVE;

fail:
    user_card->power_off(USER_CARD);
    return ret;
}

/*
 * tax_sys_transact_prepare - check system status before 
 *                            transaction(issue invoice and return invoice).
 *  @return : system status 
 */
static int tax_sys_transact_prepare(void)
{
    int ret;

    struct bcd_date today;
    struct tax_sys_amount_record amt_record;
    struct tax_sys_config_record sys_cfg_rec;
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    struct rt_operate * rt_ops = get_rt_ops();

    debug_msg("[Debug check]: get_cur_date...");

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    debug_msg("[Debug check]: done\n");

    ret = tax_sys_is_fiscal_init();
    if (ret != POSITIVE) {
        return -ETAX_FISCAL_NOT_INIT;
    }

    memset(&sys_cfg_rec, 0, sizeof(sys_cfg_rec));

    ret = tax_file_read_sys_cfg(&sys_cfg_rec);
    if (ret != SUCCESS) {
        return ret;
    }

    debug_msg("[Debug check]: sys_is_lock...");

    if (sys_cfg_rec.is_lock == POSITIVE) {
        return -ETAX_MACH_LOCKED;
    }

    debug_msg("[Debug check]: done.\n");

    if (rt_ops->cmp_bcd_date(&today, &gp_app_info->issue_limit_date) > 0) {
        return -ETAX_ISSUE_LIMIT;
    }

    ret = tax_base_check_lock_date(&today);
    if (ret < 0)
        return -ETAX_MACH_LOCKED;

    debug_msg("[Debug check]: base invoice check...");

    ret = tax_base_invoice_check();
    if (ret != SUCCESS) {
        return ret;
    }

    debug_msg("[Debug check]: done\n");

    ret = is_fiscal_card_ready();
    if (ret == NEGATIVE)
        return -ETAX_FC_NOT_READY;

    ret = tax_file_is_full(TODAY_ID_FILE);
    if (ret == POSITIVE) 
        return -ETAX_INVOICE_FULL;

    ret = tax_file_is_full(ROLL_ID_FILE);
    if (ret == POSITIVE)
        return -ETAX_INVOICE_FULL;

    debug_msg("[Debug check]: read amount...");

    /* 
     * amount limit check at last 
     */
    memset(&amt_record, 0, sizeof(amt_record));
    ret = tax_file_read_amount(&amt_record);
    if (ret < 0 && ret != -EFILE_NO_REC) 
        return ret;

    debug_msg("[Debug check]: done\n");

    if (gp_app_info->total_invoice_limit == amt_record.amt_total_this)
        return -ETAX_AMT_TOTAL_LIMIT;

    if (gp_app_info->return_invoice_limit == amt_record.amt_return_this)
        return -ETAX_AMT_RETURN_LIMIT;

    return SUCCESS;
}

#if 0
/*
 * USE tax_sys_power_on_check()) instead 
 *
 * This method will be invoked while system booting up,
 * so we just check the HW status for this time, we will
 * check the card's status later.
 *
 * -- 2014.8.05 Leonardo Physh Rev01
 */
static int tax_sys_hw_init(void)
{
    int ret;

#if CONFIG_DEBUG 
    DEBUG("TAX_SYSTEM HW CHECK.\n");
#endif     

    /* Check card device is ok */
    ret = access(CARD_DEV, F_OK);   //dese CARD_DEV exsit?
    if (ret < 0) {
        err_num = -ETAX_NO_CARD_DEV;
        /* 
         * the CARD_DEV dese not exist.
         * 'cause this is a very bad situation,  do something handle it.
         */
        DEADLY_BUG(err_num);
    }

    ret = access(CARD_DEV, R_OK | W_OK); //if CARD_DEV can be read & write?
    if (ret < 0) {
        err_num = -ETAX_PERM_DENY;
        /* 
         * if access check failed, means we are not allowed to operate CARD_DEV,
         * meanwhile, there are something wrong with linux system, this is a 
         * very bad bug.
         */
        DEADLY_BUG(err_num);
    }

    return SUCCESS;
}
#endif 


#if 0
/* tax_sys_power_on_check() will do this 
 * 
 * Check whether tax system function is OK or not.
 * 2014.8.07 Rev01
 */
static int tax_sys_func_init(void)
{
    int ret;

    struct fiscal_card * fiscal_card;
    struct user_card * user_card;
    struct tax_system * tax_system;

    tax_system = get_tax_system();

#if CONFIG_DEBUG 
    DEBUG("TAX_SYSTEM FUNCTION CHECK.\n");
#endif 

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER;

    /* check both fiscal card and user card */
    ret = fiscal_card->powre_on(FISCAL_CARD);
    if (ret != SUCCESS) {
        fiscal_card->power_off(FISCAL_CARD);
        return ret;
    }

    ret = user_card->power_on(USER_CARD);
    if (ret != SUCCESS) {
        user_card->power_off(USER_CARD);
        return ret;
    }

    /*
     * Read something from fiscal card and user card
     * ATTENTION : 
     *   Beacuase end usre may pull cards out while machine under
     * using, so checking card's status whenever needs to operate 
     * cards.
     */
    ret = fiscal_card->powre_rst(FISCAL_CARD);
    if (ret != SUCCESS) {
        fiscal_card->power_off(FISCAL_CARD);
        return ret;
    }

    ret = fiscal_card_is_ready();
    if (ret != POSITIVE) {
        fiscal_card->power_off(USER_CARD);
        return ret;
    }

    ret = user_card->power_rst(USER_CARD);
    if (ret != SUCCESS) {
        user_card->power_off(USER_CARD);
        return ret;
    }
    ret = user_card_is_ready();
    if (ret != POSITIVE) {
        user_card->power_off(USER_CARD);
        return ret;
    }

    /* 
     * if we got here, indicate that fiscal car and 
     * user card is ready, however, for avoiding mis-operation,
     * poweroff cards.
     */
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

#if CONFIG_DEBUG 
    DEBUG("TAX_SYSTEM FUNCTION CHECK OK.\n");
#endif     

    tax_system->init_mark = POSITIVE;

    return SUCCESS:
}
#endif 


/*
 * tax_sys_fiscal_pre_init - check system status
 *                            
 *  - before doing fiscal initialize, call this method 
 *  to do check if system is ready for starting fiscal.
 *  
 *  @return : status 
 */
static int tax_sys_fiscal_pre_init(void)
{
    int ret;

    struct fiscal_card * fiscal_card;
    struct user_card * user_card;

    struct machine_info_record * mach_info = get_mach_info();
    ret = tax_file_read_mach_info(mach_info);
    if (ret < 0)
        return ret;

    debug_msg("TAX_SYSTEM FISCAL PRE INTI.\n");

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    /*
     * We will not check user permission here, plz do 
     * check job before calling this method.
     *
     * We will not check fiscal in using or not, plz do
     * check job before calling this method.
     */
    ret = fiscal_card_power_on();
    if (ret != SUCCESS) 
        goto card_off;

    ret = is_fiscal_card_ready();
    if (ret != POSITIVE) {
        ret =  -ETAX_FC_NOT_READY;
        goto card_off;
    }

    ret = is_fiscal_card_register();
    if (ret != NEGATIVE) {
        ret = -ETAX_FC_HAS_REG;
        goto card_off;
    }

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;


    ret = is_user_card_ready();
    if (ret != POSITIVE) {
        ret =  -ETAX_UC_NOT_READY;
        goto card_off;
    }

    /*
     * check mechine number from fiscal card
     * and compare it with local mechine nb.
     */
    uchar machine_nb[8];
    ret = fiscal_card_get_machine_nb(machine_nb);
    if (ret < 0) 
        goto card_off;

    /*
     * get machine number from local flash 
     */
    if (memcmp(machine_nb, mach_info->machine_nb, 8)) {
        ret =  -ETAX_DIFF_MACH_NB;
        goto card_off;
    }

    /*
     * get fiscal card numer, and check it with user card
     */
    uchar fis_card_nb[8];
    ret  = fiscal_card_get_card_nb(fis_card_nb);
    if (ret < 0) 
        goto card_off;

    /*
     * get taxpayer number 
     */
    uchar taxpayer_nb[8];
    ret = fiscal_card_get_taxpayer_nb(taxpayer_nb);
    if (ret < 0) 
        goto card_off;

    /*
     * check usercard, whatever success or failed, 
     * we both need to power off fiscal & user card. 
     */
    ret = user_card_check_card(taxpayer_nb, machine_nb, fis_card_nb);
    if (ret != SUCCESS) {
        ret =  -ETAX_UC_NOT_MATCH;
        goto card_off;
    }

    /*
     * if we get here, then all prepare works are done, we can
     * intialize the fiscal function for now. 
     */
    debug_msg("TAX_SYSTEM FISCAL PRE INTI DONE.\n");
    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_sys_fiscal_init - fiscal system intializetion
 *                       one machine just need to be 
 *                       initialize for once. if mac-
 *                       hine is transfer to another, 
 *                       then need to do this again.
 *  @return : status
 */
static int tax_sys_fiscal_init(void)
{
    int ret;

    struct fiscal_card * fiscal_card;
    struct user_card * user_card;
    struct rt_operate * rt_operate = get_rt_ops();

    debug_msg("TAX_SYSTEM FISCAL INTI.\n");

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    /*
     * most of checking jobs are done in pre_init
     */
    ret = tax_sys_fiscal_pre_init();
    if (ret != SUCCESS) {
        /* 
         * cards has been power off in pre_init 
         */
        return ret;
    }

    ret = fiscal_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    ret = user_card_power_on();
    if (ret != SUCCESS)
        goto card_off;

    /*
     * Get tax system application information
     * there are many element of tax_sys_app_info, 
     * and they are useful, check it carefuly.
     */
    struct tax_sys_app_info app_info;
    ret = rt_operate->get_cur_date(&app_info.init_date);
    if (ret < 0)
        return ret;

    /* get tax application information */
    ret = fiscal_card_get_app_info(&app_info);
    if (ret < 0) 
        goto card_off;

    /* 
     * app_start_date <= init_date <= issue_limit_date <= app_vaild_date
     */
    ret = tax_base_check_date(&app_info);
    if (ret < 0) 
        goto card_off;

    /* get fiscal type and rate */
    ret = user_card_get_fis_type(&app_info);
    if (ret < 0) 
        goto card_off;

#if CONFIG_PWR_OFF_PROTECT
    /*
     * NOTICE:
     *   the following code have to run with power on.
     *   before running below code, check power state. if power is off 
     *   while we get here, DO NOT ANYTHING, and if power off after we
     *   last time we checked it's state, don't warry about that, we still 
     *   get 5s to finish our jobs.
     */
    struct power_state * pm = get_power_state();

    pm->check_power_state();
#endif 

    /* if power is off, we are not allowed to get here */

    /*
     * fiscal control information will store in local disk
     * so we should to create all fils we need here.
     */
    ret = tax_file_creat_fiscal_file();
    if (ret != SUCCESS) 
        goto card_off;

    /* GET_REGISTER_NB CMD*/
    struct register_info reg_info;
    ret = fiscal_card->get_register_nb(&reg_info);
    if (ret < 0) 
        goto card_off;

    /* REGISTER SIGN*/
    ret = user_card->register_sign(&reg_info);
    if (ret < 0) 
        goto card_off;

    /* TERMINAL_REGISTER CMD */
    ret = fiscal_card->terminal_register(&reg_info);
    if (ret < 0) 
        goto card_off;

    /* save pin to file */
    ret  = tax_file_save_pin((struct tax_sys_pin_record *)&reg_info.pin);
    if (ret < 0) 
        goto card_off;

    /* save origin pin to file */
    ret = tax_file_save_origin_pin((struct tax_sys_pin_record *)&reg_info.pin);
    if (ret < 0)
        goto card_off;

    /* save fiscal configure file */
    struct tax_sys_fis_config_record fis_cfg = {
        .declare_flag = NEGATIVE,
        .pin_lock_flag = NEGATIVE,
        .dist_err_flag = NEGATIVE,
    };

    ret = tax_file_save_fis_cfg(&fis_cfg);
    if (ret < 0) 
        goto card_off;

    /* save app information */
    ret = tax_file_save_app_info(&app_info);
    if (ret < 0) 
        goto card_off;

    /* save system configure file */
    struct tax_sys_config_record sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_cfg));

    sys_cfg.is_init = POSITIVE;

    ret = tax_file_save_sys_cfg(&sys_cfg);
    if (ret < 0) 
        goto card_off;

    debug_msg("TAX_SYSTEM FISCAL INTI.\n");
    ret = SUCCESS;

card_off:
    fiscal_card->power_off(FISCAL_CARD);
    user_card->power_off(USER_CARD);

    return ret;
}

/*
 * tax_system_card_init - initialize card 
 *  @return : status 
 */
int tax_sys_card_init(void)
{
    int ret;

    struct fiscal_card * fiscal_card;
    struct user_card * user_card;

    debug_msg("TAX_SYSTEM CARD INTI.\n"); 

    fiscal_card = get_fiscal_card();
    if (!fiscal_card)
        return -ETAX_NUL_FISCAL_CARD;

    user_card = get_user_card();
    if (!user_card)
        return -ETAX_NUL_USER_CARD;

    /*
     * FC UC used the same uart, so just needs to do 
     * initial for once.
     */
    ret = fiscal_card->device_init();
    if (ret < 0) {
        debug_msg("Card Reader INIT Failed\n");
        return FAIL;
    }

    return SUCCESS;
}

/*
 * API to other modules 
 */
static struct tax_system g_tax_system_base = {
    .is_fiscal_init = tax_sys_is_fiscal_init,
    .get_invoice_nb = tax_sys_get_invoice_nb,
    .get_buy_roll = tax_sys_get_buy_roll,
    .dist_invoice = tax_sys_dist_invoice,
    .mount_roll = tax_sys_mount_roll,
    .daily_collect = tax_sys_daily_collect,
    .issue_invoice = tax_sys_issue_invoice_proc,
    .declare_duty = tax_sys_declare_duty,
    .redeclare_duty = tax_sys_redeclare_duty,
    .check_finish_tax = tax_sys_check_finish_tax,
    .update_control = tax_sys_update_control,
    .update_taxpayer = tax_sys_update_taxpayer,
    .power_on_check = tax_sys_power_on_check,
    .transact_prepare = tax_sys_transact_prepare,

    .card_init = tax_sys_card_init,
    .fiscal_init = tax_sys_fiscal_init,

    .get_last_declare_date = tax_base_get_last_declare_date,
    .check_declare_date = tax_base_check_declare_date,

    .get_check_info = tax_base_get_check_info,
    .verify_check_card = tax_base_verify_check_card,
    .write_check_declare_detail = tax_sys_write_check_declare_detail,
    .write_check_daily_detail = tax_sys_write_check_daily_detail,
    .write_check_invoice_detail = tax_sys_write_check_invoice_detail,
};

struct tax_system * get_tax_system(void)
{
    return &g_tax_system_base;
}


/* End of tax_system.c */

