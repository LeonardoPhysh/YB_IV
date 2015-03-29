/*
 * cmd_invoice_manage.c - the set of functions of the menu "发票管理"
 *  - implement all of the functions to do management of invoice 
 *
 * Author : Leonardo.Phsy 
 * Date   : 2014.9.28 Rev01
 * ---------------------
 * Update Log:
 *    Add cmd_view_disted_roll() - 2014.12.8
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "config.h"
#include "error.h"
#include "ui_api.h"
#include "input.h"
#include "tax_system.h"
#include "tax_file_op.h"
    
/*
 * cmd_dist_invoice - service the sub-menu named "分发发票"
 *  @return : status 
 */
int cmd_dist_inv(void)
{
    int ret; 
    struct simple_frame frame;
    struct tax_sys_protect_record prot_rec; 
    struct tax_sys_invoice_roll_record inv_roll_rec;
    struct tax_system * tax_system = get_tax_system();
    
    memset(&frame, 0, sizeof(frame));
    
    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未启用，请先进行税控初始化!");
         
        return FAIL;
    }

    ret = tax_file_is_empty(PROTECT_FILE);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    } else if (ret != POSITIVE) {
        display_warn("还有已分发的发票未使用，无法分发发票!");
        return SUCCESS;
    }
 
    ret = question_user("确认用户卡插入并分发发票？");
    if (ret != POSITIVE)
        return FAIL;
    
    display_info("正在分发发票...");
     
    memset(&inv_roll_rec, 0, sizeof(inv_roll_rec));
    memset(&prot_rec, 0, sizeof(sizeof(prot_rec)));

    ret = tax_system->dist_invoice(&inv_roll_rec);
    if (ret == SUCCESS) {
        prot_rec.type = PROTECT_TYPE_MOUNT;
        prot_rec.invoice_info = inv_roll_rec;

        ret = tax_file_save_protect(&prot_rec);
        if (ret < 0) {
            display_warn("分发失败!");
            return FAIL;
        }

        frame.item_num = 2;
        frame.items[0].pos.row = 1;
        frame.items[0].pos.col = 4;
        sprintf(frame.items[0].title, "%s", "分发成功");

        frame.items[1].pos.row = 3;
        frame.items[1].pos.col = 1;

        sprintf(frame.items[1].title, "%010d ~ %010d",
                inv_roll_rec.start_num, inv_roll_rec.end_num);

        show_simple_frame(&frame);

        clear_cache();
        get_keycode();

        return SUCCESS;
    }

    display_err_msg(ret);
    return FAIL;
}

/*
 * cmd_mount_roll - service for sub-menu named "装载发票"
 *  @return : status 
 */
int cmd_mount_roll(void)
{
    int ret;
    int key_code;
    struct simple_frame frame;
    struct tax_sys_protect_record prot_rec; 
    struct tax_system * tax_system = get_tax_system(); 
    struct tax_sys_cur_roll_left_record *gp_crln = get_cur_roll_left();

    memset(&frame, 0, sizeof(frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未启用，请先进行税控初始化!");

        return FAIL;
    }

    if (gp_crln->cur_roll_left > 0) {
        display_warn("当前发票卷未使用完, 无法装载");

        return SUCCESS;
    }

    ret = tax_file_is_empty(PROTECT_FILE);
    if (ret == POSITIVE) {
        display_warn("无已分发发票，请先分发发票！");

        return SUCCESS;
    }

    ret = tax_file_read_protect(&prot_rec);
    if (ret < 0) {
        display_warn("装载失败!");

        return FAIL;
    }

    frame.item_num = 2;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    sprintf(frame.items[0].title, "%s", "发票装载");

    frame.items[1].pos.row = 3;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%010d ~ %010d",
            prot_rec.invoice_info.start_num, prot_rec.invoice_info.end_num);

    show_simple_frame(&frame);

    clear_cache();
    while ((key_code = get_keycode()) != ENTER);

    ret = question_user("确认装载？");
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在装载...");

    ret = tax_system->mount_roll(&prot_rec.invoice_info);
    if (ret == SUCCESS) {
        ret = tax_file_clear(PROTECT_FILE);
        if (ret < 0) {
            display_warn("装载失败！");

            return FAIL;
        }

        display_warn("装载成功！");
        return SUCCESS;

    }

    display_err_msg(ret);
    return FAIL;
}

