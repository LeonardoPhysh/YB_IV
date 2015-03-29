/*
 * com_system_manage.c
 *  - implement function of system manage
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
 * UI helper 
 */
int show_add_user_perm(void)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "请选择用户权限");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 4;
    sprintf(frame.items[1].title, "1-一般用户");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 4;
    sprintf(frame.items[2].title, "2-超级用户");

    show_simple_frame(&frame);
    return SUCCESS;
}

int show_add_user_info(char *title, struct user * user)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "%s", title);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "用户名： %s", user->name);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "密    码：");

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "确认密码：");

    show_simple_frame(&frame);
    return SUCCESS;
}

int show_print_info(struct printer_type * printer)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 3;
    sprintf(frame.items[0].title, "当前打印机信息");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "型号： %s", printer->name);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "状态：%s", printer->state & PRINT_UP ? 
            "正常" : "异常");

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "类型：%s打印机", printer->id == FLAT_PRINT_TYPE ?
            "平推式" : "卷式");

    show_simple_frame(&frame);
    return SUCCESS;
}

/*
 * common base APIs
 */
int do_view_user(char *title)
{
    int ret;
    int i, key_code, pos, page, page_item, user_num;
    struct simple_frame frame;
    struct user users[MAX_USER_NUM];
    char * level[] = {
        " ",
        "普通用户",
        "超级用户",
        "管理用户",
        "稽查用户",
        "开发用户",
    };

    memset(&frame, 0, sizeof(frame));
    memset(users, 0, sizeof(struct user) * MAX_USER_NUM);

    /*
     * read all user's information in this machine
     */
    user_num = tax_file_get_rec_num(USER_FILE);
    for (i = 0; i < user_num; i++) {
        ret = tax_file_read_user(i + 1, &users[i]);
        if (ret != SUCCESS) 
            goto fail;
    }

    frame.item_num = 7;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = (12 - strlen(title) / 2) / 2;

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    frame.items[2].pos.row = 2;
    frame.items[2].pos.col = 9;

    frame.items[3].pos.row = 3;
    frame.items[3].pos.col = 1;
    frame.items[4].pos.row = 3;
    frame.items[4].pos.col = 9;

    frame.items[5].pos.row = 4;
    frame.items[5].pos.col = 1;
    frame.items[6].pos.row = 4;
    frame.items[6].pos.col = 9;

    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", title);

    set_ime_status(INPUT_LOW_CASE);

    i = 1;
    page_item = pos = page = 0;

    clear_cache();
    while (1) {
        if (user_num - i >= 0) {
            sprintf(frame.items[1].title, "%s", users[i - 1].name);
            sprintf(frame.items[2].title, "%s", level[users[i - 1].level]);
            page_item = 1;
        } 

        if (user_num - i >= 1) {
            sprintf(frame.items[3].title, "%s", users[i].name);
            sprintf(frame.items[4].title, "%s", level[users[i].level]);
            page_item = 2;
        }

        if (user_num - i >= 2) {
            sprintf(frame.items[5].title, "%s", users[i + 1].name);
            sprintf(frame.items[6].title, "%s", level[users[i + 1].level]);
            page_item = 3;
        }

        show_simple_frame(&frame);
        //highlight_line(pos + 1);

        key_code = get_keycode();
        switch (key_code) {
            case BACK:
                return -EUI_BACK;
                break;

            case ESC:
                return -EUI_ESC;
                break;

            case ENTER:
                return i + pos - 1;
                break;

            case UP:
                if (pos > 0) {
                    pos --;

                } else {
                    if (page > 0) {
                        page --;
                        pos = 2;
                        i -= 3;
                    }
                }
                break;

            case DOWN:
                if (pos < 2) {
                    pos ++;
                } else {
                    if (user_num - i >= 3) {
                        page ++;
                        pos = 0;
                        i += 3;
                    }
                }
                break;

            default:
                if (isdigit(key_code)) {
                    if (key_code >= '1' && (key_code - '0') < page_item) {
                        return i + (key_code - '0') - 2;
                    }
                }
        }
    }

fail:
    display_warn("查询用户失败！");
    display_err_msg(ret);
    return ret;

}

