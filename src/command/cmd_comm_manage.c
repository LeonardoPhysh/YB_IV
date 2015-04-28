/*
 * com_comm_manage.c
 *  - implement function of plu manage
 * 
 * Author : Leonardo Physh 
 * Date   : 2014.9.24
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
#include "tax_file_op.h"
#include "ui_api.h"
#include "tax_system.h"
#include "plu.h"
#include "input.h"
#include "common.h"
#include "config.h"

/*
 * UI Helper
 */
int show_plu_item(int type, char * title, struct plu_item * plu_item) 
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 5;

    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", title);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    if (type == BY_PLU_NUM)
        snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d", "编号： ", plu_item->plu_num);
    else 
        snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%s", "条形码： ", plu_item->barcode);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%s", "名称： ", plu_item->name);

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    snprintf(frame.items[3].title, MAX_TITLE_LEN, "%s%.2f", "单价： ", (float)plu_item->price);

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 7;
    snprintf(frame.items[4].title, MAX_TITLE_LEN, "%s%d", "库存: ", plu_item->stock);

    show_simple_frame(&frame);
    return SUCCESS;
}

/*
 * get_fis_type - get a tax rate index
 *  @return : tax index 
 */
int get_fis_type(void)
{ 
    int i, key_code, page, pos;
    int fis_type_num, page_item = 0;

    struct simple_frame frame;
    struct tax_sys_app_info * app_info = get_sys_app_info();

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;

    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "请选择税种税率");

    fis_type_num = app_info->fis_type_num;
    if (fis_type_num == 0) {
        display_warn("卡中未添加税目税种信息！");
        return FAIL;
    }

    i = 1;
    pos = 1;
    page = 0;
    clear_cache();
    while (1) {
        if (fis_type_num - i >= 0 ) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%d-%s(%d)", i,
                    app_info->fis_type[i - 1].item_cn_name,
                    app_info->fis_type[i - 1].index);
            page_item = 1;
        } else {
            memset(frame.items[1].title, 0, MAX_TITLE_LEN);
        }

        if (fis_type_num - i >= 1) {
            snprintf(frame.items[2].title, MAX_TITLE_LEN, "%d-%s(%d)", i, 
                    app_info->fis_type[i].item_cn_name,
                    app_info->fis_type[i].index);
            page_item = 2;
        } else {
            memset(frame.items[2].title, 0, MAX_TITLE_LEN);
        }

        if (fis_type_num - i >= 2) {
            snprintf(frame.items[3].title, MAX_TITLE_LEN, "%d-%s(%d)", i, 
                    app_info->fis_type[i + 1].item_cn_name,
                    app_info->fis_type[i + 1].index);
            page_item = 3;
        } else {
            memset(frame.items[3].title, 0, MAX_TITLE_LEN);
        }

        show_simple_frame(&frame);
        highlight_on(1 + pos);

        key_code = get_keycode();
        switch (key_code) {
            case BACK:
                return -EUI_BACK;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case ENTER:
                return app_info->fis_type[i + pos - 2].index; 
                break;

            case UP:
                if (pos > 1) {
                    highlight_off(1 + pos--);
                    highlight_on(1 + pos);
                } else {
                    if (page > 0) {
                        page --;
                        i -= 3;
                        highlight_off(1 + pos);
                        pos = 3;
                        highlight_on(1 + pos);

                    }
                }
                break;

            case DOWN:
                if (pos < 2) {
                    highlight_off(1 + pos++);
                    highlight_on(1 + pos);
                } else {
                    if (fis_type_num - i >= 3) {
                        page ++;
                        i += 3;
                        highlight_off(1 + pos);
                        pos = 1;
                        highlight_on(1 + pos);
                    }
                }
                break;

            default:
                if (isdigit(key_code)) {
                    if (key_code >= '1' && (key_code - '0') < page_item) {
                        return app_info->fis_type[i + (key_code - '0') - 2].index;
                    } 
                } 
                break; 
        }    
    }
}

char * get_fis_type_name(int index)
{
    int rate_nb;
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    for (rate_nb = 0; rate_nb < MAX_USER_TAXRATE; rate_nb ++) {
        if (index == gp_app_info->tax_index[rate_nb])
            break;
    }

    if (rate_nb == MAX_USER_TAXRATE)
        return NULL;

    return gp_app_info->fis_type[rate_nb].item_cn_name;
}

