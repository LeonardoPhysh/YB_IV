/*
 * com_comm_manage.c
 *  - implement function of fiscal manage
 * 
 * Author : Leonardo Physh 
 * Date   : 2014.9.24
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "input.h"
#include "keyboard.h"
#include "plu.h"
#include "command.h"
#include "ui_api.h"
#include "real_time.h"
#include "tax_system.h"

/*
 * UI helper 
 */
static int show_setup_date(struct bcd_date *today)
{
    uint y, m, d;
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));
    bcd_to_greg(today, &y, &m, &d);

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "设置系统日期");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%d-%02d-%02d", "当前日期：", y, m, d);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "输入日期：");

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s", "(日期格式：20140101)");

    show_simple_frame(&frame);

    return SUCCESS;
}

static int show_setup_time(struct greg_time *now)
{
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 4;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "设置系统时间");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s%d:%02d:%02d", "当前时间：", 
            now->hour, now->min, now->sec);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "输入时间：");

    frame.items[3].pos.row = 4;
    frame.items[3].pos.col = 1;
    sprintf(frame.items[3].title, "%s", "(时间格式：141530)");

    show_simple_frame(&frame);

    return SUCCESS;
}

static int show_get_enddate(struct bcd_date *start, struct bcd_date *end)
{
    uint s_y, s_m, s_d;
    uint e_y, e_m, e_d;

    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    bcd_to_greg(start, &s_y, &s_m, &s_d);
    bcd_to_greg(end, &e_y, &e_m, &e_d);

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "%s", "正常税控申报");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "请输入数据截止日期：");

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "范围(%d%02d%02d-%d%02d%02d)", 
            s_y, s_m, s_d, e_y, e_m, e_d);

    show_simple_frame(&frame);
    return SUCCESS;
}

static int show_get_verify_pin(void)
{
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 4;
    sprintf(frame.items[0].title, "%s", "税控稽查");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "请输入认证密码(HEX)：");

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "格式为八个十六进制数");

    show_simple_frame(&frame);
    return SUCCESS;

}

static int show_inspect_UI(char *title)
{
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 6 - strlen(title)/4;
    sprintf(frame.items[0].title, "%s", title);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "%s", "起始日期：");

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "终止日期: ");

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "%s", "日期格式：20150101");

    show_simple_frame(&frame);
    return SUCCESS;
}


static int show_inspect_chk_info(char *title, struct tax_sys_check_idx *chk_idx)
{
    char buf[10] = {0};
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 6 - strlen(title)/4;
    sprintf(frame.items[0].title, "%s", title);

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    bcd_to_str((uchar *)&chk_idx->start_date, buf, 4);
    sprintf(frame.items[1].title, "起始日期: %s", buf);

    frame.items[2].pos.row = 3;
    frame.items[2].pos.col = 1;
    bcd_to_str((uchar *)&chk_idx->end_date, buf, 4);
    sprintf(frame.items[2].title, "终止日期：%s", buf);

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    sprintf(frame.items[2].title, "记录条数：%d", chk_idx->rec_num);

    show_simple_frame(&frame);
    return SUCCESS;
}

/* #open to other modules# */
int do_set_date(void)
{
    int ret;
    uint y, m, d;
    char title[48 + 1] = {0};

    struct bcd_date today, new_date;
    struct rt_operate *rt_ops = get_rt_ops();

    memset(&today, 0, sizeof(today));
    memset(&new_date, 0, sizeof(new_date));

    ret = rt_ops->get_cur_date(&today);
    if (ret != SUCCESS) {
        display_err_msg(ret, "日期设置失败！");
        return FAIL;
    }

    bcd_to_greg(&today, &y, &m, &d);
    snprintf(title, 48, "今天是%d-%02d-%02d, 是否重置日期？", y, m, d);

    ret = question_user(title);
    if (ret == NEGATIVE) {
        return SUCCESS;
    }

    while (1) {
        show_setup_date(&today);
        ret = get_bcd_date(3, 6, &new_date);
        if (ret != SUCCESS) {
            if (ret == -EUI_ESC) {
                ret = question_user("确认取消本次操作？");
                if (ret == POSITIVE)
                    return -EUI_ESC;
                else
                    continue;
            }

            if (ret == -EUI_BAD_DATE_FORMAT) {
                display_info("输入日期格式错误");
                continue;
            }

        } else if (ret == SUCCESS)
            break;
    }

    ret = rt_ops->set_cur_date(&new_date);
    if (ret != SUCCESS) {
        display_err_msg(ret, "日期设置失败！");
        return FAIL;
    }

    display_info("日期设置成功!");
    return SUCCESS;
}

