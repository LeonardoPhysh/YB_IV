/*
 * com_other_ops.c
 *  - implement function of other sub-menu "其他"
 * 
 * Author : Leonardo Phsy 
 * Date   : 2014.9.24 Rev01
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "tax_cmd.h"
#include "tax_system.h"
#include "tax_file_op.h"
#include "ui_api.h"
#include "plu.h"
#include "print.h"
#include "input.h"
#include "common.h"
#include "config.h"
#include "ui_api.h"


/* 
 * UI Heper 
 */
int show_view_single_inv_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 2;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "查询单张发票信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "发票号: ");

    show_simple_frame(&frame);
    return SUCCESS;
}

int show_view_period_collect_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "发票使用信息查询");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "起始日期: ");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "终止日期: ");
    
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "日期格式:20150101 ");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

int show_view_used_roll_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 2;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "查询已使用卷信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "请输入该卷中任意发票号：");

    show_simple_frame(&frame);
    return SUCCESS;
}

int show_view_declare_info_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "查询申报信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "请输入日期: ");
       
    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "日期格式:20150101 ");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

int show_view_daily_collect_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "查询日汇总信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "请输入日期: ");
       
    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "日期格式:20150101 ");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

int show_develop_sys_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "开发者选项");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "请输入机器编号(16 bits): ");
       
    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "（出厂日期默认为今日）");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

int show_develop_print_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "打印机支持设置");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "1-仅支持平推打印机");
       
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "2-支持平推及卷式打印机");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

int show_print_setup_UI(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "默认打印机");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "1-平推打印机");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "2-卷式打印机");

    show_simple_frame(&frame);
    return SUCCESS;
}

/*
 * Content UI 
 */
int show_single_inv_info(struct tax_sys_invoice_detail_record * ori_detail)
{
    int key, pos, item;
    char buf[20] = {0};
    char *fis_name = NULL;
    struct simple_frame frame[3];

    memset(frame, 0, 3 * sizeof(struct simple_frame));

    frame[0].item_num = 5; 
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 1;
    bcd_to_str((uchar *)&ori_detail->detail_date, buf, 4);
    sprintf(frame[0].items[0].title, "%s", buf);

    frame[0].items[1].pos.row = 1;
    frame[0].items[1].pos.col = 10;
    sprintf(frame[0].items[1].title, "%s",
            ori_detail->detail_type == NORMAL_INVOICE ? "正常票" : 
            ori_detail->detail_type == RETURN_INVOICE ? "退票" :
            ori_detail->detail_type == SPOIL_INVOICE ? "废票" : 
            "未知票");

    frame[0].items[2].pos.row = 2;
    frame[0].items[2].pos.col = 1;
    sprintf(frame[0].items[2].title, "开票总金额：%.2f元", ori_detail->detail_amt_total/100.0);

    frame[0].items[3].pos.row = 3;
    frame[0].items[3].pos.col = 1;
    sprintf(frame[0].items[3].title, "发票号:%08u", ori_detail->detail_inv_num);

    frame[0].items[4].pos.row = 4;
    frame[0].items[4].pos.col = 1;
    if (ori_detail->detail_type == RETURN_INVOICE) {
        sprintf(frame[0].items[4].title, "%s：%08u", "原发票号:", ori_detail->detail_ori_inv_num);
    } else {
        sprintf(frame[0].items[4].title, "%s", "原发票号: 无");
    }

    memset(buf, 0, 20);

    frame[1].item_num = 3;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 1;
    sprintf(frame[1].items[0].title, "税控码:");

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 2;
    sprintf(frame[1].items[1].title, "%s", ori_detail->detail_fiscal_code);

    frame[1].items[3].pos.row = 3;
    frame[1].items[3].pos.col = 1;
    sprintf(frame[1].items[0].title, "付款单位：%s", 
            strlen(ori_detail->payer_name) > 0 ? ori_detail->payer_name : "无");

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();

    pos = 0;
    item = 0;
    while (1) {
        if (ori_detail->detail_type != SPOIL_INVOICE) {
            frame[2].item_num = 5;
            frame[2].items[0].pos.row = 1;
            frame[2].items[0].pos.col = 5;
            sprintf(frame[2].items[0].title, "条目 %d/%d", item + 1, ori_detail->item_num);

            frame[2].items[1].pos.row = 2;
            frame[2].items[1].pos.col = 1;
            sprintf(frame[2].items[1].title, "数量：%d", ori_detail->item[item].num);

            frame[2].items[2].pos.row = 2;
            frame[2].items[2].pos.row = 8;
            sprintf(frame[2].items[2].title, "单价:%.2f", ori_detail->item[item].comm_price/100.0);

            frame[2].items[3].pos.row = 3;
            frame[2].items[3].pos.row = 1;
            sprintf(frame[2].items[3].title, "合计: %.2f", ori_detail->item[item].amount/100.0);

            frame[2].items[4].pos.row = 4;
            frame[2].items[4].pos.row = 1;
            fis_name = get_fis_type_name(ori_detail->item[item].comm_tax_index);
            sprintf(frame[2].items[4].title, "税种%d:%s(%d)", item, 
                    fis_name != NULL ? fis_name : "未知名字",
                    ori_detail->item[item].comm_tax_index);
        } else {  
            frame[2].item_num = 1;
            frame[2].items[0].pos.row = 1;
            frame[2].items[0].pos.col = 5;
            sprintf(frame[2].items[0].title, "条目 0/0");
        }

show_ui:
        show_simple_frame(&frame[pos]);

        key = get_keycode();
        switch(key) {
            case BACK:
                return SUCCESS;
                break;

            case ENTER:
                return SUCCESS;
                break;

            case ESC:
                return SUCCESS;
                break;

            case UP:
                if (pos == 1) {
                    pos --;
                    goto show_ui;
                } else if (pos == 2) {
                    if (item == 0) {
                        pos --;
                        goto show_ui;
                    } else {
                        item --;
                        continue;
                    }
                }

                break;

            case DOWN:
                if (pos == 0 || pos == 1) {
                    pos ++;
                    goto show_ui;
                } else if (pos == 2) {
                    if (item == 0) {
                        if (item + 1 < ori_detail->item_num) {
                            item ++;
                            continue;
                        }
                    }
                }

                break;

            default:
                break;
        }
    }
}

