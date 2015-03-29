/*
 * cmd_invoice_ops.c
 *  - implement function of transact
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
#include "real_time.h"

/*
 * global variables define
 */
static int g_hang_flag = NEGATIVE;

static struct tax_sys_transact_items g_normal_trans_items;
static struct tax_sys_transact_items g_hang_trans_items;
static struct tax_sys_invoice_detail_record g_inv_detail;

/*
 * UI Helper functions 
 */
static int show_trans_input_plu(int type, int item_num)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 2;
    sprintf(frame.items[0].title, "销售商品（第%d件）", item_num + 1);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", type == BY_PLU_NUM ? "编号：" : "条形码：");

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "小计：0.00");

    show_simple_frame(&frame);

    return SUCCESS;
}

static int show_trans_ensure_plu(int type, int item_num, struct plu_item *plu_item)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 2;
    sprintf(frame.items[0].title, "销售商品（第%d件）", item_num + 1);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    if (type == BY_PLU_NUM) 
        sprintf(frame.items[1].title, "%s%d", "编号：", plu_item->plu_num);
    else  
        sprintf(frame.items[1].title, "%s%s", "条形码:", plu_item->barcode);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s%s", "名称：", plu_item->name);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%.2f", "单价：", (float)plu_item->price);

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 8;
    sprintf(frame.items[4].title, "%s%d", "库存：", plu_item->stock);

    show_simple_frame(&frame);

    return SUCCESS;
}

static int show_trans_input_count(int item_num, struct plu_item* plu_item)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 2;
    sprintf(frame.items[0].title, "销售商品（第%d件）", item_num + 1);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "数量：");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s%s", "名称：", plu_item->name);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%.2f", "单价：", (float)plu_item->price);

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 8;
    sprintf(frame.items[4].title, "%s%d", "库存：", plu_item->stock);

    show_simple_frame(&frame);

    return SUCCESS;
}

static int show_trans_ensure_count(struct plu_item * plu_item, int item_num, int count)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 2;
    sprintf(frame.items[0].title, "销售商品（第%d件）", item_num + 1);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%d", "数量：", count);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s%s", "名称：", plu_item->name);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%.2f", "单价：", (float)plu_item->price);

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 9;
    sprintf(frame.items[4].title, "%.2f", (float)count * plu_item->price);

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_subtoal_menu(int invoice_nb) 
{
    int i;
    int count = 0, total = 0;
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    count = g_normal_trans_items.item_num;
    for (i = 0; i < count; i++) {
        total += g_normal_trans_items.comm_items[i].amount;
    }

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    sprintf(frame.items[0].title, "%s%010d", "发票号:", invoice_nb);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%d", "总计：", total);

    frame.items[2].pos.row = 2;
    frame.items[2].pos.col = 9;
    sprintf(frame.items[2].title, "共%d件", count);

    frame.items[3].pos.row = 3;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s", "实收：");

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 1;
    sprintf(frame.items[4].title, "%s", "找零：");

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_trans_input_payer(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 1;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    sprintf(frame.items[0].title, "%s", "请输入付款单位：");

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_trans_input_drawer(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 1;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    sprintf(frame.items[0].title, "%s", "请输入开票人：");

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_return_input_orige(uint inv_num)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    sprintf(frame.items[0].title, "%s", "退票处理");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%010d", "发票序号: ", inv_num);
    
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "原发票号: ");

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_return_orige_info(struct tax_sys_invoice_detail_record * ori_detail)
{
    uint y, m, d;
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));
    
    bcd_to_greg(&ori_detail->detail_date, &y, &m, &d);

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    sprintf(frame.items[0].title, "%s", "原票信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%010d", "发票号: ", ori_detail->detail_inv_num);
    
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s%d-%02d-%02d", "开票日期: ", y, m, d);
    
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s%.2f", "总金额(元):", (float)ori_detail->detail_amt_total);

    show_simple_frame(&frame);
    
    return SUCCESS;
}

static int show_spoil_invoice_num(void) 
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 1;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    sprintf(frame.items[0].title, "%s", "请输入废票张数: ");
    
    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_man_issue_inv(void)
{ 
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;

    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "手工开票");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s", "名称：");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s", "金额：：");

    show_simple_frame(&frame);
    return SUCCESS;
}

/*
 * inline helper functions  
 */
inline static int set_hang_flag(int flag)
{
    g_hang_flag = flag;
    return SUCCESS;
}

inline static int get_hang_flag(void)
{
    return g_hang_flag;
}

/* --ATTENTION-- 
 *  get_plu_num & get_plu_count & get_paid_up
 *  these input method have special inline operate, do put 
 *  them into input.c 
 */
static int get_plu_num(int row, int col, int *plu_num)
{
    int ret;
    int key, offset = 0;

    /* plu_num : 01000 - 20999 */
    char ascii_no[6] = {0}; 

    if (*plu_num != 0) {
        offset = snprintf(ascii_no, 6, "%d", *plu_num);
    } 

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        show_str(row, col, ascii_no);

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 5) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) {
                case D_ZERO :
                    if (offset != 0 && offset < 4) {
                        sprintf(ascii_no + offset, "%c%c", '0', '0');
                        offset += 2;
                    } else {
                        if (offset == 4) {
                            sprintf(ascii_no + offset, "%c", '0');
                            offset ++;
                        }
                    }
                    break;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case VIEW:
                    if (offset == 0) {
                        ret = -EUI_VIEW;
                        goto handled;
                    }
                    break;

                case HANG:
                    if (offset == 0) {
                        ret = -EUI_HANG;
                        goto handled;
                    }
                    break; 

                case MODIFY:
                    if (offset == 0) {
                        ret = -EUI_MODIFY;
                        goto handled;
                    }
                    break;

                case TOTAL:
                    if (offset == 0) {
                        ret = -EUI_TOTAL;
                        goto handled;
                    }
                    break;

                case ESC:
                    ret = -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    ret = SUCCESS;
                    goto handled;
                    break;

                default:
                    break;
            }
        }
    }