/* #open to other modules# */
int do_set_time(void)
{
    int ret;
    char title[48 + 1] = {0};

    struct greg_time now, new_time;
    struct rt_operate *rt_ops = get_rt_ops();

    memset(&now, 0, sizeof(now));
    memset(&new_time, 0, sizeof(new_time));

    ret = rt_ops->get_cur_time(&now);
    if (ret != SUCCESS) {
        display_err_msg(ret, "时间设置失败！");
        return FAIL;
    }

    snprintf(title, 48, "现在时间是%d:%02d:%02d, 是否重置时间？", 
            now.hour, now.min, now.sec);

    ret = question_user(title);
    if (ret == NEGATIVE) {
        return SUCCESS;
    }

    while (1) {
        show_setup_time(&now);
        ret = get_greg_time(3, 6, &new_time);
        if (ret == -EUI_ESC) {
            ret = question_user("确认取消本次操作？");
            if (ret == POSITIVE)
                return -EUI_ESC;
            else
                continue;
        } else if (ret == SUCCESS)
            break;
    }

    ret = rt_ops->set_cur_time(&new_time);
    if (ret != SUCCESS) {
        display_err_msg(ret, "时间设置失败！");
        return FAIL;
    }

    display_info("时间设置成功!");

    return SUCCESS;
}

static inline int do_check_date(struct bcd_date *s_date, struct bcd_date * e_date)
{
    int ret;
    struct bcd_date today;
    struct rt_operate *rt_ops = get_rt_ops();

    ret = rt_ops->get_cur_date(&today);
    if (ret != SUCCESS)
        return ret;

    if (rt_ops->cmp_bcd_date(s_date, e_date) > 0) {
        display_warn("终止日期必须在起始日期之后！");
        return FAIL;
    }

    if (rt_ops->cmp_bcd_date(e_date, &today) > 0) {
        display_warn("终止日期不能在今日之后！");
        return FAIL;
    }

    return SUCCESS;
}

/*
 * do_inspect_prepare - check current environment is suit for doing 
 *                      inspect or not.
 *  @return : POSITIVE upon OK or NEGATIVE upon fail.
 */
static int do_inspect_prepare(int type, struct tax_sys_check_info *chk_info)
{
    int ret;
    struct bcd_date today;
    uchar pin[8] = {0};
    struct rt_operate * rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    ret = tax_system->get_check_info(chk_info);
    if (ret < 0)
        return ret;

    if (chk_info->type != type) {
        display_warn("稽查卡类型不匹配！");
        return FAIL;
    }

    ret = rt_ops->get_cur_date(&today);
    if (rt_ops->cmp_bcd_date(&today, &chk_info->valid_date) > 0) {
        display_warn("稽查卡已过有效期！");
        return FAIL;
    }

    if (chk_info->level != 0xFF) {
        if (memcmp(chk_info->taxpayer_nb, gp_app_info->taxpayer_nb, 8) != 0) {
            display_warn("非本纳税户稽查卡！");
            return FAIL;
        }

        display_info("认证成功！"); //DO NOT NEED A KEY ENTER, KEEP IT
        return SUCCESS;
    } 

    /* higher level checking... */ 
    ret = question_user("是否输入稽查卡认证密码？");
    if (ret != POSITIVE)
        return SUCCESS;

get_pin:
    show_get_verify_pin();
    ret = get_hex_num(2, 1, pin, 8); //position, buffer, len
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else 
            goto get_pin;
    } 

    ret = tax_system->verify_check_card(pin);
    if (ret != SUCCESS) {
        display_warn("稽查卡认证失败！");
        return ret;
    }

    display_info("认证成功!");
    return SUCCESS;
}

/*
 * do_update_daily_record - do actually job to write record 
 *  @chk_info:
 *  @chk_idx:
 */