/*
 * cmd_add_user - service for sub-menu "添加用户"
 *  @return : status 
 */
int cmd_add_user(void)
{
    int ret;
    int key, user_perm;
    char passwd[20] = {0};
    struct user new_user;

    /*
     * only manager can do this. 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret != SUCCESS){
        display_warn("权限限制，无法继续此操作！");
        return FAIL;
    }

show_perm:
    show_add_user_perm();

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();
    while (1) {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确定取消当前操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_perm;
        }

        if (key == '1') {
            user_perm = NORMAL_USER;
            break;
        }

        if (key == '2') {
            user_perm = SUPPER_USER;
            break;
        }
    }

    memset(&new_user, 0, sizeof(new_user));

show_user:
    if (user_perm == NORMAL_USER)
        show_add_user_info("添加普通用户", &new_user);
    else 
        show_add_user_info("添加超级用户", &new_user);

    ret = get_string(2, 5, new_user.name);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto show_user;
    }

    ret = get_passwd(3, 6, new_user.passwd);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE) 
            return FAIL;
        else {
            memset(new_user.passwd, 0, sizeof(new_user.passwd));
            goto show_user;
        }
    }

    ret = get_passwd(4, 6, passwd);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(passwd, 0, sizeof(passwd));
            goto show_user; 

        }
    }

    if (strcmp(passwd, new_user.passwd) != 0) {
        display_warn("两次输入密码不匹配，请重新输入！");
        memset(new_user.passwd, 0, sizeof(new_user.passwd));
        memset(passwd, 0, sizeof(passwd));
        goto show_user;
    }

    display_info("正在添加新用户...");

    new_user.id = tax_file_get_rec_num(USER_FILE) + 1;
    new_user.level = user_perm;

    /* Add it */
    ret = tax_file_append_user(&new_user);
    if (ret < 0) {
        if (ret == -ETAX_USERNAME_IMPLICT) {
            display_warn("用户名冲突，请重新输入！");
            memset(new_user.name, 0, sizeof(new_user.name));
            memset(new_user.passwd, 0, sizeof(new_user.passwd));
            memset(passwd, 0, sizeof(passwd));
            goto show_user; 
        }

        if (ret == -EFILE_REC_FULL) {
            display_warn("用户数量超限，无法添加新用户！");
            return FAIL;
        }

        display_warn("添加用户失败，请联系厂商！");
        display_err_msg(ret);
        return FAIL;
    }

    display_warn("添加用户成功！");
    return SUCCESS;
}


/*
 * cmd_del_user - service for sub-menu "删除用户"
 *  @return : status 
 */