/**
 * do_view_dpt - list all dpt item 
 *  @return : choosen dpt_num
 */
int do_view_dpt(char *title)
{
    int ret;
    int key_code;
    int i, count = 0, pos = 0; 
    struct simple_frame frame;
    struct dpt_item dpt_items[20];
    struct plu_operate *plu_ops = get_plu_ops();

    memset(&frame, 0, sizeof(frame));
    memset(dpt_items, 0, sizeof(struct dpt_item) * 20);

    for (i = 1; i <= 20; i++) {
        ret = plu_ops->is_free_dpt(i);
        if (ret == POSITIVE)
            continue;

        ret = plu_ops->read_dpt(i, &dpt_items[count++]);
        if (ret != SUCCESS)
            return FAIL;
    }

    if (count == 0) {
        display_warn("无部类信息，请先添加部类！");
        return -EUI_ESC;
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
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 1, dpt_items[i].name);
            snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 2, dpt_items[i + 1].name);
            snprintf(frame.items[3].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 3, dpt_items[i + 2].name);
        } else if (count - i == 2) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 1, dpt_items[i].name);
            snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 2, dpt_items[i + 1].name);
            memset(frame.items[3].title, 0, MAX_TITLE_LEN);
        } else if (count - i == 1) {
            snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d %s", "部类", i + 1, dpt_items[i].name);
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
                return dpt_items[i + pos - 1].dpt_num; 
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
 * do_view_plu - find a plu_item
 *  @type : BY_BARCODE/BY_NAME
 *  @return : plu_num
 */
int do_view_plu_by_num(struct plu_item *item)
{
    int ret;
    int plu_num; 
    struct plu_item plu_item;
    struct plu_operate * plu_ops = get_plu_ops();

input_num:
    clear_screen();
    show_str(1, 1, "请输入商品编号：");
    ret = get_inter_num(2, 1, &plu_num);
    if (ret == EUI_ESC) {
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_num;
    }

    ret = plu_ops->read_plu(plu_num, &plu_item);
    if (ret != SUCCESS) {
        if (ret != -EFILE_OPEN_FAIL) {
            display_warn("无该商品编号记录！");
            goto input_num;
        }

        display_err_msg(ret, "查询商品失败！");
        return FAIL;
    }

    *item = plu_item;
    return SUCCESS;
}

int do_view_plu_by_barcode(struct plu_item *item)
{
    int ret;
    char barcode[14] = {0};
    struct plu_item plu_item;
    struct plu_operate * plu_ops = get_plu_ops();

#ifndef CONFIG_HPP
    plu_item = plu_item;
    plu_ops = plu_ops;
    barcode[0] = barcode[1]; // for killing warning 

    display_warn("尚未配置条码枪，无法进行操作");
    ret = FAIL;
#else 
get_bc:    
    clear_screen();
    show_str(1, 1, "请扫描条形码：");

    ret = get_barcode(2, 1, barcode);
    if (ret != SUCCESS)
        return FAIL;

    ret = plu_ops->index_by_bc(barcode, &plu_item);
    if (ret != SUCCESS) {
        if (ret == -EPLU_NO_BARCODE) {
            display_warn("无该条形码记录！");
            goto get_bc;
        }

        display_err_msg(ret, "查询商品失败，请联系厂商！");
        return FAIL;
    }

    *item = plu_item;
    ret = SUCCESS;
#endif 

    return ret;
}

int do_view_plu_by_name(struct plu_item * item)
{
    int ret;
    char name[24] = {0};
    struct plu_item plu_item;
    struct plu_operate * plu_ops = get_plu_ops(); 

input_name:
    clear_screen();
    show_str(1, 1, "请输入商品名称");
    ret = get_chn_str(2, 1, name);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_name;
    }

    ret =plu_ops->index_by_name(name, &plu_item);
    if (ret != SUCCESS) {
        if (ret == -EPLU_NO_NAME) {
            display_warn("无该商品名记录！");
            goto input_name;
        }        

        display_err_msg(ret, "查询商品失败，请联系厂商！");
        return FAIL;
    }

    *item = plu_item;
    return SUCCESS;
}


/*
 * cmd_add_dpt - service for sub-menu "部类添加"
 *  @return : status 
 */