static int do_update_daily_record(struct tax_sys_check_info *chk_info, 
        struct tax_sys_check_idx *chk_idx)
{
    int ret;
    int key, cur_offset;
    struct tax_system * tax_system = get_tax_system();

    cur_offset = chk_idx->start_offset;

    int i;
    for (i = 0; i < chk_idx->rec_num; i++) {
        if (chk_info->rec_num >= chk_info->total_num) {
            display_info("稽查卡已满，请换卡后按任意键继续，退出请按取消");
            key = get_anykey();
            if (key == ESC)
                return FAIL;

            while (1) {
                display_info("正在校验新稽查卡...");

                ret = do_inspect_prepare(INSPECT_DAILY_SUM, chk_info);
                if (ret != SUCCESS) {
                    display_info("校验失败，请换卡后按任意键继续，退出请按取消");
                    key = get_anykey();
                    if (key == ESC)
                        return FAIL;
                }

                break;
            }
        }

        clear_screen();
        display_str(1, 1, "正在写入...第%d条，共%d条", i + 1, chk_idx->rec_num);

        /* progress of updating record */
        ret = tax_system->write_check_daily_detail(cur_offset, chk_info->rec_num + 1);
        if (ret != SUCCESS) {
            display_err_msg(ret, "写卡出错！");
            return ret;
        }

        cur_offset ++;
        chk_info->rec_num ++;
    }

    display_warn("写卡完成！");
    return SUCCESS;
}

static int do_update_declare_record(struct tax_sys_check_info *chk_info, 
        struct tax_sys_check_idx *chk_idx) 
{
    int ret;
    int key, cur_offset;
    struct tax_system * tax_system = get_tax_system();

    cur_offset = chk_idx->start_offset;

    int i, record_nb;
    i = 1, record_nb = chk_idx->rec_num;

    while (record_nb > 0) {
        if (chk_info->rec_num >= chk_info->total_num) {
            display_info("稽查卡已满，请换卡后按任意键继续，退出请按取消");
            key = get_anykey();
            if (key == ESC)
                return FAIL;

            while (1) {
                display_info("正在校验新稽查卡...");

                ret = do_inspect_prepare(INSPECT_DAILY_SUM, chk_info);
                if (ret != SUCCESS) {
                    display_info("校验失败，请换卡后按任意键继续，退出请按取消");
                    key = get_anykey();
                    if (key == ESC)
                        return FAIL;
                }

                break;
            }
        }

        clear_screen();
        display_str(1, 1, "正在写入...第%d条，共%d条", i, chk_idx->rec_num);

        /* progress of updating record */
        if (record_nb >= 2) {
            ret = tax_system->write_check_declare_detail(2, cur_offset, 
                    chk_info->rec_num + 1);
            i += 2;
            cur_offset += 2;
            record_nb -= 2;
        } else {
            ret = tax_system->write_check_declare_detail(1, cur_offset, 
                    chk_info->rec_num + 1);
            i++;
            cur_offset++;
            record_nb--;
        }
        chk_info->rec_num ++;
        
        if (ret != SUCCESS) {
            display_err_msg(ret, "写卡出错！");
            return ret;
        }
    }

    display_warn("写卡完成！");
    return SUCCESS;
}

static int do_update_invoice_record(struct tax_sys_check_info *chk_info, 
        struct tax_sys_check_idx *chk_idx) 
{
    int ret, key;
    int index, offset, cur_offset;
    struct tax_system * tax_system = get_tax_system();

    index = ((chk_idx->start_offset) >> 16) & 0xFFFF;
    offset = (chk_idx->start_offset) & 0xFFFF;

    int i, record_nb;
    i = 1, record_nb = chk_idx->rec_num;
    