int cmd_del_user(void)
{
    int ret;
    int offset;
    struct user del_user;

    /*
     * only manager can do this. 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret != SUCCESS){
        display_warn("权限限制，无法继续此操作！");
        return FAIL;
    }

    ret = do_view_user("选择删除用户");
    if (ret == -EUI_BACK)
        return FAIL;
    else if (ret == -EUI_ESC)
        return FAIL;
    else if (ret >= 0)
        offset = ret;
    else {
        display_warn("未知错误，请联系厂商！");
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_file_read_user(offset, &del_user);
    if (ret < 0) {
        display_warn("删除用户失败！");
        display_err_msg(ret);
        return FAIL;
    }

    if (del_user.level >= MANAGER_USER) {
        display_warn("只能删除非管理员用户！");
        return FAIL;
    }

    ret = tax_file_delete_user(offset);
    if (ret < 0) {
        display_warn("删除用户失败！");
        display_err_msg(ret);
        return FAIL;
    }

    display_warn("删除用户成功！");
    return SUCCESS;
}

/*
 * cmd_modify_user - service for sub-menu "修改用户"
 *  @return : status
 */
int cmd_modify_user(void)
{
    int ret;
    int key, offset, new_perm;
    char passwd[20] = {0};
    struct user old_user, tmp_user, find_user;

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法继续操作！");
        return FAIL;
    }

    ret = do_view_user("选择修改用户");
    if (ret == -EUI_BACK)
        return FAIL;
    else if (ret == -EUI_ESC)
        return FAIL;
    else if (ret >= 0)
        offset = ret;
    else {
        display_warn("未知错误，请联系厂商！");
        display_err_msg(ret);
        return FAIL;
    }

    ret = tax_file_read_user(offset, &old_user);
    if (ret == SUCCESS) {
        display_warn("修改用户失败！");
        display_err_msg(ret);
        return FAIL;
    }

show_perm:
    show_add_user_perm();

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();
    while (1) {
        key = get_keycode();
        if (key == ESC) {
            ret = question_user("确定取消当前操作？");
            if (ret == POSITIVE)
                return FAIL;
            else 
                goto show_perm;
        }

        if (key == '1') {
            new_perm = NORMAL_USER;
            break;
        }

        if (key == '2') {
            new_perm = SUPPER_USER;
            break;
        }
    }

    memset(&tmp_user, 0, sizeof(tmp_user));

show_user:
    show_add_user_info("修改用户信息", &tmp_user);

    ret = get_string(2, 5, tmp_user.name);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto show_user;
    }

    ret = get_passwd(3, 6, tmp_user.passwd);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消当前操作？");
        if (ret == POSITIVE) 
            return FAIL;
        else {
            memset(tmp_user.passwd, 0, sizeof(tmp_user.passwd));
            goto show_user;
        }
    }

    ret = get_passwd(4, 6, passwd);
    if (ret == -EUI_ESC) {
        ret = question_user("确认取消本次操作？");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(passwd, 0, sizeof(passwd));
            goto show_user; 
        }
    }

    if (strcmp(passwd, tmp_user.passwd) != 0) {
        display_warn("两次输入密码不匹配，请重新输入！");
        memset(tmp_user.passwd, 0, sizeof(tmp_user.passwd));
        memset(passwd, 0, sizeof(passwd));
        goto show_user;
    }

    /*
     * if username been changed, it should not equal to any other user 
     */
    if (strcmp(tmp_user.name, old_user.name) != 0) {
        ret = tax_file_find_user(tmp_user.name, &find_user);
        if (ret == SUCCESS) {
            display_warn("用户名冲突，请重新输入!");
            memset(tmp_user.passwd, 0, sizeof(tmp_user.passwd));
            memset(passwd, 0, sizeof(passwd));
            goto show_user;
        }
    }

    /*
     * it's safe to modify the record 
     */ 
    tmp_user.id = old_user.id;
    tmp_user.level = new_perm;
    ret = tax_file_modify_user(offset, &tmp_user);
    if (ret != SUCCESS) {
        display_warn("修改用户失败！");
        display_err_msg(ret);
        return FAIL;
    }

    display_warn("修改用户成功！");
    return SUCCESS;
}

/*
 * cmd_view_user - service for sub-menu "查询用户"
 *  @return : statu
 */
int cmd_view_user(void)
{
    int ret;

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法继续操作！");
        return FAIL;
    }

    do_view_user("查询用户信息");

    return SUCCESS;
}

/*
 * cmd_view_ower_info - service for sub-menu "系统信息-机主信息"
 * @return : status
 */