handled:
    *plu_num = atoi(ascii_no);

    return ret;
}


static int get_plu_count(int *count, struct plu_item * tmp_item)
{
    int ret;
    int key, offset = 0;
    char ascii_no[4] = {0}; 

    if (*count != 0) {
        offset = snprintf(ascii_no, 4, "%d", *count);
    } 

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        if (offset == 0)
            show_trans_input_count(g_normal_trans_items.item_num, tmp_item);

        if (*count > 0)
            show_trans_ensure_count(tmp_item, g_normal_trans_items.item_num, *count);

        show_str(2, 4, ascii_no);

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 3) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
                *count = atoi(ascii_no);
            }
        } else {
            switch (key) {
                case D_ZERO :
                    if (offset != 0 && offset < 2) {
                        sprintf(ascii_no + offset, "%c%c", '0', '0');
                        offset += 2;
                        *count = atoi(ascii_no);
                    } else {
                        if (offset == 2) {
                            sprintf(ascii_no + offset, "%c", '0');
                            offset ++;
                            *count = atoi(ascii_no);
                        }
                    }

                    break;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                        *count = atoi(ascii_no);
                    }
                    break;

                case ESC:
                    ret = -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    ret = SUCCESS;
                    goto handled;
                    break;

                default:
                    break;
            }
        }
    }

handled:
    *count = atoi(ascii_no);

    return ret;
}

static int get_paid_up(int *paid_up)
{
    int ret;
    int key, offset = 0;
    int i, count, total = 0;
    int charg = 0; 
    char charg_str[7] = {0};
    char ascii_no[7] = {0}; 

    if (*paid_up != 0) {
        offset = snprintf(ascii_no, 6, "%d", *paid_up);
    } 

    count = g_normal_trans_items.item_num;
    for (i = 0; i < count; i++) {
        total += g_normal_trans_items.comm_items[i].amount;
    }

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        show_str(3, 4, ascii_no);

        charg = *paid_up - total;
        if (charg > 0) {
            memset(charg_str, 0, 7);
            sprintf(charg_str, "%.2f", (float)charg);

            show_str(4, 4, charg_str); 
        } else 
            show_str(4, 4, NULL); 

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 5) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;

                *paid_up = atoi(ascii_no);
            }
        } else {
            switch (key) {
                case D_ZERO:
                    if (offset != 0 && offset < 5) {
                        sprintf(ascii_no + offset, "%c%c", '0', '0');
                        offset += 2;

                        *paid_up = atoi(ascii_no);
                    } else {
                        if (offset == 5) {
                            sprintf(ascii_no + offset, "%c", '0');
                            offset ++;

                            *paid_up = atoi(ascii_no);
                        }
                    }
                    break;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;

                        *paid_up = atoi(ascii_no);
                    }
                    break;

                case ESC:
                    ret = -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    ret = SUCCESS;
                    goto handled;
                    break;

                default:
                    break;
            }
        }
    }

handled:
    *paid_up = atoi(ascii_no);

    return ret;
}

static int get_fiscal_code(struct issue_invoice_res * inv_res) 
{
    uint top_half = 0;
    uint bot_half = 0;

#if 0
    char top_buf[10 + 1] = {0};
    char bot_buf[10 + 1] = {0};

    int i;
    for (i = 0; i < 4; i++) {
        tmp = inv_res->half_top[i];
        top_half <<= 8;
        top_half += tmp;

        tmp = inv_res->half_bot[i];
        bot_half <<= 8;
        bot_half += tmp;
    }

    sprintf(top_buf, "%10u", top_half);
    sprintf(bot_buf, "%10u", bot_half);

    for (i = 0; i < 10; i++) {
        inv_res->tax_num[i] = top_buf[i];

        if (bot_buf[i] == ' ')
            inv_res->tax_num[10 + i] = '0';
        else 
            inv_res->tax_num[10 + i] = bot_buf[i];
    }
#endif

    end_cover_int((uchar *)&top_half, (uchar *)inv_res->half_top);
    end_cover_int((uchar *)&bot_half, (uchar *)inv_res->half_bot);

    sprintf(inv_res->tax_num, "%010u%010u", top_half, bot_half);

    return SUCCESS;
}

/*
 * do_get_comm_item - show comm_items on screen and 
 *                    get a user choosed item then return it.
 *   @return : the index of choosed comm_item
 */