int show_dayly_collect_info(struct tax_sys_daily_collect_record * daily_rec)
{
    int key, pos, item;
    char *fis_name;
    char buf[20] = {0};
    struct simple_frame frame[2];
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    memset(frame, 0, 2 * sizeof(struct simple_frame));

    frame[0].item_num = 4;
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 1;
    bcd_to_str((uchar *)&daily_rec->cur_date, buf, 4);
    sprintf(frame[0].items[0].title, "日期：%s", buf);

    frame[0].items[1].pos.row = 2;
    frame[0].items[1].pos.col = 1;
    sprintf(frame[0].items[1].title, "正常开票数：%d", daily_rec->valid_count);

    frame[0].items[2].pos.row = 3;
    frame[0].items[2].pos.col = 1;
    sprintf(frame[0].items[2].title, "退票数：%d", daily_rec->return_count);

    frame[0].items[3].pos.row = 4;
    frame[0].items[3].pos.col = 1;
    sprintf(frame[0].items[3].title, "废票数：%d", daily_rec->spoil_count);

    frame[1].item_num = 4;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 4;

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 1;

    frame[1].items[2].pos.row = 3;
    frame[1].items[2].pos.col = 1;

    frame[1].items[3].pos.row = 4;
    frame[1].items[3].pos.col = 1;

    pos = item = 0;
    while (1) {
        sprintf(frame[1].items[0].title, "各税种开票金额");

        fis_name = get_fis_type_name(daily_rec->tax_index[item]);
        sprintf(frame[1].items[1].title, "税种%d:%s(%d)", item, 
                fis_name != NULL ? fis_name : "未知名字",
                daily_rec->tax_index[item]);

        sprintf(frame[1].items[2].title, "开票总额：%d", daily_rec->amt_valid[item]);
        sprintf(frame[1].items[3].title, "退票总额：%d", daily_rec->amt_return[item]);

show_ui:
        show_simple_frame(&frame[pos]);

        key = get_keycode();
        switch (key) {
            case ENTER:
                return SUCCESS;
                break;

            case BACK:
                return -EUI_BACK;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case UP:
                if (pos > 0) {
                    if (item == 0) {
                        pos --;
                        goto show_ui;
                    } else 
                        item --;
                }
                break;

            case DOWN:
                if (pos == 0) {
                    pos ++;
                    goto show_ui;
                } else {
                    if (item + 1 < gp_app_info->tax_item_nb) {
                        item ++;
                    }
                }
                break;

            default:
                break; 
        }
    }
}