    ret = 0; //just for clearing warn
    while (record_nb > 0) {
        if (chk_info->rec_num >= chk_info->total_num) {
            display_info("稽查卡已满，请换卡后按任意键继续，退出请按取消");
            key = get_anykey();
            if (key == ESC)
                return FAIL;

            while (1) {
                display_info("正在校验新稽查卡...");

                ret = do_inspect_prepare(INSPECT_DAILY_SUM, chk_info);
                if (ret != SUCCESS) {
                    display_info("校验失败，请换卡后按任意键继续，退出请按取消");
                    key = get_anykey();
                    if (key == ESC)
                        return FAIL;
                }

                break;
            }
        }

        clear_screen();
        display_str(1, 1, "正在写入...第%d条，共%d条", i, chk_idx->rec_num);
        
        cur_offset = ((index & 0xffff) << 16) | (offset & 0xffff);

        /* progress of updating record */
        if (record_nb >= 7) {
            ret = tax_system->write_check_invoice_detail(7, cur_offset, 
                    chk_info->rec_num + 1);
            
            i += 7;
            record_nb -= 7;
            if (offset + 7 > INVOICE_DETAIL_REC_NUM) {
                offset = offset + 7 - INVOICE_DETAIL_REC_NUM;
                if (index == 10)
                    index = 1;
                else 
                    index ++;
            }
        } else {
            ret = tax_system->write_check_invoice_detail(record_nb, cur_offset, 
                    chk_info->rec_num + 1);
            record_nb = 0;
        }
        chk_info->rec_num ++;
        
        if (ret != SUCCESS) {
            display_err_msg(ret, "写卡出错！");
            return ret;
        }
    }

    if (ret != SUCCESS)
        display_warn("写卡出错！");
    else 
        display_warn("写卡完成！");
    return ret;
}


/*
 * cmd_normal_declare_duty - service for sub-menu "正常申报"
 *  @return : status 
 */
int cmd_normal_declare_duty(void)
{
    int ret;
    struct bcd_date today;
    struct bcd_date last_declare_date, declare_limit_date, declare_end_date;
    struct rt_operate *rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_app_info * g_app_info = get_sys_app_info();

    memset(&today, 0, sizeof(today));
    memset(&last_declare_date, 0, sizeof(last_declare_date));
    memset(&declare_limit_date, 0, sizeof(declare_limit_date));
    memset(&declare_end_date, 0, sizeof(declare_end_date));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，无法进行此操作！");
        return FAIL;
    }

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_info("权限限制，无法进行此操作！");
        return FAIL;
    }

    display_info("正在准备税控申报...");

    ret = rt_ops->get_cur_date(&today);
    if (ret != SUCCESS) 
        goto fail;    

    /* declare date limit to the day before today */
    ret = rt_ops->get_prev_date(&today);
    if (ret != SUCCESS)
        goto fail;

    ret = tax_system->get_last_declare_date(&last_declare_date);
    if (ret != SUCCESS){ 
        if (ret == -EFILE_NO_REC) {    
            last_declare_date = g_app_info->init_date;
        } else 
            goto fail;
    }

    ret = rt_ops->cmp_bcd_date(&today, &last_declare_date);
    if (ret == 0) {
        display_warn("本机暂无可申报数据, 明日再进行申报！");
        return FAIL;
    }

    declare_limit_date = today;
    ret = tax_system->check_declare_date(&declare_limit_date);
    if (ret != SUCCESS)
        goto fail;

    ret = rt_ops->cmp_bcd_date(&declare_limit_date, &last_declare_date);
    if (ret <= 0) {
        display_warn("本机暂无可申报数据, 明日再进行申报！");
        return FAIL;
    }

    while (1) {
        show_get_enddate(&last_declare_date, &declare_limit_date);
        ret = get_bcd_date(3, 1, &declare_end_date);
        if (ret != SUCCESS) {
            if (ret == -EUI_ESC) {
                ret = question_user("是否取消本次申报？");
                if (ret == POSITIVE)
                    return FAIL;
                else 
                    continue;
            }

            if (ret == -EUI_BAD_DATE_FORMAT) {
                display_warn("输入日期格式错误！");
                continue;
            }

        } else if (ret == SUCCESS) {
            if ((rt_ops->cmp_bcd_date(&declare_end_date, &last_declare_date) > 0)
                    && (rt_ops->cmp_bcd_date(&declare_end_date, &declare_limit_date) <= 0)){
                break;
            } else {
                display_warn("输入的截止日期不在可申报范围之内！");
                continue;
            }
        }
    }

    display_info("正在申报...");

    ret = tax_system->declare_duty(NORMAL_DECLARE, &declare_end_date);
    if (ret != SUCCESS) {
        if (ret == -ETAX_NODATA_TO_DECLARE) {
            display_warn("本机暂无可申报数据！");
            return FAIL;
        } else 
            goto fail;
    }