static int do_get_comm_item(char * title)
{
    int key_code;
    int i, count = 0, pos = 0; 

    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    count = g_normal_trans_items.item_num; 
    if (count == 0) {
        display_warn("尚未添加商品！");
        return SUCCESS;
    }

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 6 - strlen(title)/4;
    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;

    strcpy(frame.items[0].title, title);

    i = 0;  
    pos = 1;  
    clear_cache();
    while (1) {
        if (count - i >= 3) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 1, 
                    g_normal_trans_items.comm_items[i].comm_plu_name);
            snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 2, 
                    g_normal_trans_items.comm_items[i + 1].comm_plu_name);
            snprintf(frame.items[3].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 3, 
                    g_normal_trans_items.comm_items[i + 2].comm_plu_name);
        } else if (count - i == 2) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 1, 
                    g_normal_trans_items.comm_items[i].comm_plu_name);
            snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 2, 
                    g_normal_trans_items.comm_items[i + 1].comm_plu_name);
            memset(frame.items[3].title, 0, MAX_TITLE_LEN);
        } else if (count - i == 1) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "商品", i + 1, 
                    g_normal_trans_items.comm_items[i].comm_plu_name);
            memset(frame.items[2].title, 0, MAX_TITLE_LEN);
            memset(frame.items[3].title, 0, MAX_TITLE_LEN);
        }

        show_simple_frame(&frame);
        highlight_on(1 + pos);

        key_code = get_keycode();
        switch (key_code) {
            case UP:
                if (pos > 1) {
                    highlight_off(1 + pos--);
                    highlight_on(1 + pos);
                } else {
                    if (i > 0) {
                        i -= 3;
                        highlight_off(1 + pos);
                        pos = 3;
                        highlight_on(1 + pos);
                    }
                }
                break;

            case DOWN:
                if (count > i + pos) {
                    if (pos < 3) {
                        highlight_off(1 + pos++);
                        highlight_on(1 + pos);
                    } else {
                        if (count - i > 3) {
                            i += 3;
                            highlight_off(1 + pos);
                            pos = 1;
                            highlight_on(1 + pos);
                        }
                    }
                }
                break; 

            case ENTER:
                clear_screen();
                return (i + pos - 1); 
                break;

            case BACK:
                clear_screen();
                return -EUI_BACK;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            default:
                break;  
        }
    }
}

/*
 * do_view_comm_items - do real job of view a list of comm_items
 *  @return : SUCCESS
 */