int cmd_view_ower_info(void)
{
    int ret, key;
    int office_code;
    char buf[25] = {0};
    struct simple_frame frames[4];
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret == NEGATIVE) {
        display_warn("本机尚未初始化！");
        return FAIL;
    } else if (ret < 0) {
        display_warn("未知错误，请联系厂商！");
        return FAIL;
    }

    memset(&frames, 0, sizeof(struct simple_frame) * 4);

    /* page 1 */
    frames[0].item_num = 3;
    frames[0].items[0].pos.row = 1;
    frames[0].items[0].pos.col = 1;
    sprintf(frames[0].items[0].title, "IC卡编号：");

    frames[0].items[1].pos.row = 2;
    frames[0].items[1].pos.col = 2;
    bcd_to_str(gp_app_info->fiscal_card_nb, buf, 8);
    sprintf(frames[0].items[0].title, "%s", buf);

    frames[0].items[2].pos.row = 3; 
    frames[0].items[2].pos.col = 1;
    sprintf(frames[0].items[2].title, "纳税人：%s", gp_app_info->taxpayer_name);

    memset(buf, 0, 25);

    /* page 2*/
    frames[1].item_num = 4;
    frames[1].items[0].pos.row = 1;
    frames[1].items[0].pos.col = 1;
    bcd_to_str((uchar *)&gp_app_info->app_start_date, buf, 4);
    sprintf(frames[1].items[0].title, "应用起始日期：%s", buf);

    frames[1].items[1].pos.row = 2;
    frames[1].items[1].pos.col = 1;
    bcd_to_str((uchar *)&gp_app_info->app_vaild_date, buf, 4);
    sprintf(frames[1].items[1].title, "应用有效日期：%s", buf);

    frames[1].items[2].pos.row = 3; 
    frames[1].items[2].pos.col = 1;
    bcd_to_str((uchar *)&gp_app_info->issue_limit_date, buf, 4);
    sprintf(frames[1].items[2].title, "开票截止日期：%s", buf);

    frames[1].items[3].pos.row = 4; 
    frames[1].items[3].pos.col = 1;
    sprintf(frames[1].items[3].title, "申报方式：%s", gp_app_info->declare_mode == 0x01 ? "用户卡" : "其他");

    memset(buf, 0, 25);

    /* page 3 */
    frames[2].item_num = 4;
    frames[2].items[0].pos.row = 1;
    frames[2].items[0].pos.col = 1;
    sprintf(frames[2].items[0].title, "纳税户编号:");

    frames[2].items[0].pos.row = 2;
    frames[2].items[0].pos.col = 2;
    bcd_to_str(gp_app_info->taxpayer_nb, buf, 8);
    sprintf(frames[2].items[0].title, "%s", buf);

    frames[2].items[1].pos.row = 3;
    frames[2].items[1].pos.col = 1;
    end_cover_int((uchar *)&office_code, gp_app_info->office_code);
    sprintf(frames[2].items[0].title, "主管分局代码：%09u", office_code);

    frames[2].items[2].pos.row = 4; 
    frames[2].items[2].pos.col = 1;
    memcpy(buf, gp_app_info->taxpayer_id, 20);
    buf[20] = 0;
    sprintf(frames[2].items[0].title, "税号：%s", gp_app_info->taxpayer_id);

    /* page 4 */
    frames[3].item_num = 4;
    frames[3].items[0].pos.row = 1;
    frames[3].items[0].pos.col = 1;
    sprintf(frames[3].items[0].title, "单张发票限额: %.2f", gp_app_info->single_invoice_limit/100.0);

    frames[3].items[1].pos.row = 2;
    frames[3].items[1].pos.col = 1;
    sprintf(frames[3].items[1].title, "开票累计限额: %.2f", gp_app_info->total_invoice_limit/100.0);

    frames[3].items[2].pos.row = 3;
    frames[3].items[2].pos.col = 1;
    sprintf(frames[3].items[2].title, "退票累计额限: %.2f", gp_app_info->return_invoice_limit/100.0);

    frames[3].items[3].pos.row = 4;
    frames[3].items[3].pos.col = 1;
    if ((gp_app_info->detail_mode >> 4) == 3) {
        sprintf(frames[3].items[3].title, "申报明细和日交易");
    } else if ((gp_app_info->detail_mode >> 4) == 2) {
        sprintf(frames[3].items[3].title, "不申报明细，申报日交易");
    } else if ((gp_app_info->detail_mode >> 4) == 1) {
        sprintf(frames[3].items[3].title, "申报明细，不申报日交易");
    } else if ((gp_app_info->detail_mode >> 4) == 0) {
        sprintf(frames[3].items[3].title, "不申报明细和日交易");
    }

    show_simple_frame(&frames[0]);

    set_ime_status(INPUT_LOW_CASE);
    clear_cache(); 

    int i = 0;
    while (1) { 
        key = get_keycode();

        switch (key) {
            case ENTER:
                return SUCCESS;

            case BACK:
                return SUCCESS;
                break;

            case ESC:
                return SUCCESS;
                break;

            case UP:
                if (i > 0)
                    show_simple_frame(&frames[--i]);
                break;

            case DOWN:
                if (i < 3)
                    show_simple_frame(&frames[++i]);
                break;

            default:
                break;
        } 
    }    

    /* * no way to get here */
    return FAIL;
}