int show_declare_info(struct tax_sys_declare_duty_record * declare_rec)
{
    int key, pos, item;
    char *fis_name;
    char buf[20] = {0};
    struct simple_frame frame[3];
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    memset(frame, 0, 3 * sizeof(struct simple_frame));

    frame[0].item_num = 4;
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 5;
    sprintf(frame[0].items[0].title, "申报信息");

    frame[0].items[1].pos.row = 2;
    frame[0].items[1].pos.col = 1;
    bcd_to_str((uchar *)&declare_rec->cur_date, buf, 4);
    sprintf(frame[0].items[1].title, "申报日期：%s", buf);

    frame[0].items[2].pos.row = 3;
    frame[0].items[2].pos.col = 1;
    bcd_to_str((uchar *)&declare_rec->start_date, buf, 4);
    sprintf(frame[0].items[2].title, "起始日期：%s", buf);

    frame[0].items[3].pos.row = 4;
    frame[0].items[3].pos.col = 1;
    bcd_to_str((uchar *)&declare_rec->end_date, buf, 4);
    sprintf(frame[0].items[3].title, "截止日期：%s", buf);

    frame[1].item_num = 4;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 5;
    sprintf(frame[1].items[1].title, "申报信息");

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 1;
    sprintf(frame[1].items[1].title, "正常票数：%d", declare_rec->valid_count);

    frame[1].items[2].pos.row = 3;
    frame[1].items[2].pos.col = 1;
    sprintf(frame[1].items[1].title, "退票数：%d", declare_rec->return_count);

    frame[1].items[3].pos.row = 4;
    frame[1].items[3].pos.col = 1;
    sprintf(frame[1].items[1].title, "废票数：%d", declare_rec->spoil_count);

    frame[2].item_num = 4;
    frame[2].items[0].pos.row = 1;
    frame[2].items[0].pos.col = 4;

    frame[2].items[1].pos.row = 2;
    frame[2].items[1].pos.col = 1;

    frame[2].items[2].pos.row = 3;
    frame[2].items[2].pos.col = 1;

    frame[2].items[3].pos.row = 4;
    frame[2].items[3].pos.col = 1;

    pos = item = 0;
    while (1) {
        sprintf(frame[2].items[0].title, "各税种开票金额");

        fis_name = get_fis_type_name(declare_rec->tax_index[item]);
        sprintf(frame[2].items[1].title, "税种%d:%s(%d)", item, 
                fis_name != NULL ? fis_name : "未知名字",
                declare_rec->tax_index[item]);

        sprintf(frame[2].items[2].title, "开票总额：%d", declare_rec->amt_valid[item]);
        sprintf(frame[2].items[3].title, "退票总额：%d", declare_rec->amt_return[item]);

show_frame:
        show_simple_frame(&frame[pos]);

        key = get_keycode();
        switch (key) {
            case ENTER:
                return SUCCESS;
                break;

            case BACK:
                return -EUI_BACK;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case UP:
                if (pos > 0) {
                    if (item == 0) {
                        pos --;
                        goto show_frame;
                    } else 
                        item --;
                }
                break;

            case DOWN:
                if (pos < 2) {
                    pos ++;
                    goto show_frame;
                } else {
                    if (item + 1 < gp_app_info->tax_item_nb) {
                        item ++;
                    }
                }
                break;

            default:
                break; 
        }
    }
}