/*
 * cmd_view_cur_roll - service for sub-menu named "查看当前卷"
 *  @retrun : status 
 */
int cmd_view_cur_roll(void)
{
    int ret;
    int key, pos;
    int total_num, used_num;
    struct simple_frame frame, bot_frame;
    struct tax_sys_buy_roll_record * gp_cur_roll = get_cur_roll();
    struct tax_sys_cur_roll_left_record *gp_crln = get_cur_roll_left();
    struct tax_system * tax_system = get_tax_system();

    memset(&frame, 0, sizeof(frame));
    memset(&bot_frame, 0, sizeof(bot_frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未启用，请先进行税控初始化!");

        return FAIL;
    }

    total_num = gp_cur_roll->end_num - gp_cur_roll->start_num; 
    used_num = total_num - gp_crln->cur_roll_left;

    if (total_num == 0) {
        display_warn("请先进行发票装载!");
        return FAIL;
    }

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "%s", "当前发票信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "发票代号");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 2;
    int i;
    for (i = 0; i < 10; i++)
        sprintf(frame.items[2].title + i*2, "%02x", gp_cur_roll->invoice_code[i]);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%010d", "起始号：", gp_cur_roll->start_num);

    bot_frame.item_num = 4;
    bot_frame.items[0].pos.row = 1;
    bot_frame.items[0].pos.col = 4;
    sprintf(bot_frame.items[0].title, "%s", "当前发票信息");

    bot_frame.items[1].pos.row = 2;
    bot_frame.items[1].pos.col = 1;
    sprintf(bot_frame.items[1].title, "%s%010d", "终止号:", gp_cur_roll->end_num);

    bot_frame.items[2].pos.row = 3;
    bot_frame.items[2].pos.col = 1;
    sprintf(bot_frame.items[2].title, "已使用：%d 张", used_num);

    bot_frame.items[3].pos.row = 4;
    bot_frame.items[3].pos.col = 1;
    sprintf(bot_frame.items[3].title, "剩余：%d 张", gp_crln->cur_roll_left);

    show_simple_frame(&frame);

    pos = 1;
    clear_cache();
    while (1) {
        key = get_keycode();
        switch (key) {
            case ENTER: 
                return SUCCESS;
                break;

            case UP:
                if (pos != 1) {
                    show_simple_frame(&frame);
                    pos = 1;
                }
                break;

            case DOWN:
                if (pos != 2) {
                    show_simple_frame(&bot_frame);
                    pos = 2;
                }

                break;

            default:
                break;
        }
    }
}

/*
 * cmd_view_buyed_invoice_info - the function of sub-menu "查看领购信息"
 *  @return : status 
 */
int cmd_view_buyed_inv_info(void)
{
    int ret;
    int roll_num = 0;
    int offset, roll, key;

    struct simple_frame frame[2];

    struct tax_system * tax_system = get_tax_system(); 
    struct tax_sys_buy_roll_record buy_roll_rec[10];

    memset(buy_roll_rec, 0, 10 * sizeof(struct tax_sys_buy_roll_record));
    memset(frame, 0, 2 * sizeof(struct simple_frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未启用，请先进行税控初始化!");
        return FAIL;
    }

    ret = tax_system->get_buy_roll(&roll_num, buy_roll_rec);
    if (ret != SUCCESS) {
        if (ret == -ETAX_INVOICE_EMPTY) {
            display_warn("用户卡不存在领购信息！");
            return FAIL;
        } else {
            display_warn("获取信息失败!");
            return FAIL;
        }
    }

    offset = 0;
    roll = 1;

    frame[0].item_num = 4;
    frame[0].items[0].pos.row = 1;
    frame[0].items[0].pos.col = 4;

    frame[0].items[1].pos.row = 2;
    frame[0].items[1].pos.col = 1;

    frame[0].items[2].pos.row = 3;
    frame[0].items[2].pos.col = 1;

    frame[0].items[3].pos.row = 4;
    frame[0].items[3].pos.col = 1;

    frame[1].item_num = 4;
    frame[1].items[0].pos.row = 1;
    frame[1].items[0].pos.col = 4;

    frame[1].items[1].pos.row = 2;
    frame[1].items[1].pos.col = 1;

    frame[1].items[2].pos.row = 3;
    frame[1].items[2].pos.col = 1;

    frame[1].items[3].pos.row = 4;
    frame[1].items[3].pos.col = 1;

    sprintf(frame[0].items[0].title, "%s", "发票领购信息");
    sprintf(frame[1].items[0].title, "%s", "发票领购信息");

    sprintf(frame[0].items[2].title, "%s", "发票代号：");

    while (1) {
        memset(frame[0].items[1].title, 0, MAX_TITLE_LEN);
        memset(frame[0].items[3].title, 0, MAX_TITLE_LEN);
        memset(frame[1].items[1].title, 0, MAX_TITLE_LEN);
        memset(frame[1].items[2].title, 0, MAX_TITLE_LEN);

        sprintf(frame[0].items[1].title, "第%d 卷，共%d 卷", roll, roll_num);
        sprintf(frame[1].items[1].title, "第%d 卷，共%d 卷", roll, roll_num);

        int i;
        for (i = 0; i < 10; i++) 
            sprintf(frame[0].items[3].title + i * 2, "%02x", buy_roll_rec[roll - 1].invoice_code[i]);

        sprintf(frame[1].items[2].title, "%s%010d", "起始号:", buy_roll_rec[roll - 1].start_num);
        sprintf(frame[1].items[3].title, "%s%010d", "终止号:", buy_roll_rec[roll - 1].end_num);

        show_simple_frame(&frame[offset]);

        clear_cache();
        key = get_keycode();
        switch (key) {
            case ENTER:
                return SUCCESS;
                break;

            case UP:
                if (offset != 0) {
                    offset = 0;
                    continue;
                }
                break;

            case DOWN:
                if (offset != 1) {
                    offset = 1;
                    continue;
                }
                break;

            case LEFT:
                if (roll > 1) {
                    roll --;
                    continue;
                }
                break;

            case RIGHT:
                if (roll < roll_num) {
                    roll ++;
                    continue;
                }
                break;

            default:
                break;
        } 
    }

    return SUCCESS;
}

/*
 * cmd_view_disted_roll - service for sub-menu "查看分发信息"
 *  @return : status
 */
int cmd_view_disted_roll(void)
{
    int ret;
    int key, pos;

    struct tax_system * tax_system =get_tax_system();
    struct tax_sys_invoice_roll_record roll_rec;
    struct simple_frame frame, bot_frame;
    memset(&frame, 0, sizeof(frame));
    memset(&bot_frame, 0, sizeof(bot_frame));
    
    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未启用，请先进行税控初始化!");

        return FAIL;
    }

    ret = tax_file_read_last_dist(&roll_rec);
    if (ret < 0) {
        if (ret == -EFILE_NO_REC) {
            display_warn("无分发记录！");
            return FAIL;
        }

        display_err_msg(ret);
        return FAIL;
    }

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "%s", "已分发卷信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "发票代号");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 2;
    int i;
    for (i = 0; i < 10; i++)
        sprintf(frame.items[2].title + i*2, "%02x", roll_rec.invoice_code[i]);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%010d", "起始号：", roll_rec.start_num);

    bot_frame.item_num = 2;
    bot_frame.items[0].pos.row = 1;
    bot_frame.items[0].pos.col = 4;
    sprintf(bot_frame.items[0].title, "%s", "已分发卷信息");

    bot_frame.items[1].pos.row = 2;
    bot_frame.items[1].pos.col = 1;
    sprintf(bot_frame.items[1].title, "%s%010d", "终止号:", roll_rec.end_num);

    show_simple_frame(&frame);

    pos = 1;
    clear_cache();
    while (1) {
        key = get_keycode();
        switch (key) {
            case ENTER: 
                return SUCCESS;
                break;

            case UP:
                if (pos != 1) {
                    show_simple_frame(&frame);
                    pos = 1;
                }
                break;

            case DOWN:
                if (pos != 2) {
                    show_simple_frame(&bot_frame);
                    pos = 2;
                }

                break;

            default:
                break;
        }
    }

    return SUCCESS;
}

/* end of cmd_invoice_manage */