/*
 * cmd_view_declare_info - service for sub-menu "系统信息-申报信息"
 *  @returnm : status
 */
int cmd_view_declare_info(void)
{
    int ret; 
    struct tax_sys_fis_config_record * gp_fis_cfg = get_fis_config();  
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret == NEGATIVE) {
        display_warn("本机尚未初始化！");
        return FAIL;
    }

    if (gp_fis_cfg->declare_flag == 0) {
        display_warn("本申报期尚未税控申报！");
    } else {
        display_warn("已申报，请及时完税并进行税控更新！");
    }

    return SUCCESS;
}

/*
 * cmd_view_mach_info - service for sub-menu "机器信息"
 * @return : status
 */
int cmd_view_mach_info(void)
{
    int key;
    char buf[25] = {0};
    struct simple_frame frames[2];
    struct machine_info_record * gp_mach_info = get_mach_info();

    memset(frames, 0, 2 * sizeof(struct simple_frame));

    /* first page */
    frames[0].item_num = 4;
    frames[0].items[0].pos.row = 1;
    frames[0].items[0].pos.col = 1;
    sprintf(frames[0].items[0].title, "机器编号:");

    frames[0].items[1].pos.row = 2;
    frames[0].items[1].pos.col = 2;
    bcd_to_str(gp_mach_info->machine_nb, buf, 8);
    sprintf(frames[0].items[1].title, "%s", buf);

    memset(buf, 0, 25);

    frames[0].items[2].pos.row = 3;
    frames[0].items[2].pos.col = 1;
    sprintf(frames[0].items[2].title, "生产日期: %s", buf);

    frames[0].items[3].pos.row = 4;
    frames[0].items[3].pos.col = 2;
    bcd_to_str((uchar *)&gp_mach_info->produce_date, buf, 4);
    sprintf(frames[0].items[3].title, "生产日期: %s", buf);

    /* second page */
    frames[1].item_num = 4;
    frames[1].items[0].pos.row = 1;
    frames[1].items[0].pos.col = 1;
    sprintf(frames[1].items[0].title, "硬件版本号:");

    frames[1].items[1].pos.row = 2;
    frames[1].items[1].pos.col = 2;
    sprintf(frames[1].items[1].title, "%s", gp_mach_info->hw_version);

    frames[1].items[0].pos.row = 3;
    frames[1].items[0].pos.col = 1;
    sprintf(frames[1].items[0].title, "软件版本号:");

    frames[1].items[0].pos.row = 4;
    frames[1].items[0].pos.col = 2;
    sprintf(frames[1].items[0].title, "%s", gp_mach_info->sw_version);

    show_simple_frame(&frames[0]);

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();

    int i = 0;
    while (1) {
        key = get_keycode();

        switch (key) {
            case ENTER:
                return SUCCESS;

            case BACK:
                return SUCCESS;
                break;

            case ESC:
                return SUCCESS;
                break;

            case UP:
                if (i > 0)
                    show_simple_frame(&frames[--i]);
                break;

            case DOWN:
                if (i < 1)
                    show_simple_frame(&frames[++i]);
                break;

            default:
                break;
        }
    }

    return FAIL;
}

/*
 * cmd_view_issue_info - service for sub-menu "开票信息"
 *  @return : status
 */