int show_used_roll_info(struct tax_sys_used_roll_id_record * used_roll)
{
    int key, pos;
    char buf[40] = {0};
    struct simple_frame frame[3];

    memset(frame, 0, 3 * sizeof(struct simple_frame));

    frame[0].item_num = 4;
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 1;
    sprintf(frame[0].items[0].title, "起始号：%d", used_roll->start_num);

    frame[0].items[1].pos.row = 2;
    frame[0].items[1].pos.col = 1;
    sprintf(frame[0].items[1].title, "终止号：%d", used_roll->end_num);

    frame[0].items[2].pos.row = 3;
    frame[0].items[2].pos.col = 1;
    bcd_to_str((uchar *)&used_roll->start_date, buf, 4);
    sprintf(frame[0].items[2].title, "起始日期：%s", buf);

    frame[0].items[3].pos.row = 4;
    frame[0].items[3].pos.col = 1;
    bcd_to_str((uchar *)&used_roll->end_date, buf, 4);
    sprintf(frame[0].items[3].title, "终止日期：%s", buf);

    frame[1].item_num = 4;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 1;
    bcd_to_str(used_roll->invoice_code, buf, 10);
    sprintf(frame[1].items[0].title, "代码：%s", buf);

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 1;
    sprintf(frame[1].items[1].title, "正常票数：%d", used_roll->valid_count);

    frame[1].items[2].pos.row = 3;
    frame[1].items[2].pos.col = 1;
    sprintf(frame[1].items[2].title, "退票数：%d", used_roll->return_count);

    frame[1].items[3].pos.row = 4;
    frame[1].items[3].pos.col = 1;
    sprintf(frame[1].items[3].title, "废票数：%d", used_roll->spoil_count);

    frame[2].item_num = 2;
    frame[2].items[0].pos.row = 1;
    frame[2].items[0].pos.col = 5;
    sprintf(frame[2].items[0].title, "正常票总额：%.2f", used_roll->amt_valid/100.0);

    frame[2].items[1].pos.row = 2;
    frame[2].items[1].pos.col = 1;
    sprintf(frame[2].items[1].title, "退票总额：%.2f", used_roll->amt_return/100.0);

    pos = 0;
    while (1) {
        show_simple_frame(&frame[pos]);

        key = get_keycode();
        switch(key) {
            case ENTER:
                return SUCCESS;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case BACK:
                return -EUI_BACK;
                break;

            case UP:
                if (pos > 0)
                    pos --;

                break;

            case DOWN:
                if (pos < 2)
                    pos ++;

                break;

            default:
                break;
        }
    }
}

int show_period_collect(struct tax_sys_period_collect_record * collect_rec)
{
    int key, pos;
    char buf_t[20] = {0};
    char buf_b[20] = {0};
    struct simple_frame frame[2];

    memset(frame, 0, 2 * sizeof(struct simple_frame));

    frame[0].item_num = 4;
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 3;
    sprintf(frame[0].items[0].title, "发票使用汇总信息");

    frame[0].items[1].pos.row = 2;
    frame[0].items[1].pos.col = 1;
    bcd_to_str((uchar *)&collect_rec->start_date, buf_t, 4);
    bcd_to_str((uchar *)&collect_rec->end_date, buf_b, 4);
    sprintf(frame[0].items[1].title, "%s -- %s", buf_t, buf_b);

    frame[0].items[2].pos.row = 3;
    frame[0].items[2].pos.col = 1;
    sprintf(frame[0].items[2].title, "正常票数：%d", collect_rec->valid_count);

    frame[0].items[3].pos.row = 4;
    frame[0].items[3].pos.col = 1;
    sprintf(frame[0].items[3].title, "金额：%.2f", collect_rec->amt_valid/100.0);

    frame[1].item_num = 4;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 1;
    sprintf(frame[1].items[0].title, "退票数：%d", collect_rec->return_count);

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 1;
    sprintf(frame[1].items[1].title, "金额：%.2f", collect_rec->amt_return/100.0);

    pos = 0;
    while (1) {
        show_simple_frame(&frame[pos]);

        key = get_keycode();
        switch(key) {
            case ENTER:
                return SUCCESS;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case BACK:
                return -EUI_BACK;
                break;

            case UP:
                if (pos > 0)
                    pos --;

                break;

            case DOWN:
                if (pos < 1)
                    pos ++;

                break;

            default:
                break;
        }
    }
}

int do_check_user_and_date(struct bcd_date * date)
{
    int ret;
    struct bcd_date today;
    struct bcd_date declare_date;

    struct user *cur_user = get_cur_user();
    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    ret = tax_system->get_last_declare_date(&declare_date);
    if (ret < 0)
        return ret;

    if (cur_user->level == NORMAL_USER) {
        /* normal user can only view today's invoice */
        ret = rt_ops->cmp_bcd_date(&today, date);
        if (ret != 0)
            return 1;
    } else if (cur_user->level == SUPPER_USER) {
        /* supper user can only view invoice between declare */
        ret = rt_ops->cmp_bcd_date(&declare_date, date);
        if (ret > 0) 
            return 2;
    }

    return SUCCESS;
}