int cmd_add_dpt(void)
{
    int ret;
    int dpt_num;
    char dpt_name[DPT_NAME_LEN + 1] = {0};

    struct dpt_item dpt_item;
    struct simple_frame frame;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未初始化，无法进行操作！");
        return FAIL;
    }

    dpt_num = plu_ops->get_free_dpt();
    memset(&dpt_item, 0, sizeof(dpt_item));
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "部类添加");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d", "编号：", dpt_num);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s", "名称：");

add_dpt:
    show_simple_frame(&frame);
    ret = get_chn_str(3, 4, dpt_name);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;

        goto add_dpt; 
    } else if (ret == -EUI_BACK)
        return FAIL;

    /* get_chn_str do not check the string for up-layer, 
     * we need to check it ourself */
    if (strlen(dpt_name) == 0) {
        display_warn("部类名称不能为空！");
        goto add_dpt;
    } 

    dpt_item.dpt_num = dpt_num;
    memcpy(dpt_item.name, dpt_name, DPT_NAME_LEN + 1);

    display_info("正在添加部类...");

    ret = plu_ops->append_dpt(dpt_num, &dpt_item);
    if (ret != SUCCESS) {
        display_err_msg(ret, "添加部类错误！");
        return FAIL;
    }

    display_warn("添加成功部类成功！");
    return SUCCESS;
}

/*
 * cmd_del_dpt - service for sub-menu "部类删除"
 *  @return : status 
 */
int cmd_del_dpt(void)
{
    int ret;
    int dpt_num;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未初始化，无法进行操作！");
        return FAIL;
    }

view_dpt:
    dpt_num = do_view_dpt("请选择删除的部类");
    switch (dpt_num) {
        case -EUI_BACK:
            return FAIL;
            break;

        case -EUI_ESC:
            return FAIL;
            break;

        default:
            if (dpt_num >= 1 && dpt_num <= 20) {
                ret = question_user("确认删除该部类？");
                if (ret != POSITIVE)
                    goto view_dpt;

                ret = plu_ops->delete_dpt(dpt_num);
                if (ret != SUCCESS) {
                    if (ret == -EPLU_DPT_NOT_EMPTY) {
                        display_warn("部类非空,无法删除!");
                    } else {
                        display_warn("删除部类失败，请联系厂商！");
                    }

                    return FAIL;
                }
            } else {
                display_warn("未知错误，请联系厂商！");
                return FAIL;
            }
            break;
    }

    return SUCCESS;
}

/*
 * cmd_modify_dpt - service for sub-menu "部类修改"
 *  @return : status 
 */
int cmd_modify_dpt(void)
{
    int ret;
    int dpt_num;
    char dpt_name[DPT_NAME_LEN + 1];
    struct dpt_item dpt_item;
    struct simple_frame frame;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    memset(&dpt_item, 0, sizeof(dpt_item));
    memset(&frame, 0, sizeof(frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未初始化，无法进行操作！");
        return FAIL;
    }

view_dpt:
    dpt_num = do_view_dpt("请选择修改的部类");
    switch (dpt_num) {
        case -EUI_BACK:
            return FAIL;
            break;

        case -EUI_ESC:
            return FAIL;
            break;

        default:
            if (dpt_num >= 1 && dpt_num <= 20) {
                ret = question_user("确认修改该部类？");
                if (ret != POSITIVE)
                    goto view_dpt;
                break;
            } else {
                display_warn("未知错误，请联系厂商！");
                return FAIL;
            }
    }

    ret = plu_ops->read_dpt(dpt_num, &dpt_item);
    if (ret != SUCCESS) {
        display_warn("未知错误，请联系厂商！");
        return FAIL;
    }

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "部类修改");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d", "编号：", dpt_num);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s", "名称：");

dpt_name:
    show_simple_frame(&frame);

    ret = get_chn_str(3, 4, dpt_name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE) {
            return FAIL;
        }
        goto dpt_name;
    } else if (ret == -EUI_BACK) {
        goto view_dpt;
    }
    
    /* check dpt_name */
    if (strlen(dpt_name) == 0) {
        display_info("部类名字不能为空！");
        goto dpt_name;
    }

    display_info("正在修改部类...");

    ret = plu_ops->modify_dpt(dpt_num, &dpt_item);
    if (ret != SUCCESS) {
        display_err_msg(ret, "修改部类错误！");
        return FAIL;
    }

    display_warn("修改成功部类成功！");
    return SUCCESS;
}