#ifdef CONFIG_DETAIL_DECLARE
    display_info("明细申报尚未完善！");
#endif  

    display_warn("申报完毕，请及时完成报税并进行税控更新！");
    return SUCCESS;

fail:
    display_err_msg(ret, "税控申报失败！");
    return FAIL;
}

/*
 * cmd_month_declare_duty - service for sub-menu "自然月申报"
 *  @return : status 
 */
int cmd_month_declare_duty(void)
{
    int ret;
    struct bcd_date today;
    struct bcd_date last_declare_date, declare_limit_date, declare_end_date;

    struct rt_operate *rt_ops = get_rt_ops();
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_app_info * g_app_info = get_sys_app_info();

    memset(&today, 0, sizeof(today));
    memset(&last_declare_date, 0, sizeof(last_declare_date));
    memset(&declare_limit_date, 0, sizeof(declare_limit_date));
    memset(&declare_end_date, 0, sizeof(declare_end_date));

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，无法进行此操作！");
        return FAIL;
    }

    ret = check_userlevel(MANAGER_USER);
    if (ret != POSITIVE) {
        display_warn("权限限制，无法进行此操作！");
        return FAIL;
    }

    display_info("正在准备税控申报...");

    ret = rt_ops->get_cur_date(&today);
    if (ret != SUCCESS) 
        goto fail;    

    ret = rt_ops->get_prev_date(&today);
    if (ret != SUCCESS)
        goto fail;

    ret = tax_system->get_last_declare_date(&last_declare_date);
    if (ret != SUCCESS){ 
        if (ret == -EFILE_NO_REC) {    
            last_declare_date = g_app_info->init_date;
        } else 
            goto fail;
    }

    ret = rt_ops->cmp_bcd_date(&today, &last_declare_date);
    if (ret == 0) {
        display_warn("本月暂无可申报数据, 下月再进行申报！");
        return FAIL;
    }

    display_info("正在申报...");

    ret = tax_system->declare_duty(MONTH_DECLARE, NULL);
    if (ret != SUCCESS) {
        if (ret == -ETAX_NODATA_TO_DECLARE) {
            display_warn("本月暂无可申报数据, 下月再进行申报！");
            return FAIL;
        } else 
            goto fail;
    }

#ifdef CONFIG_DETAIL_DECLARE
    display_info("明细申报尚未完善！");
#endif  

    display_info("申报完毕，请及时完成报税并进行税控更新！");

    return SUCCESS;

fail:
    display_err_msg(ret, "税控申报失败！");
    return FAIL;
}

/*
 * cmd_declare_preview - service for sub-menu "申报预览"
 *  @return : status 
 */
int cmd_declare_preview(void)
{
    /*
     * obsoled API
     */
    display_warn("功能尚未完善！");

    return SUCCESS;
}

/* __ATTENTION__: set date need to verify_check_card 
 * so we move the callback function here.
 */
/*
 * cmd_set_date_time - service for sub-menu "设置时钟"
 *  @return : status
 */
int cmd_set_date_time(void)
{
    int ret;    
    struct tax_system * tax_system = get_tax_system(); 
    struct tax_sys_check_info chk_info;

    memset(&chk_info, 0, sizeof(chk_info)); 
    ret = check_userlevel(DEVELOP_USER);
    if (ret == POSITIVE)
        goto set_date;

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) 
        goto set_date;
    else {
        display_info("正在校验稽查卡...");
        ret = do_inspect_prepare(INSPECT_MOD_TIME, &chk_info);
        if (ret != SUCCESS) {
            display_warn("稽查卡校验失败！");
            return ret;
        }

        goto set_date; 
    }

set_date:
    ret = do_set_date();
    if (ret != SUCCESS)
        return ret;

    ret = do_set_time();
    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

/*
 * cmd_inspect_inv_by_date - service for sub-menu "稽查交易详情-按日期"
 *  @return : status
 */