int do_check_start_and_end(struct bcd_date * start_date, struct bcd_date * end_date)
{
    int ret;
    struct bcd_date today;
    struct bcd_date declare_date;

    struct user *cur_user = get_cur_user();
    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    ret = tax_system->get_last_declare_date(&declare_date);
    if (ret < 0)
        return ret;

    if (rt_ops->cmp_bcd_date(start_date, end_date) > 0) 
        return 1;

    if (rt_ops->cmp_bcd_date(&today, end_date) < 0)
        return 2;

    if (cur_user->level == SUPPER_USER) {
        /* supper user can only view invoice between declare */
        ret = rt_ops->cmp_bcd_date(&declare_date, start_date);
        if (ret > 0) 
            return 3;
    }

    return SUCCESS;
}

/*
 * cmd_view_single_inv - service for submenu "查询-单张发票"
 *  @return : status
 */
int cmd_view_single_inv(void)
{
    int ret;
    int ori_num; 
    struct tax_sys_invoice_detail_record ori_detail;
    struct print_sys * print_sys = get_print_sys();

    memset(&ori_detail, 0, sizeof(ori_detail));

input_ori:
    show_view_single_inv_UI();
    ret = get_inter_num(2, 5, &ori_num);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_ori;
    }

    display_info("正在查找发票信息...");

    ret = tax_file_find_invoice_detail((uint *)&ori_num, &ori_detail);
    if (ret == -EFILE_NO_REC) {
        display_warn("未找到发票信息，请确认发票号输入正确！");
        goto input_ori;
    }

    /* check invoice */
    ret = do_check_user_and_date(&ori_detail.detail_date);
    if (ret < 0) {
        display_warn("查找发票信息出错！");
        display_err_msg(ret);
        return FAIL;
    } else {
        if (ret == 1) {
            display_warn("您为普通用户，只能查询当天发票信息！");
            goto input_ori;
        } else if (ret == 2) {
            display_warn("您为超级用户，只能查询申报期内发票信息！");
            goto input_ori;
        }     
    }

    show_single_inv_info(&ori_detail);

    if (print_sys->ops->print_invoice_stub) {
        ret = question_user("是否打印电子发票存根？");
        if (ret == POSITIVE) {
            display_info("正在打印打电子发票存根...");
            print_sys->ops->print_invoice_stub(&ori_detail);
            sleep(1);
            display_info("打印成功！");
        }
    }
    
    get_anykey();
    return SUCCESS;
}


/*
 * cmd_view_inv_collect - service for submenu "查询-阶段查询"
 *  @return : status 
 */
int cmd_view_period_collect(void)
{
    int ret;
    struct bcd_date today;
    struct bcd_date start_date, end_date;
    struct tax_sys_period_collect_record collect_rec;
    struct tax_sys_daily_collect_record tmp_rec;
    struct user *cur_user = get_cur_user();
    struct rt_operate * rt_ops = get_rt_ops();
    struct print_sys * print_sys = get_print_sys();

    memset(&collect_rec, 0, sizeof(collect_rec));
    memset(&tmp_rec, 0, sizeof(tmp_rec));

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0) {
        display_warn("查询发票信息失败！");
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_file_read_last_record(DAILY_COLLECT_FILE, (uchar *)&tmp_rec, sizeof(tmp_rec));
    if (ret < 0) {
        display_warn("查询发票信息失败！");
        display_err_msg(ret);
        return FAIL;
    }

    /* normal user, limited upto today */
    if (cur_user->level == NORMAL_USER) { 
        if (rt_ops->cmp_bcd_date(&tmp_rec.cur_date, &today) != 0) {
            display_warn("今日尚无可查信息！");
            return SUCCESS;
        }
    }

show_ui: 
    show_view_period_collect_UI();
    ret = get_bcd_date(2, 6, &start_date);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else {         
            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));

            goto show_ui;
        } 
    } else if (ret == -EUI_BAD_DATE_FORMAT) {
        display_warn("请按照正确日期格式输入！");

        memset(&start_date, 0, sizeof(start_date));
        memset(&end_date, 0, sizeof(end_date));
        goto show_ui;
    }

    ret = get_bcd_date(3, 6, &end_date);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else {  
            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));
            goto show_ui;
        } 
    }else if (ret == -EUI_BAD_DATE_FORMAT) {
        display_warn("请按照正确日期格式输入！");

        memset(&start_date, 0, sizeof(start_date));
        memset(&end_date, 0, sizeof(end_date));
        goto show_ui;
    }

    /* check input date */
    ret = do_check_start_and_end(&start_date, &end_date);
    switch(ret) {
        case 1:
            display_warn("终止日期不能小于起始日期！");

            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));
            goto show_ui;
            break;

        case 2:
            display_warn("终止日期不能大于今日！");

            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));
            goto show_ui;
            break;

        case 3:
            display_warn("您只能查询本申报期数据！");

            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));
            goto show_ui;
            break;

        default:
            break;
    }

    collect_rec.start_date = start_date;
    collect_rec.end_date = end_date;
    ret = tax_file_find_period_collect(&collect_rec);
    if (ret < 0) {
        if (ret == -EFILE_NO_REC) {
            display_warn("未找到发票信息，请重新输入日期！");

            memset(&start_date, 0, sizeof(start_date));
            memset(&end_date, 0, sizeof(end_date));
            goto show_ui;
        } else {
            display_warn("查询发票信息失败！");
            display_err_msg(ret);
            return FAIL;
        }
    }

    show_period_collect(&collect_rec);
    
    if (print_sys->ops->print_period_info) {
        ret = question_user("是否打印发票使用信息？");
        if (ret == POSITIVE) {
            display_info("正在打印发票使用信息...");
            print_sys->ops->print_period_info(&collect_rec);
            sleep(1);
            display_info("打印成功！");
        }
    }
    
    get_anykey();
    return SUCCESS;
}