static int do_view_comm_items(void)
{
    int ret;
    int item, type, key;
    struct plu_item * plu_item;
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    sprintf(frame.items[0].title, "商品信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 8;

    type = g_normal_trans_items.type; 
    while (1) {
        ret= do_get_comm_item("商品查询");
        if (ret == -EUI_BACK)
            return SUCCESS;
        else if (ret == -EUI_ESC)
            return ret;
        item = ret;

        plu_item = &g_normal_trans_items.comm_items[item].plu_item;
        if (type == BY_PLU_NUM) 
            sprintf(frame.items[1].title, "%s%d", "编号：", plu_item->plu_num);
        else  
            sprintf(frame.items[1].title, "%s%s", "条形码:", plu_item->barcode);

        sprintf(frame.items[2].title, "%s%s", "名称：", plu_item->name);
        sprintf(frame.items[3].title, "%s%.2f", "单价：", (float)plu_item->price);
        sprintf(frame.items[4].title, "%s%d", "数量：", g_normal_trans_items.comm_items[item].num);

        show_simple_frame(&frame);

        clear_cache();
        do {
            key = get_keycode();
            if (key == ESC) 
                return ret;
            
            if (key == BACK)
                break;
        } while (key != ENTER);
    }
}

/*
 * do_modify_comm_items - do real job of modify a com_item
 *  @return : status 
 */
static int do_modify_comm_items(void)
{
    int ret;
    int key, item, type;
    int new_plu_num, new_count;
    struct plu_item new_plu;
    struct plu_item * plu_item;
    struct simple_frame frame;
    struct plu_operate * plu_ops = get_plu_ops();

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 2;
    sprintf(frame.items[0].title, "商品信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 7;

    type = g_normal_trans_items.type; 
    while (1) {
        ret = do_get_comm_item("商品修改");
        if (ret == -EUI_BACK)
            return SUCCESS;
        else if (ret == -EUI_ESC)
            return ret;
        item = ret;

        plu_item = &g_normal_trans_items.comm_items[item].plu_item;
        if (type == BY_PLU_NUM) 
            sprintf(frame.items[1].title, "%s%d", "编号：", plu_item->plu_num);
        else  
            sprintf(frame.items[1].title, "%s%s", "条形码:", plu_item->barcode);

        sprintf(frame.items[2].title, "%s%s", "名称：", plu_item->name);
        sprintf(frame.items[3].title, "%s%.2f", "单价：", (float)plu_item->price);
        sprintf(frame.items[4].title, "%s%d", "数量：", g_normal_trans_items.comm_items[item].num);

        show_simple_frame(&frame);
        
        if (type == BY_PLU_NUM) {
            new_plu_num = plu_item->plu_num;
            while (1) {
                ret = get_plu_num(2, 4, &new_plu_num);
                if (ret == -EUI_ESC) 
                    return ret;

                if (new_plu_num != plu_item->plu_num) {
                    ret = plu_ops->read_plu(new_plu_num, &new_plu);
                    if (ret != SUCCESS) {
                        /* clear name, price, count in sreen */
                        show_str(3, 4, "        ");
                        display_str(4, 4, "        ");
                        display_str(4, 10, "       ");
                        continue;
                    } else {
                        new_count = 0;
                        break;
                    }
                } else {
                    new_count = g_normal_trans_items.comm_items[item].num;
                    new_plu = *plu_item;
                    break;
                }
            }
        } 
#ifdef CONFIG_HPP
        else {
            memcpy(new_barcode, plu_item->barcode, 13);
            while (1) { 
                char new_barcode[14] = {0};
                ret = get_barcode(2, 5, new_barcode);
                if (ret == -EUI_ESC) 
                    return ret;

                if (memcpm(new_barcode, plu_item->barcode) != 0) {
                    ret = plu_ops->index_by_bc(new_barcode, &new_plu);
                    if (ret != SUCCESS) {
                        /* clear name, price, count in screen */
                        show_str(3, 4, "        ");
                        display_str(4, 4, "        ");
                        display_str(4, 10, "       ");
                        continue;
                    }
                } else {
                    new_count = g_normal_trans_items.comm_items[item].num;
                    new_plu = *plu_item;
                    break;
                }
            }
        }
#endif
        /* reshow plu_item info */
        show_str(3, 4, new_plu.name);
        display_str(4, 4, "%d", new_plu.price);
        display_str(4, 10, "%d", new_count);

        ret = get_inter_num(4, 10, &new_count);
        if (ret == -EUI_ESC)
            return ret;

        clear_cache();
        do {
            key = get_keycode();
            if (key == ESC) {
                ret = question_user("是否要取消本次销售？");
                if (ret == POSITIVE)
                    return FAIL;
                else 
                    continue; 
            }
        } while (key != ENTER);

        /* remove the plu_item if count is 0 */
        if (new_count == 0) {
            int i;
            for (i = item; i < g_normal_trans_items.item_num - 1; i ++)
                g_normal_trans_items.comm_items[i] = g_normal_trans_items.comm_items[i + 1];

            g_normal_trans_items.item_num --;
        } else {
            g_normal_trans_items.comm_items[item].num = new_count;
            g_normal_trans_items.comm_items[item].amount = new_count * new_plu.price;
            g_normal_trans_items.comm_items[item].plu_item = new_plu;
        }
    }

    display_warn("修改成功！");
    return SUCCESS;
}

/*
 * do_hang_up_transact : hang up a transact
 *  @return : status
 */
static int do_hang_up_transact(void)
{
    if (g_hang_flag == POSITIVE) {
        display_warn("后台已存在挂单，请先恢复挂单");
        return FAIL;
    }

    g_hang_trans_items = g_normal_trans_items;
    set_hang_flag(POSITIVE);

    display_warn("挂单至后台成功！");
    return SUCCESS;
}


/*
 * do_add_comm_to_list : add a comm item to transact list 
 *  @return : status 
 */
static int do_add_comm_to_list(struct plu_item * tmp_item, int count)
{
    int pos = g_normal_trans_items.item_num;

    g_normal_trans_items.comm_items[pos].num = count;
    g_normal_trans_items.comm_items[pos].amount = count * tmp_item->price;
    g_normal_trans_items.comm_items[pos].plu_item = *tmp_item;

    g_normal_trans_items.item_num ++;

    return SUCCESS;
}

/*
 * do_subtotal_comm_items - do subtotal 
 *  @return : status 
 */
static int do_subtotal_comm_items(void)
{
    int ret, paid_up;
    uint invoice_nb;
    struct tax_system *tax_system = get_tax_system();

    ret = tax_system->get_invoice_nb(&invoice_nb);
    if (ret != SUCCESS) {
        if (ret == -ETAX_INVOICE_MC_EMPTY)
            display_warn("未发现可用发票，请先装载发票！");

        return FAIL;
    }

    while (1) {
        show_subtoal_menu(invoice_nb);

        ret = get_paid_up(&paid_up);
        if (ret == -EUI_BACK) {
            ret = question_user("是否要取消本次销售？");
            if (ret == POSITIVE)
                return FAIL;

            continue;
        } else if (ret == SUCCESS)
            break;
    }

    return SUCCESS;
}

/*
 * do_check_origin_inv - check the origin invoice can be returned or not 
 *  @inv_detail: invoice detail
 */
static int do_check_origin_inv(struct tax_sys_invoice_detail_record * inv_detail)
{
    int ret;
    struct bcd_date declare_date;
    struct tax_system * tax_system = get_tax_system();
    struct rt_operate * rt_ops = get_rt_ops();

    ret = tax_system->get_last_declare_date(&declare_date);
    if (ret < 0)
        return ret;

    if (rt_ops->cmp_bcd_date(&inv_detail->detail_date, &declare_date) < 0)
        return -ETAX_OUT_DECLARE_DATE;

    if (inv_detail->detail_type == RETURN_INVOICE)
        return -ETAX_RETURN_INV;

    if (inv_detail->detail_type == SPOIL_INVOICE)
        return -ETAX_SPOIL_INV;

    return SUCCESS;
}

/*
 * do_return_invoice - return a spoil_invoice
 *  @return : orige invoice detail information
 *  @return : status 
 */
static int do_return_invoice(struct tax_sys_invoice_detail_record * ori_detail)
{
    int i, j, ret;
    int count, tax_index, inv_nb;
    struct bcd_date today;
    struct tax_sys_issue_invoice issue_inv;
    struct issue_invoice_res issue_res;
    struct tax_sys_invoice_detail_record inv_detail;
    struct tax_system * tax_system = get_tax_system();
    struct rt_operate * rt_ops = get_rt_ops();

    memset(&inv_detail, 0, sizeof(inv_detail));
    memset(&issue_inv, 0, sizeof(issue_inv));

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0)
        return ret;

    ret = tax_system->get_invoice_nb((uint *)&inv_nb);
    if (ret < 0)
        return ret;

    count = ori_detail->item_num;
    for (i = 0; i < count; i++) {
        tax_index = ori_detail->item[i].comm_tax_index;

        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (issue_inv.item[j].index == tax_index) {
                issue_inv.item[j].amount += ori_detail->item[i].amount;
            }
        }
    }

    issue_inv.invoice_type = RETURN_INVOICE;
    issue_inv.amt_total = ori_detail->detail_amt_total;
    issue_inv.date = today;
    issue_inv.invoice_num = (uint)inv_nb;
    issue_inv.origin_inv_num = ori_detail->detail_inv_num;

    ret = tax_system->issue_invoice(&issue_inv, &issue_res);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    }

    get_fiscal_code(&issue_res);

    inv_detail = *ori_detail;
    inv_detail.detail_type = RETURN_INVOICE;
    inv_detail.detail_amt_total = issue_inv.amt_total;
    inv_detail.detail_ori_inv_num = issue_inv.origin_inv_num;
    inv_detail.detail_date = issue_inv.date;
    memcpy(inv_detail.detail_fiscal_code, issue_res.tax_num, 20);

    ret = tax_file_append_invoice_detail(&inv_detail);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    }

    return SUCCESS;
}