/*
 * cmd_view_dpt - service for sub-menu "部类查询"
 */
int cmd_view_dpt(void)
{
    int ret;
    int dpt_num;
    struct dpt_item dpt_item;
    struct simple_frame frame;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    memset(&dpt_item, 0, sizeof(dpt_item));
    memset(&frame, 0, sizeof(frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未初始化，无法进行操作！");
        return FAIL;
    }

    dpt_num = do_view_dpt("请选择查询的部类");
    switch (dpt_num) {
        case -EUI_BACK:
            return FAIL;
            break;

        case -EUI_ESC:
            return FAIL;
            break;

        default:
            if (!(dpt_num >= 1 && dpt_num <= 20)) {
                display_warn("未知错误，请联系厂商！");
                return FAIL;
            }
            break;
    }

    ret = plu_ops->read_dpt(dpt_num, &dpt_item);
    if (ret != SUCCESS) {
        display_warn("查询部类失败！");
        return FAIL;
    }

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "部类查询");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d", "编号：", dpt_num);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s%s", "名称：", dpt_item.name);

    show_simple_frame(&frame);

    get_anykey();
    return SUCCESS;
}

/*
 * cmd_add_plu - service for sub-menu "添加商品"
 *  @return : statsu 
 */
int cmd_add_plu(void)
{
    int ret;
    int dpt_num;
    int plu_num;
    int tax_index; 
    struct plu_item plu_item;
    struct simple_frame frame;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    memset(&plu_item, 0, sizeof(plu_item));
    memset(&frame, 0, sizeof(frame));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("税控功能未初始化，无法进行操作！");
        return FAIL;
    }

view_dpt:
    dpt_num = do_view_dpt("请选择商品所属部类");
    switch (dpt_num) {
        case -EUI_BACK:
            return FAIL;
            break;

        case -EUI_ESC:
            return FAIL;
            break;

        default:
            if (!(dpt_num >= 1 && dpt_num <= 20)) {
                display_warn("未知错误，请联系厂商！");
                return FAIL;
            }
            break;
    }

fis_type: 
    tax_index = get_fis_type();
    if (tax_index < 0) {
        if (tax_index == -EUI_ESC) { 
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto fis_type;
        } else if (tax_index == -EUI_BACK)
            goto view_dpt; 
    }

    plu_num = plu_ops->get_free_plu(dpt_num);
    if (plu_num < 0) {
        display_warn("添加商品出错，请联系厂商！");
        return FAIL;
    }

    plu_item.plu_num = plu_num;
    plu_item.tax_index = tax_index;

    //frame.item_num = 5;
    frame.item_num = 3;

    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "商品添加");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s%d", "编号：", plu_num);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s", "名称：");

#if 0
    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    snprintf(frame.items[3].title, MAX_TITLE_LEN, "%s", "单价：");

    frame.items[4].pos.row = 4;
    frame.items[4].pos.col = 7;
    snprintf(frame.items[4].title, MAX_TITLE_LEN, "%s", "库存:");
#endif 