/*
 * cmd_view_inv_roll - service for submenu "查询-查询使用卷"
 *  @return : status 
 */
int cmd_view_inv_roll(void)
{
    int ret; 
    int rec_num, ori_num;

    struct bcd_date today;
    struct tax_sys_used_roll_id_record used_roll;
    struct rt_operate *rt_ops = get_rt_ops();
    struct user *cur_user = get_cur_user();
    struct print_sys * print_sys = get_print_sys();

    memset(&used_roll, 0, sizeof(used_roll));

    ret = rt_ops->get_cur_date(&today); 
    if (ret < 0)
        goto fail;

    rec_num = tax_file_get_rec_num(ROLL_COLLECT_FILE);
    if (rec_num < 0)
        goto fail;
    else if (rec_num == 0) {
        display_warn("尚无已使用卷！");
        return SUCCESS;
    } 

    if (cur_user->level == NORMAL_USER) {
        ret = tax_file_read_last_record(ROLL_COLLECT_FILE, (uchar *)&used_roll, sizeof(used_roll));
        if (ret < 0) 
            goto fail;    
        else if (rt_ops->cmp_bcd_date(&today, &used_roll.end_date) > 0) {
            display_warn("今日尚无已使用卷信息！");
            return SUCCESS;
        }
    }

show_ui:
    show_view_used_roll_UI();
    ret = get_inter_num(3, 1, &ori_num);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto show_ui;
    }

    display_info("正在查询...");

    memset(&used_roll, 0, sizeof(used_roll));
    ret = tax_file_find_used_roll_id(ori_num, &used_roll);
    if (ret != SUCCESS) {
        if (ret == -EFILE_NO_REC) {
            display_warn("未查询到发票卷信息，请重新输入！");
            goto show_ui;
        } else 
            goto fail;
    }

    ret = do_check_user_and_date(&used_roll.end_date);
    if (ret < 0) {
        display_warn("查找发票卷信息出错！");
        display_err_msg(ret);
        return FAIL;
    } else {
        if (ret == 1) {
            display_warn("您为普通用户，只能查询当天使用卷信息！");
            goto show_ui;
        } else if (ret == 2) {
            display_warn("您为超级用户，只能查询申报期内使用卷信息！");
            goto show_ui;
        }     
    }

    show_used_roll_info(&used_roll);
    
    if (print_sys->ops->print_roll_info) {
        ret = question_user("是否打印发票卷使用信息？");
        if (ret == POSITIVE) {
            display_info("正在打印发票卷使用信息...");
            print_sys->ops->print_roll_info(&used_roll);
            sleep(1);
            display_info("打印成功！");
        }
    }
    
    get_keycode();
    return SUCCESS;

fail:
    display_warn("查询使用卷信息失败！");
    display_err_msg(ret);
    return FAIL;
}

/*
 * cmd_view_declare_data - service for submenu "查询-查询申报信息"
 *  @return : status 
 */