/*
 * do_issue_invoice - issue invoice 
 *  @return : status 
 */
static int do_issue_invoice(void)
{
    int i, j, ret;
    int count, tax_index;
    struct tax_sys_issue_invoice issue_inv;
    struct issue_invoice_res inv_res;
    struct tax_system *tax_system = get_tax_system();
    struct rt_operate *rt_ops =  get_rt_ops();
    struct tax_sys_app_info  *sys_app_info = get_sys_app_info();

    memset(&issue_inv, 0, sizeof(issue_inv));
    memset(&inv_res, 0, sizeof(inv_res));

    display_info("正在进行开票检查...");

    ret = tax_system->transact_prepare();
    switch (ret) {
        case -ETAX_FISCAL_NOT_INIT:
            display_warn("商业功能未启用，无法进行交易！");
            return FAIL;

            break;

        case -ETAX_MACH_LOCKED:
            display_warn("机器过期锁死，无法进行交易！");
            return FAIL;

            break;

        case -ETAX_ISSUE_LIMIT:
            display_warn("已过开票截止日期，无法进行交易！");
            return FAIL;

            break;

        case -ETAX_FC_NOT_READY:
            display_warn("税控卡未就绪，请联系厂商！");
            return FAIL;

            break;

        case -ETAX_INVOICE_FULL:
            display_warn("本申报期所开发票已达最大限制，请先进行申报.");
            return FAIL;

            break;

        case -ETAX_AMT_TOTAL_LIMIT:
            display_warn("本申报期开票金额已达最大限制，无法继续开票.");
            return FAIL;

            break;

        default:
            if (ret != SUCCESS) {
                display_err_msg(ret);
                return FAIL;
            }

            break;
    }

    ret = rt_ops->get_cur_date(&issue_inv.date);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    /* save tax_index as sys_app_info */ 
    for (i = 0; i < MAX_USER_TAXRATE; i++) {
        issue_inv.item[i].index = sys_app_info->tax_index[i];
    }

    count = g_normal_trans_items.item_num;

    for (i = 0; i < count; i++) {
        issue_inv.amt_total += g_normal_trans_items.comm_items[i].amount;

        tax_index = g_normal_trans_items.comm_items[i].comm_tax_index;

        for (j = 0; j < MAX_USER_TAXRATE; j++) {
            if (issue_inv.item[j].index == tax_index) {
                issue_inv.item[j].amount += g_normal_trans_items.comm_items[i].amount;
            }
        }
    }

    issue_inv.invoice_type = NORMAL_INVOICE;

    display_info("正在开票...");

    ret = tax_system->issue_invoice(&issue_inv, &inv_res);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    get_fiscal_code(&inv_res);

    /* save fiscal code */
    memcpy(g_inv_detail.invoice.fiscal_code, inv_res.tax_num, 20);

    g_inv_detail.invoice.date = issue_inv.date;
    g_inv_detail.invoice.type = issue_inv.invoice_type;
    g_inv_detail.invoice.amt_total = issue_inv.amt_total;
    g_inv_detail.detail_inv_num = issue_inv.invoice_num;

    return SUCCESS; 
}

/*
 * do_print_invoice - generate print frame buffer and print it 
 *  @return : status 
 */
static int do_print_invoice(void)
{
    int ret;
    struct tax_sys_app_info  *sys_app_info = get_sys_app_info();
    struct print_sys * print_sys = get_print_sys();    

    memcpy(g_inv_detail.tax_payee, sys_app_info->taxpayer_name, 41);
    g_inv_detail.item_num = g_normal_trans_items.item_num;

    int i;
    for (i = 0; i < 10; i++) {
        g_inv_detail.item[i] = g_normal_trans_items.comm_items[i];
    }

    memcpy(g_inv_detail.register_num, sys_app_info->taxpayer_nb, 8);

    /* save invoice detail */
    ret = tax_file_append_invoice_detail(&g_inv_detail);
    if (ret < 0) {
        display_err_msg(ret);
        return ret;
    }

    display_info("正在打印发票...");
    ret = print_sys->ops->print_invoice(&g_inv_detail);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    ret = question_user("发票是否打印正确？");
    if (ret == NEGATIVE) {
        display_info("请做退票或废票处理！");
        return FAIL;
    }

    return SUCCESS;
}

/*
 * do_decrease_plu_count - while transace done with success, 
 *                         we need to decrease plu count.
 * @return : status 
 */
static int do_decrease_plu_count(void)
{
    int ret;

    struct plu_item tmp_item;
    struct plu_operate *plu_ops = get_plu_ops();

    int i; 
    for (i = 0; i < g_normal_trans_items.item_num; i++) {
        tmp_item = g_normal_trans_items.comm_items[i].plu_item;
        tmp_item.stock -= g_normal_trans_items.comm_items[i].num;

        ret = plu_ops->modify_plu(tmp_item.plu_num, &tmp_item);
        if (ret < 0)
            return ret;
    }

    return SUCCESS;
}


/*
 * do_transact - service for normal transact and resume transact 
 *  @return : status 
 */