int cmd_inspect_inv_by_date(void)
{
    int ret;
    struct tax_sys_check_idx chk_idx;
    struct tax_sys_check_info chk_info;

    memset(&chk_idx, 0, sizeof(chk_idx));
    memset(&chk_info, 0, sizeof(chk_info));

    display_info("正在校验稽查卡...");

    ret = do_inspect_prepare(INSPECT_INV_DETAIL, &chk_info);
    if (ret != SUCCESS) 
        return ret;

get_date:
    show_inspect_UI("核查发票明细数据");
    ret = get_bcd_date(2, 6, &chk_idx.start_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            goto get_date;
        }
    }

    ret = get_bcd_date(3, 6, &chk_idx.end_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            memset(&chk_idx.end_date, 0, sizeof(chk_idx.end_date));
            goto get_date;
        }
    }

    /* check date format */
    ret = do_check_date(&chk_idx.start_date, &chk_idx.end_date);
    if (ret != SUCCESS)
        goto get_date;

    display_info("正在查找发票明细数据...");

    ret = tax_file_find_chk_invoice_idx(&chk_idx);
    if (ret != SUCCESS) {
        if (ret == -EFILE_NO_REC) {
            display_warn("无找到记录，请确认日期输入正确！");
            goto get_date;
        }
    } else {
        display_err_msg(ret, "查找发票明细出错！");
        return ret;
    }

    show_inspect_chk_info("发票明细查找结果", &chk_idx);
    get_anykey();

    ret = question_user("是否将记录写入稽查卡？");
    if (ret != POSITIVE)
        return SUCCESS;

    /* update card record...*/
    ret = do_update_invoice_record(&chk_info, &chk_idx);
    return ret;
}

/*
 * cmd_inspect_inv_by_nb - service for sub-menu "稽查交易详情-按日期"
 *  @return : status 
 */
int cmd_inspect_inv_by_nb(void)
{
    int ret;
    int inv_nb = 0;
    struct tax_sys_check_idx chk_idx;
    struct tax_sys_check_info chk_info;

    struct simple_frame frame;
    struct tax_sys_invoice_detail_record inv_detail;

    memset(&chk_idx, 0, sizeof(chk_idx));
    memset(&chk_info, 0, sizeof(chk_info));
    memset(&frame, 0, sizeof(frame));

    display_info("正在校验稽查卡...");

    ret = do_inspect_prepare(INSPECT_INV_DETAIL, &chk_info);
    if (ret != SUCCESS) 
        return ret;

    frame.item_num = 2;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    sprintf(frame.items[0].title, "核查发票明细数据");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "请输入发票号：");

get_inv_nb:
    show_simple_frame(&frame);
    ret = get_inter_num(3, 1, &inv_nb);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;

        goto get_inv_nb;        
    }

    ret = tax_file_find_invoice_detail((uint *)&inv_nb, &inv_detail);
    if (ret != SUCCESS || ret != -ETAX_INV_HAS_RETURNED) {
        display_err_msg(ret, "查找发票明细出错！");
        return FAIL;
    }

    chk_idx.rec_num = 1;
    chk_idx.start_offset = inv_nb; //inv_nb has been changed to offset,
                                   //before find a better way, leave item_num
                                   //like that.
    ret = do_update_invoice_record(&chk_info, &chk_idx);
    return ret;
}

/*
 * cmd_inspect_daily_collect - 稽查日交易
 *  @return : status 
 */
int cmd_inspect_daily_collect(void)
{
    int ret;
    struct tax_sys_check_idx chk_idx;
    struct tax_sys_check_info chk_info;
    struct tax_system * tax_system = get_tax_system();

    memset(&chk_idx, 0, sizeof(chk_idx));
    memset(&chk_info, 0, sizeof(chk_info));

    display_info("正在校验稽查卡...");

    ret = do_inspect_prepare(INSPECT_DAILY_SUM, &chk_info);
    if (ret != SUCCESS) 
        return ret;

get_date:
    show_inspect_UI("稽查日汇总");
    ret = get_bcd_date(2, 6, &chk_idx.start_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            goto get_date;
        }
    }

    ret = get_bcd_date(3, 6, &chk_idx.end_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            memset(&chk_idx.end_date, 0, sizeof(chk_idx.end_date));
            goto get_date;
        }
    }

    /* check date format */
    ret = do_check_date(&chk_idx.start_date, &chk_idx.end_date);
    if (ret != SUCCESS)
        goto get_date;

    display_info("正在进行日汇总...");

    ret = tax_system->daily_collect();
    if (ret == SUCCESS) {
        display_err_msg(ret, "日汇总出错！");
        return ret;
    }

    display_info("正在查找日汇总记录...");

    ret = tax_file_find_chk_daily_idx(&chk_idx);
    if (ret != SUCCESS) {
        if (ret == -EFILE_NO_REC) {
            display_warn("无找到记录，请确认日期输入正确！");
            goto get_date;
        }
    } else {
        display_err_msg(ret, "查找日汇总记录出错！");
        return ret;
    }

    show_inspect_chk_info("日汇总查找结果", &chk_idx);
    get_anykey();

    ret = question_user("是否将记录写入稽查卡？");
    if (ret != POSITIVE)
        return SUCCESS;

    /* update card record...*/
    ret = do_update_daily_record(&chk_info, &chk_idx);
    return ret;
}