int cmd_view_declare_data(void)
{
    int ret;
    int rec_num;
    struct bcd_date today, declare_date;
    struct tax_sys_declare_duty_record declare_rec;
    struct rt_operate *rt_ops = get_rt_ops();

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法继续操作！");
        return FAIL;
    }

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        goto fail;

    rec_num = tax_file_get_rec_num(DECLARE_DUTY_FILE);
    if (rec_num < 0)
        goto fail;
    else if (rec_num == 0) {
        display_warn("尚无申报数据可查！");
        return SUCCESS;
    } 

show_ui:
    show_view_declare_info_UI();
    ret = get_bcd_date(3, 1, &declare_date); 
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto show_ui;
    }

    if (rt_ops->cmp_bcd_date(&declare_date, &today) > 0) {
        display_warn("日期不能在今日之后！");
        goto show_ui;
    }

    ret = tax_file_find_declare_duty(&declare_date, &declare_rec);
    if (ret < 0) {
        if (ret == -EFILE_NO_REC) {
            display_warn("未找到申报信息，请重新输入日期！");
            goto show_ui;
        } else 
            goto fail;
    }

    show_declare_info(&declare_rec);

    get_anykey();
    return SUCCESS;

fail:
    display_warn("查询申报信息失败！");
    return ret;
}

/*
 * cmd_view_daily_collect - service for submenu "查询-日汇总"
 *  @return : status 
 */
int cmd_view_daily_collect(void)
{
    int ret;
    int rec_num;
    struct bcd_date today, collect_date;
    struct tax_sys_daily_collect_record collect_rec;
    struct rt_operate *rt_ops = get_rt_ops();

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法继续操作！");
        return FAIL;
    }

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        goto fail;

    rec_num = tax_file_get_rec_num(DAILY_COLLECT_FILE);
    if (rec_num < 0)
        goto fail;
    else if (rec_num == 0) {
        display_warn("尚无日汇总数据可查！");
        return SUCCESS;
    } 

show_ui:
    show_view_daily_collect_UI();
    ret = get_bcd_date(3, 1, &collect_date); 
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto show_ui;
    }

    if (rt_ops->cmp_bcd_date(&collect_date, &today) > 0) {
        display_warn("日期不能在今日之后！");
        goto show_ui;
    }

    ret = tax_file_find_daily_collect(&collect_date, &collect_rec);
    if (ret < 0) {
        if (ret == -EFILE_NO_REC) {
            display_warn("未找到日汇总信息，请重新输入日期！");
            goto show_ui;
        } else 
            goto fail;
    }

    show_dayly_collect_info(&collect_rec);

    get_anykey();
    return SUCCESS;

fail:
    display_warn("查询申报信息失败！");
    return ret;
}


/*
 * cmd_develop_sys - service for sub-menu "开发者选项"
 *  @return : status 
 */
int cmd_develop_sys(void)
{
    int ret;
    uint dec_data = 0;
    uchar bcd_data = 0;
    int key, print_nb, cur_print;
    char tmp[3] = {0};
    char mach_nb[16 + 1] = {0};
    struct bcd_date today;
    struct machine_info_record mach_info;
    struct file_operate * file_ops = get_file_ops();
    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();

    ret = check_userlevel(DEVELOP_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法继续操作！");
        return FAIL;
    }

    ret = tax_system->is_fiscal_init();
    if (ret == POSITIVE) {
        display_warn("本机已经初始化，无需出厂设置！");
        return FAIL;
    }

show_ui:
    show_develop_sys_UI();
    ret = get_string(3, 1, mach_nb);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消当前操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto show_ui;
    }

    if (strlen(mach_nb) != 16) {
        display_warn("机器编号必须为16Bits！");
        goto show_ui;
    }