static int do_transact(void)
{
    int ret, key;
    int plu_count = 0, count = 0;
    int plu_num = 0;
    uchar barcode[13] = {0};
    struct plu_item tmp_item;
    struct plu_operate *plu_ops = get_plu_ops();
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，请先进行初始化！");

        return FAIL;
    }

    memset(&g_inv_detail, 0, sizeof(g_inv_detail));

    ret = plu_ops->get_plu_count(&plu_count);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    if (plu_count == 0) {
        display_warn("尚未添加商品，请先添加商品！");
        return FAIL;
    }

    while (1) { 
        plu_num = 0;
        memset(barcode, 0, 13);

input_plu:
        show_trans_input_plu(g_normal_trans_items.type, g_normal_trans_items.item_num);
        while (1) {
            if (g_normal_trans_items.type == BY_PLU_NUM) 
                ret = get_plu_num(2, 4, &plu_num);
            else 
                ret = get_barcode(2, 5, (char *)barcode);

            switch (ret) {
                case -EUI_ESC:
                    ret = question_user("是否要取消本次销售？");
                    if (ret == POSITIVE)
                        return SUCCESS;
                    else 
                        goto input_plu;

                    break;

                case -EUI_TOTAL:
                    if (g_normal_trans_items.item_num > 0)
                        goto subtotal;

                    break;

                case -EUI_VIEW:
                    ret = do_view_comm_items();
                    if (ret == -EUI_ESC) {
                        ret = question_user("是否要取消本次销售？");
                        if (ret == POSITIVE)
                            return SUCCESS;
                        else 
                            goto input_plu;
                    } else if (ret == SUCCESS)
                        goto input_plu;

                    break;

                case -EUI_MODIFY:
                    ret = do_modify_comm_items();
                    if (ret == -EUI_ESC) {
                        ret = question_user("是否要取消本次销售？");
                        if (ret == POSITIVE)
                            return SUCCESS;
                        else 
                            goto input_plu;
                    } else if (ret == SUCCESS)
                        goto input_plu;

                    break;

                case -EUI_HANG:
                    ret = question_user("是否要挂单到后台？");
                    if (ret == POSITIVE) {
                        ret = do_hang_up_transact();
                        if (ret == SUCCESS)
                            return SUCCESS;
                    } else 
                        goto input_plu;

                    break;

                default:
                    break;
            }

            if (g_normal_trans_items.type == BY_PLU_NUM) {
                ret = plu_ops->read_plu(plu_num, &tmp_item);
                if (ret == SUCCESS) 
                    goto ensure_plu;            
                else 
                    continue;
            } 
#ifdef CONFIG_HPP 
            else {
                ret = plu_ops->index_by_bc(barcode, &tmp_item);
                if (ret != SUCCESS)
                    continue;
            }
#endif 
        }

        if (tmp_item.stock == 0) {
            display_warn("商品无库存，请先销售其他商品！");
            continue;
        }

ensure_plu: 
        show_trans_ensure_plu(g_normal_trans_items.type, 
                g_normal_trans_items.item_num, &tmp_item);

        clear_cache();
        do {
            key = get_keycode();
            if (key == ESC) {
                ret = question_user("是否要取消本次销售？");
                if (ret == POSITIVE)
                    return SUCCESS;
                else 
                    goto ensure_plu; 
            }
        } while (key != ENTER);

input_count:
        /*
         * UI update in get_plu_count
         */
        ret = get_plu_count(&count, &tmp_item);
        if (ret == -EUI_ESC) {
            ret = question_user("是否要取消本次销售？");
            if (ret == POSITIVE)
                return SUCCESS;
            else 
                goto input_count; 
        }

        if (count > tmp_item.stock) {
            display_warn("商品库存不足，请销售其他商品！");
            continue;
        }

        do_add_comm_to_list(&tmp_item, count);
    }

subtotal:
    ret = do_subtotal_comm_items();
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

input_payer:
    show_trans_input_payer();
    ret = get_chn_str(2, 1, g_inv_detail.payer_name);
    if (ret == -EUI_ESC) {
        ret = question_user("是否要取消本次销售？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto input_payer;
    }

    if (strlen(g_inv_detail.payer_name) == 0)
        strcpy(g_inv_detail.payer_name, "个人");

input_drawer:
    show_trans_input_drawer();
    ret = get_chn_str(2, 1, g_inv_detail.drawer);
    if (ret == -EUI_ESC) {
        ret = question_user("是否要取消本次销售？");
        if (ret == POSITIVE)
            return SUCCESS;
        else
            goto input_drawer;
    }
    
    if (strlen(g_inv_detail.drawer) == 0)
        strcpy(g_inv_detail.drawer, "无");

    ret = do_issue_invoice();
    if (ret != SUCCESS)
        return FAIL;

    ret = do_print_invoice();
    if (ret != SUCCESS)
        return FAIL;

    /* transact success, dec plu_count */
    ret = do_decrease_plu_count(); 
    if (ret != SUCCESS){
        display_err_msg(ret);
        return FAIL;
    }

    return SUCCESS;
}

/*
 * cmd_transact_by_barcode - the function of sub-menu "按条形码"
 *  @reurn : status 
 */
int cmd_transact_by_barcode(void)
{
#ifdef CONFIG_HPP
    int ret;

    memset(g_normal_trans_items, 0, sizeof(g_normal_trans_items));

    /* save transact type */
    g_normal_trans_items.type = BY_BARCODE; 

    ret = do_transact();
    if (ret != SUCCESS) {
        return FAIL;
    }
#else 
    display_warn("没有装配条码枪，无法进行交易！");
#endif 

    return SUCCESS;
}

/*
 * cmd_transact_by_barcode - the function of sub-menu "按商品编号"
 *  @reurn : status 
 */
int cmd_transact_by_num(void)
{
    int ret;
    memset(&g_normal_trans_items, 0, sizeof(g_normal_trans_items));

    /* save transact type */
    g_normal_trans_items.type =  BY_PLU_NUM; 

    ret = do_transact();
    if (ret != SUCCESS) {
        return FAIL;
    }

    return SUCCESS;
}