int cmd_view_issue_info(void)
{
    int ret, key;
    struct simple_frame frames[2];
    struct tax_sys_amount_record amt_rec;
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();
    struct tax_system *tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret == NEGATIVE) {
        display_warn("本机尚未初始化！");
        return FAIL;
    } else if (ret < 0) {
        display_warn("未知错误！");
        display_err_msg(ret);
        return FAIL;
    }

    display_info("正在获取本申报期开票信息...");

    ret = tax_file_read_amount(&amt_rec);
    if (ret < 0) {
        display_warn("获取申报信息失败！");
        display_err_msg(ret);
        return FAIL;
    }

    /* first page */ 
    frames[0].item_num = 4;
    frames[0].items[0].pos.row = 1;
    frames[0].items[0].pos.col = 3;
    sprintf(frames[0].items[0].title, "本申报期开票信息");

    frames[0].items[1].pos.row = 2;
    frames[0].items[1].pos.col = 1;
    sprintf(frames[0].items[1].title, "开票累计金额: %.2f", amt_rec.amt_total_this/100.0);

    frames[0].items[2].pos.row = 3;
    frames[0].items[2].pos.col = 1;
    sprintf(frames[0].items[2].title, "退票累计金额: %.2f", amt_rec.amt_return_this/100.0);

    /* second page */
    frames[1].item_num = 4;
    frames[1].items[0].pos.row = 1;
    frames[1].items[0].pos.col = 3;
    sprintf(frames[1].items[0].title, "本申报期开票信息");

    frames[1].items[1].pos.row = 2;
    frames[1].items[1].pos.col = 1;
    sprintf(frames[1].items[1].title, "剩余开票金额: %.2f", 
            (gp_app_info->total_invoice_limit - amt_rec.amt_total_this)/100.0);

    frames[1].items[2].pos.row = 3;
    frames[1].items[2].pos.col = 1;
    sprintf(frames[1].items[2].title, "剩余退票金额: %.2f", 
            (gp_app_info->return_invoice_limit - amt_rec.amt_return_this)/100.0);

    show_simple_frame(&frames[0]);

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();

    int i = 0;
    while (1) {
        key = get_keycode();
        switch (key) {
            case ENTER:
                return SUCCESS;

            case BACK:
                return SUCCESS;
                break;

            case ESC:
                return SUCCESS;
                break;

            case UP:
                if (i > 0)
                    show_simple_frame(&frames[--i]);
                break;

            case DOWN:
                if (i < 1)
                    show_simple_frame(&frames[++i]);
                break;

            default:
                break;
        }
    }

    return FAIL;
}

/*
 * cmd_view_taxrate_info - service for sub-menu "税率信息"
 *  @return : status
 */
int cmd_view_taxrate_info(void)
{
    int key, num, pos, page;
    char buf[20] = {0};

    struct simple_frame frames[2];
    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    memset(frames, 0, sizeof(struct simple_frame) * 2);

    /* first page */
    frames[0].item_num = 4;
    frames[0].items[0].pos.row = 1;
    frames[0].items[0].pos.col = 5;

    frames[0].items[1].pos.row = 2;
    frames[0].items[1].pos.col = 1;

    frames[0].items[2].pos.row = 3;
    frames[0].items[2].pos.col = 1;

    frames[0].items[3].pos.row = 4;
    frames[0].items[3].pos.col = 1;

    /* second page */
    frames[1].item_num = 3;
    frames[1].items[0].pos.row = 1;
    frames[1].items[0].pos.col = 5;

    frames[1].items[1].pos.row = 2;
    frames[1].items[1].pos.col = 1;

    frames[1].items[2].pos.row = 3;
    frames[1].items[2].pos.col = 1;

    set_ime_status(INPUT_LOW_CASE);
    clear_cache();

    num = pos = page = 0;
    while (1) {
        /* find a vaild item */
        for (num = 0; num < gp_app_info->fis_type_num; num++) {
            if (gp_app_info->fis_type[num].index == gp_app_info->tax_index[pos])
                break;
        }

        sprintf(frames[0].items[0].title, "税种税目(%d)", pos + 1);
        sprintf(frames[0].items[1].title, "%s%d", "税种税目引索号", gp_app_info->tax_index[pos]);
        sprintf(frames[1].items[0].title, "税种税目(%d)", pos + 1);

        /* not found */
        if (num == gp_app_info->fis_type_num) {
            sprintf(frames[0].items[2].title, "经营项目信息：未找到");

            memset(frames[0].items[3].title, 0, MAX_TITLE_LEN);
            memset(frames[1].items[0].title, 0, MAX_TITLE_LEN);
            memset(frames[1].items[1].title, 0, MAX_TITLE_LEN);
            memset(frames[1].items[2].title, 0, MAX_TITLE_LEN);
        } else {
            bcd_to_str(gp_app_info->fis_type[num].item_code, buf, 4);
            sprintf(frames[0].items[2].title, "经营项目代码: %s", buf);
            sprintf(frames[0].items[3].title, "%s%.2f%s", "税率：", 
                    gp_app_info->fis_type[num].tax_rate/100.0, "%");

            sprintf(frames[1].items[1].title, "%s%s", "中文名字:", 
                    strlen(gp_app_info->fis_type[num].item_cn_name) > 0 ? gp_app_info->fis_type[num].item_en_name : "无");
            sprintf(frames[1].items[2].title, "%s%s", "英文名字:", 
                    strlen(gp_app_info->fis_type[num].item_cn_name) > 0 ? gp_app_info->fis_type[num].item_en_name : "无");
        }

show_tax:
        show_simple_frame(&frames[page]);
        key = get_keycode();
        switch (key) {
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
                if (page > 0) 
                    page --;
                goto show_tax;

                break;

            case DOWN:
                if (page < 1)
                    page ++;
                goto show_tax;

                break;

            case RIGHT:
                if (pos < gp_app_info->fis_type_num - 1)
                    pos ++;
                else 
                    goto show_tax;

                break;

            case LEFT:
                if (pos > 0)
                    pos --;
                else 
                    goto show_tax;

                break;

            default:
                break;
        }
    }

    /* Never to get here */
    return FAIL;
}