plu_name:
    show_simple_frame(&frame);

    ret = get_chn_str(3, 4, plu_item.name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto plu_name;
    } else if (ret == -EUI_BACK)
        goto fis_type;
    
    /* check plu name */ 
    if (strlen(plu_item.name) == 0) {
        display_warn("商品名字不能为空！");
        goto plu_name;
    }

    show_str(4, 1, "单价: ");
    show_str(4, 7, "库存: ");
    
    /* price can not be zero */
    do {
        ret = get_inter_num(4, 4, (int *)&plu_item.price);
        if (ret == -EUI_ESC) {
            ret = question_user("确定取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;

            goto plu_name;
        }
    } while(plu_item.price == 0);
    
    /* stock can not be zero */
    do {
        ret = get_inter_num(4, 10, (int *)&plu_item.stock);
        if (ret == -EUI_ESC) {
            ret = question_user("确定取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;

            goto plu_name;
        }
    }while (plu_item.stock == 0);

    display_info("正在添加商品...");

    ret = plu_ops->append_plu(plu_num, &plu_item);
    if (ret != SUCCESS) {
        display_warn("添加商品出错，请联系厂商！");
        return FAIL;
    }

    display_warn("添加商品成功！");
    return SUCCESS;
}

/*
 *cmd_del_plu_by_name - service for sub-menu "删除商品-按编号查询"
 * return : status 
 */
int cmd_del_plu_by_num(void)
{
    int ret;
    int key;
    struct plu_item plu_item;
    struct plu_operate *plu_ops = get_plu_ops();

    ret = do_view_plu_by_num(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品删除", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认删除该商品？");
    if (ret != POSITIVE)
        goto show_plu;

    display_info("正在删除商品...");

    ret = plu_ops->delete_plu(plu_item.plu_num);
    if (ret < 0) {
        display_err_msg(ret, "删除商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("删除商品成功！");
    return SUCCESS;
}

/*
 * cmd_del_plu_by_barcode - service for sub-menu "商品删除-按条形码查询"
 *  @return : status 
 */
int cmd_del_plu_by_barcode(void)
{
    int ret;
    int key;
    struct plu_item plu_item;
    struct plu_operate *plu_ops = get_plu_ops();

    ret = do_view_plu_by_barcode(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_BARCODE, "商品删除", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认删除该商品？");
    if (ret != POSITIVE)
        goto show_plu;

    display_info("正在删除商品...");

    ret = plu_ops->delete_plu(plu_item.plu_num);
    if (ret < 0) {
        display_err_msg(ret, "删除商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("删除商品成功！");
    return SUCCESS;
}

/*
 * cmd_del_plu_by_name - service for sub-menu "删除商品-按名字查询"
 *  @return : status
 */
int cmd_del_plu_by_name(void)
{
    int ret;
    int key;
    struct plu_item plu_item;
    struct plu_operate * plu_ops = get_plu_ops();

    ret = do_view_plu_by_name(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品删除", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认删除该商品？");
    if (ret != POSITIVE)
        goto show_plu;

    display_info("正在删除商品...");

    ret = plu_ops->delete_plu(plu_item.plu_num);
    if (ret < 0) {
        display_err_msg(ret, "删除商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("删除商品成功！");
    return SUCCESS;
}

/*
 * cmd_modify_plu_by_num - service for sub-menu "修改商品-按编号查询"
 *  @return : status 
 */
int cmd_modify_plu_by_num(void)
{
    int ret;
    int key; 
    int new_count, new_price;
    char new_name[24] = {0};
    struct plu_item plu_item;
    struct plu_item new_item;
    struct plu_operate * plu_ops = get_plu_ops();

    ret = do_view_plu_by_num(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品修改", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认修改该商品？");
    if (ret != POSITIVE)
        goto show_plu;

input_name:
    clear_screen();
    show_str(1, 1, "请输入新的商品名称：");
    ret = get_chn_str(2, 1, new_name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_name;
    }
    
    if (strlen(new_name) == 0) {
        display_warn("商品名称不能为空！");
        goto input_name;
    }

input_price:
    clear_screen();
    show_str(1, 1, "请输入新的商品单价：");
    ret = get_inter_num(2, 1, &new_price);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_price;
    }

    if (new_price == 0) {
        display_warn("商品单价不能为零！");
        goto input_price;
    }

input_count:
    clear_screen();
    show_str(1, 1, "请输入新的商品数量：");
    ret = get_inter_num(2, 1, &new_count);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_count;
    }
    
    if (new_count == 0) {
        display_warn("商品数量不能为零！");
        goto input_count;
    }

    display_info("正在修改商品...");

    new_item = plu_item;
    new_item.price = new_price;
    new_item.stock = new_count;
    memcpy(new_item.name, new_name, 24);

    ret = plu_ops->modify_plu(plu_item.plu_num, &new_item);
    if (ret < 0) {
        display_err_msg(ret, "修改商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("修改成功！");
    return SUCCESS;
}

/*
 * cmd_modify_plu_by_barcode - service for sub-menu "商品修改-按条形码查询"
 *  @return : status 
 */
int cmd_modify_plu_by_barcode(void)
{
    int ret;
    int key;
    int new_count, new_price;
    char new_name[24] = {0};
    struct plu_item new_item;
    struct plu_item plu_item;
    struct plu_operate *plu_ops = get_plu_ops();

    ret = do_view_plu_by_barcode(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_BARCODE, "商品修改", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认修改该商品？");
    if (ret != POSITIVE)
        goto show_plu;

input_name:
    clear_screen();
    show_str(1, 1, "请输入新的商品名称：");
    ret = get_chn_str(2, 1, new_name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_name;
    }

input_price:
    clear_screen();
    show_str(1, 1, "请输入新的商品单价：");
    ret = get_inter_num(2, 1, &new_price);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_price;
    }

input_count:
    clear_screen();
    show_str(1, 1, "请输入新的商品数量：");
    ret = get_inter_num(2, 1, &new_count);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_count;
    }

    display_info("正在修改商品...");

    new_item = plu_item;
    new_item.price = new_price;
    new_item.stock = new_count;
    memcpy(new_item.name, new_name, 24);

    ret = plu_ops->modify_plu(plu_item.plu_num, &new_item);
    if (ret < 0) {
        display_err_msg(ret, "修改商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("修改成功！");
    return SUCCESS;
}

/*
 * cmd_modify_plu_by_name - service for sub-menu "商品修改-按名字查询"
 *  @return : status
 */
int cmd_modify_plu_by_name(void)
{
    int ret;
    int key;
    int new_count, new_price;
    char new_name[24] = {0};
    struct plu_item new_item;
    struct plu_item plu_item;
    struct plu_operate * plu_ops = get_plu_ops();

    ret = do_view_plu_by_name(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品修改", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    ret = question_user("确认修改该商品？");
    if (ret != POSITIVE)
        goto show_plu;

input_name:
    clear_screen();
    show_str(1, 1, "请输入新的商品名称：");
    ret = get_chn_str(2, 1, new_name);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_name;
    }

input_price:
    clear_screen();
    show_str(1, 1, "请输入新的商品单价：");
    ret = get_inter_num(2, 1, &new_price);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_price;
    }

input_count:
    clear_screen();
    show_str(1, 1, "请输入新的商品数量：");
    ret = get_inter_num(2, 1, &new_count);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_count;
    }

    display_info("正在修改商品...");

    new_item = plu_item;
    new_item.price = new_price;
    new_item.stock = new_count;
    memcpy(new_item.name, new_name, 24);

    ret = plu_ops->modify_plu(plu_item.plu_num, &new_item);
    if (ret < 0) {
        display_err_msg(ret, "修改商品失败，请联系厂商！");
        return FAIL;
    }

    display_warn("修改成功！");
    return SUCCESS;
}

/*
 * cmd_view_plu_by_num - service for sub-menu "按商品编号查询"
 *  @return : status 
 */
int cmd_view_plu_by_num(void)
{
    int ret;
    int key;
    struct plu_item plu_item;

    ret = do_view_plu_by_num(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品查询", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    return SUCCESS;
}

/*
 * cmd_view_plu_by_barcode - service for sub-menu "按条形码查询"
 * @return : status 
 */
int cmd_view_plu_by_barcode(void)
{
    int ret;
    int key;
    struct plu_item plu_item;

    ret = do_view_plu_by_barcode(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品查询", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    return SUCCESS;
}

/*
 * cmd_view_plu_by_name - service for sub-menu "按名字查询"
 *  @return : status 
 */
int cmd_view_plu_by_name(void)
{
    int ret, key;
    struct plu_item plu_item;

    ret = do_view_plu_by_name(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品查询", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

    return SUCCESS;
}

/*
 * cmd_stock_manage - service for sub-menu "进货管理"
 *  @return : status 
 */
int cmd_stock_manage(void)
{
    int ret;
    int key, new_stock;
    struct plu_item plu_item;
    struct plu_operate *plu_ops = get_plu_ops();

    ret = do_view_plu_by_num(&plu_item);
    if (ret != SUCCESS)
        return ret;

show_plu:
    show_plu_item(BY_PLU_NUM, "商品信息", &plu_item);

    clear_cache();
    do {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_plu;
        }
    } while (key != ENTER);

input_stock:
    clear_screen();
    display_str(1, 1, "当前库存: %d", plu_item.stock);
    show_str(2, 1, "输入新库存: ");
    ret = get_inter_num(2, 7, &new_stock);
    if (ret == -EUI_ESC) {
        ret = question_user("确定取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto input_stock;
    }

    plu_item.stock = new_stock;

    ret = plu_ops->modify_plu(plu_item.plu_num, &plu_item);
    if (ret != SUCCESS) {
        display_err_msg(ret, "进货管理失败");
        return FAIL;
    }

    display_warn("修改成功！");
    return SUCCESS;
}