/*
 * cmd_resume_transact - resume a transact which has been hang-up 
 *  @return : status 
 */
int cmd_resume_transact(void)
{
    if (g_hang_flag != POSITIVE) {
        display_warn("后台不存在挂单");
        return FAIL;
    }

    g_normal_trans_items = g_hang_trans_items;

    memset(&g_hang_trans_items, 0, sizeof(g_hang_trans_items));
    memset(&g_inv_detail, 0, sizeof(g_inv_detail));

    set_hang_flag(NEGATIVE);
    do_transact();

    return SUCCESS;
}

/*
 * cmd_do_man_issue_inv - service for sub-menu "手工开票"
 * @return : status 
 */
int cmd_man_issue_inv(void)
{
    int ret;
    int rate = 0;
    struct plu_item tmp_item;
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，请先进行初始化！");

        return FAIL;
    }

    memset(&tmp_item, 0, sizeof(tmp_item));
    memset(&g_inv_detail, 0, sizeof(g_inv_detail)); 
    memset(&g_normal_trans_items, 0, sizeof(g_normal_trans_items));

input_name:
    show_man_issue_inv(); 
    ret = get_chn_str(2, 4, tmp_item.name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;

        goto input_name;
    }

    ret = get_inter_num(3, 4, (int *)&tmp_item.price);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;

        goto input_name;
    }

get_rate:
    rate = get_fis_type();
    if (rate == -EUI_ESC) {
        rate = question_user("确定取消本次操作？");
        if (rate == POSITIVE)
            return FAIL;

        goto get_rate;
    } else if (ret == -EUI_BACK) 
        goto input_name;

    tmp_item.tax_index = rate;

    do_add_comm_to_list(&tmp_item, 1);
    ret = do_subtotal_comm_items();
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

input_payer:
    show_trans_input_payer();
    ret = get_chn_str(2, 1, g_inv_detail.payer_name);
    if (ret == -EUI_ESC) {
        ret = question_user("是否要取消本次销售？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto input_payer;
    }

    if (strlen(g_inv_detail.payer_name) == 0)
        strcpy(g_inv_detail.payer_name, "个人");

input_drawer:
    show_trans_input_drawer();
    ret = get_chn_str(2, 1, g_inv_detail.drawer);
    if (ret == -EUI_ESC) {
        ret = question_user("是否要取消本次销售？");
        if (ret == POSITIVE)
            return SUCCESS;
        else
            goto input_drawer;
    }

    if (strlen(g_inv_detail.drawer) == 0)
        strcpy(g_inv_detail.drawer, "无");

    ret = do_issue_invoice();
    if (ret != SUCCESS)
        return FAIL;

    ret = do_print_invoice();
    if (ret != SUCCESS)
        return FAIL;

    return SUCCESS;
}

/*
 * cmd_do_return_invoice - service for submenu "退票操作" 
 *  @return : status 
 */
int cmd_return_inv(void)
{
    int ret, key;
    int inv_num;
    int ori_num;
    struct tax_sys_invoice_detail_record ori_detail;
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->get_invoice_nb((uint *)&inv_num);
    memset(&ori_detail, 0, sizeof(ori_detail));

input_ori:
    show_return_input_orige(inv_num);
    ret = get_inter_num(3, 6, &ori_num);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return SUCCESS;
        else 
            goto input_ori;
    }

    display_info("正在查询开票记录...");

    ret = tax_file_find_invoice_detail((uint *)&ori_num, &ori_detail);
    if (ret == -EFILE_NO_REC){
        display_warn("不存在该票记录！"); 
        goto input_ori;
    } else if (ret == -ETAX_INV_HAS_RETURNED) {
        display_warn("该发票已被退票！"); 
        goto input_ori;
    }

    ret = do_check_origin_inv(&ori_detail);
    switch (ret) {
        case -ETAX_OUT_DECLARE_DATE:
            display_warn("该票不在申报期内，无法退票！");
            goto input_ori;
            break;

        case -ETAX_SPOIL_INV:
            display_warn("该票是已被退票，无法退票！");
            goto input_ori;
            break;

        case -ETAX_RETURN_INV:
            display_warn("该票是废票，无法退票！");
            goto input_ori;
            break;

        default:
            break;
    }

show_ori:
    show_return_orige_info(&ori_detail); 

    set_ime_status(INPUT_LOW_CASE); 
    clear_cache();
    do {
        key = get_keycode();
        switch (key) {
            case ESC:
                ret = question_user("确认取消本次操作？");
                if (ret == POSITIVE)
                    return FAIL;
                break;

            case BACK:
                goto input_ori;
                break;

            case ENTER:
                break;

            default:
                break;
        }
    } while (key != ENTER);

    ret = question_user("确认进行退票？");
    if (ret != POSITIVE)
        goto show_ori;

    display_info("正在退票...");

    ret = do_return_invoice(&ori_detail);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    display_warn("退票成功！");
    return SUCCESS;
}

/*
 * cmd_spoil_left_inv - spoil current invoice 
 *  @return : status
 */