/*
 * cmd_inspect_declare_data - service for sub-menu "稽查申报数据"
 *  @return: status 
 */
int cmd_inspect_declare_data(void)
{
    int ret;
    struct tax_sys_check_idx chk_idx;
    struct tax_sys_check_info chk_info;

    memset(&chk_idx, 0, sizeof(chk_idx));
    memset(&chk_info, 0, sizeof(chk_info));

    display_info("正在校验稽查卡...");

    ret = do_inspect_prepare(INSPECT_DECLARE, &chk_info);
    if (ret != SUCCESS) 
        return ret;

get_date:
    show_inspect_UI("稽查申报数据");
    ret = get_bcd_date(2, 6, &chk_idx.start_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            goto get_date;
        }
    }

    ret = get_bcd_date(3, 6, &chk_idx.end_date);
    if (ret == -EUI_ESC) {
        ret = question_user("是否取消当前操作！");
        if (ret == POSITIVE)
            return FAIL;
        else {
            memset(&chk_idx.start_date, 0, sizeof(chk_idx.start_date));
            memset(&chk_idx.end_date, 0, sizeof(chk_idx.end_date));
            goto get_date;
        }
    }

    /* check date format */
    ret = do_check_date(&chk_idx.start_date, &chk_idx.end_date);
    if (ret != SUCCESS)
        goto get_date;

    display_info("正在查找申报记录...");

    ret = tax_file_find_chk_declare_idx(&chk_idx);
    if (ret != SUCCESS) {
        if (ret == -EFILE_NO_REC) {
            display_warn("无找到记录，请确认日期输入正确！");
            goto get_date;
        }
    } else {
        display_err_msg(ret, "查找申报记录出错！");
        return ret;
    }

    show_inspect_chk_info("申报数据查找结果", &chk_idx);
    get_anykey();

    ret = question_user("是否将记录写入稽查卡？");
    if (ret != POSITIVE)
        return SUCCESS;

    /* update card record...*/
    ret = do_update_declare_record(&chk_info, &chk_idx);
    return ret;
}

/*
 * cmd_inspect_by_uart - service for sub-menu "串口稽查"
 *  @status 
 */
int cmd_inspect_by_uart(void)
{
    /*
     * obsoleted API 
     */
    display_warn("功能尚未完善！");

    return SUCCESS;

}

/*
 * cmd_update_control - service for sub-menu "税控更新"
 *  @retrun : status 
 */
int cmd_update_control(void)
{
    int ret;
    struct tax_system * tax_system = get_tax_system();
    struct tax_sys_fis_config_record *gp_fis_cfg = get_fis_config();    
    struct tax_sys_app_info *gp_app_info = get_sys_app_info();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，无法进行更新！");
        return FAIL;
    }

    /*
     * current userlevel expect to MANAGER_USER or higher 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret == NEGATIVE) {
        display_warn("权限限制，无法进行此操作！");
        return FAIL;
    }

    if (gp_fis_cfg->declare_flag == 0) {
        display_warn("尚未申报，无需更新！");
        return FAIL;
    }

    ret = question_user("确认进行税控更新？");
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在进行更新...");

    /* whether user card hasn't finished tax operation */
    ret = tax_system->check_finish_tax(gp_app_info->fiscal_card_nb);
    if (ret == NEGATIVE) {
        display_warn("该卡已经申报完毕，请至税务局完税后再尝试更新！");
        return FAIL;
    }

    ret = tax_system->update_control();
    if (ret != SUCCESS) {
        display_err_msg(ret, "更新失败，请联系厂商！");
        return FAIL;
    }

    display_warn("更新成功！");
    return SUCCESS;
}