setup_print:
    show_develop_print_UI();

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();
    while ((key = get_keycode()) != '1' && key != '2') {
        if (key == ESC) {
            ret = question_user("确定取消当前操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto setup_print;
        }
    }

    /* we set FLAT_PRINT_TYPE as default printer type 
     * and print_nb = 2 means we support two kinds of 
     * prnter <roll printer is embedded into machine> 
     */
    if (key == '1') {
        print_nb = 1;
        cur_print = FLAT_PRINT_TYPE;
    } else if (key == '2') {
        print_nb = 2;
        cur_print = FLAT_PRINT_TYPE;
    }

    display_info("正在生成出厂设置...");

    ret = file_ops->creat_file(MACH_INFO_FILE, MACH_INFO_FILE_MODE, MACH_INFO_REC_NUM);
    if (ret < 0) {
        display_warn("出厂设置出错！");
        display_err_msg(ret);
        return ret;
    }

    memset(&mach_info, 0, sizeof(mach_info));

    int i;
    for (i = 0; i < 8; i++) { 
        memcpy(tmp, mach_nb + 2 * i, 2);
        dec_data = atoi(tmp);
        dec_to_bcd(&dec_data, &bcd_data);
        mach_info.machine_nb[i] = bcd_data;
    }

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0) {
        display_warn("出厂设置出错！");
        display_err_msg(ret);
        return ret;
    }

    mach_info.print_nb = print_nb;
    mach_info.cur_print = cur_print;
    mach_info.produce_date = today;
    strcpy(mach_info.hw_version, HW_VERSION);
    strcpy(mach_info.sw_version, SW_VERSION);

    ret = tax_file_save_mach_info(&mach_info);
    if (ret != SUCCESS) {
        display_warn("出厂设置出错！");
        display_err_msg(ret);
        return ret;
    }

    display_warn("出厂设置成功！");
    return SUCCESS;
}

/*
 * cmd_system_restart - service for sub-menu "打印设置"
 *  @return : status 
 */
int cmd_print_setup(void)
{
    int ret;
    int fd, key, cur_print;
    struct machine_info_record mach_rec;

    ret = tax_file_read_mach_info(&mach_rec); 
    if (ret < 0) {
        display_warn("打印设置出错！");
        display_err_msg(ret);
        return FAIL;
    }

show_ui:
    show_print_setup_UI();

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();
    while ((key = get_keycode()) != '1' && key != '2') {
        if (key == ESC) {
            ret = question_user("确定取消当前操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_ui;
        }
    }

    if (key == '1') {
        fd = open(PRINT_DEV, O_RDWR);
        if (fd < 0) {
            display_warn("平推打印机未就绪！");
            goto show_ui;
        }
        close(fd);
        cur_print = FLAT_PRINT_TYPE;
    } else if (key == '2') {
        if (mach_rec.print_nb == 1) {
            display_warn("本机仅支持平推打印机！");
            goto show_ui;
        }
        cur_print = ROLL_PRINT_TYPE;
    }

    mach_rec.cur_print = cur_print;
    tax_file_save_mach_info(&mach_rec);

    display_warn("设置成功！");
    return SUCCESS; 
}

/*
 * cmd_system_restart - service for sub-menu "重新启动"
 *  @return : status
 */
int cmd_system_restart(void)
{
    int ret;

    ret = question_user("确定重新启动机器？");
    if (ret == POSITIVE) {
        display_info("正在重新启动...");

        usleep(100000);
        sync();
        ret = system("reboot");
        if (ret < 0)
            return FAIL;
    }

    return SUCCESS;
}

/*
 * cmd_system_logout - service for sub-menu "注销"
 *  @return : status 
 */
int cmd_system_logout(void)
{
    int ret;

    ret = question_user("确定注销登陆？");
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在注销...");

    sync();
    set_logout_flag(POSITIVE);

    return SUCCESS;
}

/*
 * cmd_view_safe_log - service for submenu "查询-安全日志"
 *  @return : status 
 */
int cmd_view_safe_log(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}


/*
 * cmd_demo_fiscal_init - service for submenu "培训-税控初始化"
 *  @return : status 
 */
int cmd_demo_fiscal_init(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_dist_roll - service for submenu "培训-分发发票"
 *  @return : status 
 */
int cmd_demo_dist_roll(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_mount_roll - service for submenu "培训-挂载发票"
 *  @return : status 
 */
int cmd_demo_mount_roll(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_issue_inv - service for submenu "培训-商品销售"
 *  @return ： status 
 */
int cmd_demo_issue_inv(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_return_inv - service for submenu "培训-退票操作"
 *  @return : status 
 */
int cmd_demo_return_inv(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_spoil_inv - service for submenu "培训-废票操作"
 *  @return : status 
 */
int cmd_demo_spoil_inv(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_view_cur_roll - service for submenu "培训-查看当前卷"
 *  @return : status 
 */
int cmd_demo_view_cur_roll(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_view_inv - service for submenu "培训-查看发票"
 *  @return : status 
 */
int cmd_demo_view_inv(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_demo_view_inv_roll - service for submenu "培训-查看发票卷"
 *  @return : status
 */
int cmd_demo_view_inv_roll(void)
{
    /*
     * obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