int cmd_spoil_cur_inv(void)
{
    int ret;
    char title[50] = {0};
    struct issue_invoice_res issue_res;
    struct tax_sys_issue_invoice issue_inv;
    struct tax_sys_invoice_detail_record inv_detail;
    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();

    memset(&issue_inv, 0, sizeof(issue_inv));
    memset(&inv_detail, 0, sizeof(inv_detail));

    ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
    if (ret < 0) {
        if (ret == -ETAX_INVOICE_MC_EMPTY) {
            display_warn("当前发票卷已用尽，无法废票！");
            return FAIL;
        }

        display_err_msg(ret);
        return FAIL;
    }

    snprintf(title, 50, "当前发票号为: %010d，是否要将该票作废?", issue_inv.invoice_num);
    ret = question_user(title);
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在废票...");

    /* doing spoil invoice */ 
    issue_inv.invoice_type = SPOIL_INVOICE;
    ret = rt_ops->get_cur_date(&issue_inv.date);
    if (ret < 0) {
        display_err_msg(ret);
        return ret;
    }

    ret = tax_system->issue_invoice(&issue_inv, &issue_res);
    if (ret < 0) {
        display_err_msg(ret);
        return ret;
    }

    get_fiscal_code(&issue_res); 

    inv_detail.detail_date = issue_inv.date;
    inv_detail.detail_type = issue_inv.invoice_type;
    inv_detail.detail_inv_num = issue_inv.invoice_num;
    memcpy(inv_detail.detail_fiscal_code, issue_res.tax_num, 20);

    ret = tax_file_append_invoice_detail(&inv_detail);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    }

    display_warn("废票成功！");
    return SUCCESS;
}

/*
 * cmd_spoil_left_inv - spoil left invoice 
 *  @return : status
 */
int cmd_spoil_left_inv(void)
{
    int ret;
    char title[50] = {0};

    struct bcd_date today;
    struct issue_invoice_res issue_res;
    struct tax_sys_issue_invoice issue_inv;
    struct tax_sys_invoice_detail_record inv_detail;

    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_cur_roll_left_record * gp_crln = get_cur_roll_left();

    memset(&issue_inv, 0, sizeof(issue_inv));
    memset(&inv_detail, 0, sizeof(inv_detail));

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
    if (ret < 0) {
        if (ret == -ETAX_INVOICE_MC_EMPTY) {
            display_warn("当前发票卷已用尽，无法废票！");
            return FAIL;
        }

        display_err_msg(ret);
        return FAIL;
    }

    snprintf(title, 50, "当前剩余发票：%d，确定作废剩余票？", gp_crln->cur_roll_left);
    ret = question_user(title);
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在废票...");

    /* doing spoil invoice */ 
    issue_inv.date = today;
    inv_detail.detail_date = today;
    inv_detail.detail_type = SPOIL_INVOICE;
    issue_inv.invoice_type = SPOIL_INVOICE;

    while (1) {
        ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
        if (ret < 0) {
            if (ret != -ETAX_INVOICE_MC_EMPTY) {
                display_err_msg(ret);
                return FAIL;
            } else 
                break;
        }

        ret = tax_system->issue_invoice(&issue_inv, &issue_res);
        if (ret < 0) {
            display_err_msg(ret);
            return ret;
        }

        get_fiscal_code(&issue_res); 

        inv_detail.detail_inv_num = issue_inv.invoice_num;
        memcpy(inv_detail.detail_fiscal_code, issue_res.tax_num, 20);

        ret = tax_file_append_invoice_detail(&inv_detail);
        if (ret < 0) {
            display_err_msg(ret);
            return FAIL;
        }
    }

    display_warn("作废剩余票成功！");

    return SUCCESS;
}

/*
 * cmd_spoil_area_inv - the handler of spoil mul-invoces
 *  @return : status 
 */
int cmd_spoil_area_inv(void)
{
    int ret;
    int spoil_nb;
    char title[50] = {0};

    struct bcd_date today;
    struct issue_invoice_res issue_res;
    struct tax_sys_issue_invoice issue_inv;
    struct tax_sys_invoice_detail_record inv_detail;

    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_cur_roll_left_record * gp_crln = get_cur_roll_left();

    memset(&issue_inv, 0, sizeof(issue_inv));
    memset(&inv_detail, 0, sizeof(inv_detail));

    ret = rt_ops->get_cur_date(&today);
    if (ret < 0) {
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
    if (ret < 0) {
        if (ret == -ETAX_INVOICE_MC_EMPTY) {
            display_warn("当前发票卷已用尽，无法废票！");
            return FAIL;
        }

        display_err_msg(ret);
        return FAIL;
    }

input_nb:
    show_spoil_invoice_num();
    ret = get_inter_num(2, 1, &spoil_nb);
    if (ret == -EUI_ESC) { 
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_nb;
    }

    if (spoil_nb > gp_crln->cur_roll_left) {
        display_warn("废票数大于剩余发票数，请重新输入！");
        goto input_nb;
    }

    snprintf(title, 50, "确定作废%d张票？", spoil_nb);
    ret = question_user(title);
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在废票...");

    /* doing spoil invoice */ 
    issue_inv.date = today;
    inv_detail.detail_date = today;
    inv_detail.detail_type = SPOIL_INVOICE;
    issue_inv.invoice_type = SPOIL_INVOICE;

    int i;
    for (i = 0; i < spoil_nb; i ++) {
        ret = tax_system->get_invoice_nb(&issue_inv.invoice_num);
        if (ret < 0) {
            if (ret != -ETAX_INVOICE_MC_EMPTY) {
                display_err_msg(ret);
                return FAIL;
            } else 
                break;
        }

        ret = tax_system->issue_invoice(&issue_inv, &issue_res);
        if (ret < 0) {
            display_err_msg(ret);
            return ret;
        }

        get_fiscal_code(&issue_res); 

        inv_detail.detail_inv_num = issue_inv.invoice_num;
        memcpy(inv_detail.detail_fiscal_code, issue_res.tax_num, 20);

        ret = tax_file_append_invoice_detail(&inv_detail);
        if (ret < 0) {
            display_err_msg(ret);
            return FAIL;
        }
    }

    display_warn("废票成功！");

    return SUCCESS;
}

/* end of transact.c */