/*
 * 税控初始化 - service for sub-menu "税控初始化"
 *  @return : status 
 */
int cmd_fiscal_init(void)
{
    int ret;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    ret = tax_system->is_fiscal_init();
    if (ret == POSITIVE) {
        display_warn("已经初始化完毕，无需再初始化！");
        return FAIL;
    }

    /*
     * current userlevel expect to MANAGER_USER or higher 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret == NEGATIVE) {
        display_warn("权限限制,无法进行此操作！");
        return FAIL;
    }

    display_info("正在准备税控初始化...");

    /*
     * setup date and time 
     */
    ret = cmd_set_date_time();
    if (ret != SUCCESS) {
        if (ret == -EUI_ESC)
            return SUCCESS;

        display_err_msg(ret, "税控初始化失败！");
        return FAIL;
    }

    display_info("正在进行税控初始化...");

    ret = tax_system->fiscal_init();
    if (ret != SUCCESS) {
        display_err_msg(ret, "税控初始化失败！");
        return FAIL;

    } else {
        display_info("正在重置税控机...");

        /*
         * no need to reboot system here, call modules' power 
         * on check functions  
         */
        ret = tax_system->power_on_check();
        if (ret != SUCCESS) {
            display_err_msg(ret, "重置税控机失败！");
            return FAIL;
        }

        ret = plu_ops->plu_init();
        if (ret != SUCCESS) {
            display_err_msg(ret, "重置税控机失败！");
            return FAIL;
        }

        display_warn("税控初始化成功!!");
        return SUCCESS;
    }
}

/*
 * cmd_mach_transfer - service for sub-menu "停机过户"
 *  @return : statu 
 */
int cmd_mach_transfer(void)
{
    int ret;
    struct tax_system * tax_system = get_tax_system();
    struct plu_operate * plu_ops = get_plu_ops();

    /*
     * current userlevel expect to MANAGER_USER or higher 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret == NEGATIVE) {
        display_warn("权限限制，无法进行此操作！");
        return FAIL;
    }

    display_info("正在准备停机过户，请插入新税控卡及用户卡...");

    ret = question_user("本操作将清除本机数据，是否继续？");
    if (ret != POSITIVE)
        return FAIL;

    display_info("正在清除本机税控数据...");
    sync();

    ret = tax_file_term_clear();
    if (ret != SUCCESS) {
        display_err_msg(ret, "停机过户失败！");
        return FAIL;
    }

    display_info("清除数据成功，正在重新税控初始化...");

    ret = tax_system->fiscal_init();
    if (ret != SUCCESS) {
        display_err_msg(ret, "税控初始化失败！");
        return FAIL;

    } else {
        display_info("正在重置税控机...");

        /*
         * no need to reboot system here, call modules' power 
         * on check functions  
         */
        ret = tax_system->power_on_check();
        if (ret != SUCCESS) {
            display_err_msg(ret, "重置税控机失败！");
            return FAIL;
        }

        ret = plu_ops->plu_init();
        if (ret != SUCCESS) {
            display_err_msg(ret, "重置税控机失败！");
            return FAIL;
        }

        display_warn("停机过户成功!!");
        return SUCCESS;
    }
}

/*
 * cmd_update_taxpayer - service for sub-menu "更新纳税户"
 *  @return ： status 
 */
int cmd_update_taxpayer(void)
{
    int ret;
    struct tax_system * tax_system = get_tax_system();

    ret = tax_system->is_fiscal_init();
    if (ret != POSITIVE) {
        display_warn("本机尚未税控初始化，无法进行更新！");
        return FAIL;
    }

    /*
     * current userlevel expect to MANAGER_USER or higher 
     */
    ret = check_userlevel(MANAGER_USER);
    if (ret == NEGATIVE) {
        display_warn("权限限制，无法进行此操作！");
        return FAIL;
    }

    ret = question_user("确认更新纳税人名称？");
    if (ret != POSITIVE)
        return FAIL;

    ret = tax_system->update_taxpayer();
    if (ret != SUCCESS) {
        display_err_msg(ret, "更新失败，请联系厂商！");
        return FAIL;
    }

    display_warn("更新纳税人成功！");
    return SUCCESS;
}