/*
 * cmd_view_print_info - service for sub-menu "打印信息"
 *  @return : status 
 */
int cmd_view_print_info(void)
{
    struct print_sys * print_sys = get_print_sys();

    show_print_info(print_sys->print_type);
    
    get_anykey();
    return SUCCESS;
}


/*
 * cmd_sw_update - service for sub-menu "软件更新"
 *  @return : status 
 */
int cmd_sw_update_by_uart(void)
{
    /*
     * obsoleted API
     */
    display_warn("该功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_sw_update - service for sub-menu "软件更新"
 *  @return : status 
 */
int cmd_sw_update_by_network(void)
{
    /*
     * obsoleted API
     */
    display_warn("该功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_sw_update - service for sub-menu "软件更新"
 *  @return : status 
 */
int cmd_sw_update_by_usb(void)
{
    /*
     * obsoleted API
     */
    display_warn("该功能尚未完善！");

    return SUCCESS;
}

/*
 * cmd_sw_update - service for sub-menu "软件更新"
 *  @return : status 
 */
int cmd_sw_update_set_keycode(void)
{
    /*
     * obsoleted API
     */
    display_warn("该功能尚未完善！");

    return SUCCESS;
}


int cmd_system_setup(void)
{
    /*
     * no need, obsoleted API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}


/*
 * cmd_view_date_time - sevice for sub-menu "查看时间"
 *  @return : status
 */
int cmd_view_date_time(void)
{
    int ret;
    uint y, m, d; 
    struct greg_time now;
    struct bcd_date today;
    struct rt_operate *rt_ops = get_rt_ops();
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));
    memset(&now, 0, sizeof(now));
    memset(&today, 0, sizeof(today));

    ret = rt_ops->get_cur_date(&today);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    bcd_to_greg(&today, &y, &m, &d);

    ret = rt_ops->get_cur_time(&now);
    if (ret != SUCCESS) {
        display_err_msg(ret);
        return FAIL;
    }

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "当前日期时间");

    frame.items[1].pos.row = 2; 
    frame.items[1].pos.row = 1;
    sprintf(frame.items[1].title, "当前日期：%d-%02d-%02d", y, m, d);

    frame.items[2].pos.row = 4; 
    frame.items[2].pos.row = 1;
    sprintf(frame.items[2].title, "当前时间:%d:%02d:%02d", now.hour, now.min, now.sec);

    clear_cache();
    get_keycode();

    return SUCCESS;
}





