/*
 * UI Common head file 
 * 		defined some comon data structure of UI. 
 *
 * Author: Leoanrdo Physh leonardo.yu@yeah.com
 * Data : 2014.6.28
 *
 * Aningsk change this file 2014-10-10
 */

#ifndef _UI_MENUS_
#define _UI_MENUS_

#include <stdio.h>
#include "command.h"
#include "ui_api.h"

/*
 * 主界面
 */
#if 0
struct frame main_menu = {
    .id = 0,
    .item_num = 7,

    .items = {
        /* id title         color       pos     next */ 
        {0, "请选择操作", COLOR_TINT, {1, 3}, {0, NULL}},
        {1, "1-开票操作", COLOR_TINT, {2, 1}, {1, NULL}},
        {2, "2-商品管理", COLOR_TINT, {3, 1}, {2, NULL}},
        {3, "3-发票管理", COLOR_TINT, {4, 1}, {3, NULL}},
        {4, "4-税控管理", COLOR_TINT, {2, 6}, {4, NULL}},
        {5, "5-系统管理", COLOR_TINT, {3, 6}, {5, NULL}},
        {6, "6-其   他", COLOR_TINT, {4, 6}, {6, NULL}},
    },
};

/*
 * 开票操作1
 */
struct frame inv_ops_menu = {
    .id = 1,
    .item_num = 7,

    .items = {
        {0, "开票操作",     COLOR_TINT, {1, 4}, {1, NULL}},
        {1, "1-商品销售",   COLOR_TINT, {2, 1}, {11, NULL}},
        {2, "2-手工开票",   COLOR_TINT, {3, 1}, {1, cmd_man_issue_inv}},
        {3, "3-退票处理",   COLOR_TINT, {4, 1}, {1, cmd_return_inv}},
        {4, "4-作废当前票", COLOR_TINT, {2, 6}, {1, cmd_spoil_cur_inv}},
        {5, "5-作废剩余票", COLOR_TINT, {3, 6}, {1, cmd_spoil_left_inv}},
        {6, "6-区间废票",   COLOR_TINT, {4, 6}, {1, cmd_spoil_area_inv}},
    },
};

/*
 * 商品销售11
 */
struct frame comm_sell_menu = {
    .id = 11,
    .item_num = 3,

    .items = {
        {0, "商品销售",     COLOR_TINT, {1, 4}, {11, NULL}},
        {1, "1-按商品编号", COLOR_TINT, {2, 3}, {1, cmd_transact_by_num}},
        {2, "2-条形码",     COLOR_TINT, {3, 3}, {1, cmd_transact_by_barcode}},
    },
};

/*
 * 商品管理2
 */
struct frame comm_manage_menu = {
    .id = 2,
    .item_num = 7,

    .items = {
        {0, "商品管理",     COLOR_TINT, {1, 4}, {2, NULL}},
        {1, "1-部类管理",   COLOR_TINT, {2, 1}, {21, NULL}},
        {2, "2-添加商品",   COLOR_TINT, {3, 1}, {2, cmd_add_plu}},
        {3, "3-删除商品",   COLOR_TINT, {4, 1}, {2, cmd_del_plu}},
        {4, "4-修改商品",   COLOR_TINT, {2, 6}, {2, cmd_modify_plu}},
        {5, "5-查询商品",   COLOR_TINT, {3, 6}, {22, NULL}},
        {6, "6-进货管理",   COLOR_TINT, {4, 6}, {2, cmd_stock_manage}},
    },
};

/*
 * 部类管理21
 */
struct frame dpt_manage_menu = {
    .id = 21,
    .item_num = 5,

    .items = {
        {0, "部类管理",     COLOR_TINT, {1, 4}, {21, NULL}},
        {1, "1-添加部类",   COLOR_TINT, {2, 1}, {2, cmd_add_dpt}},
        {2, "2-删除部类",   COLOR_TINT, {3, 1}, {2, cmd_del_dpt}},
        {3, "3-修改部类",   COLOR_TINT, {2, 6}, {2, cmd_modify_dpt}},
        {4, "4-查看部类",   COLOR_TINT, {3, 6}, {2, cmd_view_dpt}},
    },
};

/*
 * 查询商品22
 */
struct frame view_plu_menu = {
    .id = 22,
    .item_num = 4,

    .items = {
        {0, "查询商品",   COLOR_TINT, {1, 4}, {22, NULL}},
        {1, "1-编  号",   COLOR_TINT, {2, 4}, {2, cmd_view_plu_by_num}},
        {2, "2-条形码",   COLOR_TINT, {3, 4}, {2, cmd_view_plu_by_barcode}},
        {3, "3-名  称",   COLOR_TINT, {4, 4}, {2, cmd_view_plu_by_name}},
    },
};

/*
 * 发票管理3
 */
struct frame inv_manage_menu = {
    .id = 3,
    .item_num = 6,

    .items = {
        {0, "发票管理",     COLOR_TINT, {1, 4}, {3, NULL}},
        {1, "1-发票分发",   COLOR_TINT, {2, 1}, {3, cmd_dist_inv}},
        {2, "2-发票装卷",   COLOR_TINT, {3, 1}, {3, cmd_mount_roll}},
        {3, "3-查看当前卷", COLOR_TINT, {4, 1}, {3, cmd_view_cur_roll}},
        {4, "4-读领购信息", COLOR_TINT, {2, 6}, {3, cmd_view_buyed_inv_info}},
        {5, "5-查已分发卷", COLOR_TINT, {3, 6}, {3, cmd_view_disted_roll}},
    },
};

/*
 * 税控管理4
 */
struct frame fiscal_manage_menu = {
    .id = 4,
    .item_num = 7,

    .items = {
        {0, "税控管理",     COLOR_TINT, {1, 4}, {4,  NULL}},
        {1, "1-税控申报",   COLOR_TINT, {2, 1}, {41, NULL}},
        {2, "2-税控更新",   COLOR_TINT, {3, 1}, {4,  cmd_update_control}},
        {3, "3-税控稽查",   COLOR_TINT, {4, 1}, {42, NULL}},
        {4, "4-税控初始化", COLOR_TINT, {2, 6}, {0, cmd_fiscal_init}},
        {5, "5-停机过户",   COLOR_TINT, {3, 6}, {4, cmd_mach_transfer}},
        {6, "6-更新用户名", COLOR_TINT, {4, 6}, {4, cmd_update_taxpayer}},
    },
};

/*
 * 税控申报41
 */
struct frame fiscal_declare_menu = {
    .id = 41,
    .item_num = 4,

    .items = {
        {0, "税控申报",     COLOR_TINT, {1, 4}, {41, NULL}},
        {1, "1-正常申报",   COLOR_TINT, {2, 3}, {4,  cmd_normal_declare_duty}},
        {2, "2-自然月申报", COLOR_TINT, {3, 3}, {4,  cmd_month_declare_duty}},
        {3, "3-申报预览",   COLOR_TINT, {4, 3}, {4,  cmd_declare_preview}},
    },
};

/*
 * 税控稽查42
 */
struct frame fiscal_inspect_menu = {
    .id = 42,
    .item_num = 5,

    .items = {
        {0, "税控稽查",     COLOR_TINT, {1, 4}, {42, NULL}},
        {1, "1-发票明细",   COLOR_TINT, {2, 1}, {421, NULL}},
        {2, "2-日 汇 总",   COLOR_TINT, {3, 1}, {4, cmd_inspect_daily_collect}},
        {3, "3-申报数据",   COLOR_TINT, {2, 6}, {4, cmd_inspect_declare_data}},
        {4, "4-串口稽查",   COLOR_TINT, {3, 6}, {4, cmd_inspect_by_uart}},
    },
};

/*
 * 发票明细稽查421
 */
struct frame inv_inspect_menu = {
    .id = 421,
    .item_num = 3,

    .items = {
        {0, "发票明细稽查", COLOR_TINT, {1, 3}, {421, NULL}},
        {1, "1-按起止时间", COLOR_TINT, {2, 3}, {4, cmd_inspect_inv_by_date}},
        {2, "2-按发票号",   COLOR_TINT, {3, 3}, {4, cmd_inspect_inv_by_nb}},
    },
};

/*
 * 系统管理5
 */
struct frame system_manage_menu = {
    .id = 5,
    .item_num = 7,

    .items = {
        {0, "系统管理",     COLOR_TINT, {1, 4}, {5,  NULL}},
        {1, "1-用户管理",   COLOR_TINT, {2, 1}, {51, NULL}},
        {2, "2-系统信息",   COLOR_TINT, {3, 1}, {52, NULL}},
        {3, "3-设置时钟",   COLOR_TINT, {4, 1}, {5,  cmd_set_data_time}},
        {4, "4-软件升级",   COLOR_TINT, {2, 6}, {53, NULL}},
        {5, "5-系统设置",   COLOR_TINT, {3, 6}, {5,  cmd_system_setup}},
        {6, "6-查看时间",   COLOR_TINT, {4, 6}, {5,  cmd_view_date_time}},
    },
};

/*
 * 用户管理51
 */
struct frame user_manage_menu = {
    .id = 51,
    .item_num = 5,

    .items = {
        {0, "用户管理",     COLOR_TINT, {1, 4}, {51, NULL}},
        {1, "1-添加用户",   COLOR_TINT, {2, 1}, {5,  cmd_add_user}},
        {2, "2-删除用户",   COLOR_TINT, {3, 1}, {5,  cmd_del_user}},
        {3, "3-修改用户",   COLOR_TINT, {2, 6}, {5,  cmd_modify_user}},
        {4, "4-查看用户",   COLOR_TINT, {3, 6}, {5,  cmd_view_user}},
    },
};

/*
 * 系统信息52
 */
struct frame system_info_menu = {
    .id = 52,
    .item_num = 7,

    .items = {
        {0, "系统信息",     COLOR_TINT, {1, 4}, {52, NULL}},
        {1, "1-机主信息",   COLOR_TINT, {2, 1}, {5, cmd_view_ower_info}},
        {2, "2-申报信息",   COLOR_TINT, {3, 1}, {5, cmd_view_declare_info}},
        {3, "3-机器信息",   COLOR_TINT, {4, 1}, {5, cmd_view_mach_info}},
        {4, "4-开票信息",   COLOR_TINT, {2, 6}, {5, cmd_view_issue_info}},
        {5, "5-税种信息",   COLOR_TINT, {3, 6}, {5, cmd_view_taxrate_info}},
        {6, "6-打印信息",   COLOR_TINT, {4, 6}, {5, cmd_view_print_info}},
    },
};

/*
 * 软件升级53
 */
struct frame soft_update_menu = {
    .id = 53,
    .item_num = 5,

    .items = {
        {0, "软件升级",     COLOR_TINT, {1, 4}, {54,  NULL}},
        {1, "1-串口升级",   COLOR_TINT, {2, 1}, {5, cmd_sw_update_by_uart}},
        {2, "2-网口升级",   COLOR_TINT, {3, 1}, {5, cmd_sw_update_by_network}},
        {3, "3-USB 升级",  COLOR_TINT, {2, 6}, {5, cmd_sw_update_by_usb}},
        {4, "4-设置密钥",   COLOR_TINT, {3, 6}, {5, cmd_sw_update_set_keycode}},
    },
};


/*
 * 其他菜单6
 */
struct frame others_menu = {
    .id = 6,
    .item_num = 5,

    .items = {
        {0, "其    他",     COLOR_TINT, {1, 4}, {6,  NULL}},
        {1, "1-查    询",   COLOR_TINT, {2, 1}, {61, NULL}},
        {2, "2-注销系统",   COLOR_TINT, {3, 1}, {CLEAN_UI, cmd_system_logout}},
        {3, "3-重新启动",   COLOR_TINT, {2, 6}, {CLEAN_UI, cmd_system_restart}},
        {4, "4-培训演示",   COLOR_TINT, {3, 6}, {62, NULL}},
    },
};

/*
 * 查询61
 */
struct frame view_menu = {
    .id = 61,
    .item_num = 7,

    .items = {
        {0, "查    询",    COLOR_TINT, {1, 4}, {61,  NULL}},
        {1, "1-单张发票",   COLOR_TINT, {2, 1}, {6, cmd_view_single_inv}},
        {2, "2-发票汇总",   COLOR_TINT, {3, 1}, {6, cmd_view_inv_collect}},
        {3, "3-发 票 卷",   COLOR_TINT, {4, 1}, {6, cmd_view_inv_roll}},
        {4, "4-日 汇 总",   COLOR_TINT, {2, 6}, {6, cmd_view_daily_collect}},
        {5, "5-申报数据",   COLOR_TINT, {3, 6}, {6, cmd_view_declare_data}},
        {6, "6-安全日志",   COLOR_TINT, {4, 6}, {6, cmd_view_safe_log}},
    },
};

/*
 * 演示培训62
 */
struct frame demo_train_menu = {
    .id = 62,
    .item_num = 5,

    .items = {
        {0, "演示培训",     COLOR_TINT, {1, 4}, {62,  NULL}},
        {1, "1-初 始 化",   COLOR_TINT, {2, 1}, {62, cmd_demo_fiscal_init}},
        {2, "2-发票管理",   COLOR_TINT, {3, 1}, {621, NULL}},
        {3, "3-开票操作",   COLOR_TINT, {2, 6}, {622, NULL}},
        {4, "4-查    询",   COLOR_TINT, {3, 6}, {623, NULL}},
    },
};


/*
 * 演示分发发票621
 */
struct frame demo_distribute_inv_menu = {
    .id = 621,
    .item_num = 3,

    .items = {
        {0, "演示发票管理", COLOR_TINT, {1, 2}, {621, NULL}},
        {1, "1-分卷发票",   COLOR_TINT, {2, 4}, {62, cmd_demo_dist_roll}},
        {2, "2-装卷发票",   COLOR_TINT, {3, 4}, {62, cmd_demo_mount_roll}},
    },
};

/*
 * 演示开票操作622
 */
struct frame demo_issue_inv_menu = {
    .id = 622,
    .item_num = 4,

    .items = {
        {0, "演示开票操作", COLOR_TINT, {1, 2}, {622, NULL}},
        {1, "1-开    票",   COLOR_TINT, {2, 4}, {62, cmd_demo_issue_inv}},
        {2, "2-退    票",   COLOR_TINT, {3, 4}, {62, cmd_demo_return_inv}},
        {3, "3-废    票",   COLOR_TINT, {4, 4}, {62, cmd_demo_spoil_inv}},
    },
};

/*
 * 演示查询623
 */
struct frame demo_view_inv_menu = {
    .id = 623,
    .item_num = 4,

    .items = {
        {0, "演示查询",     COLOR_TINT, {1, 4}, {623, NULL}},
        {1, "1-查看当前卷", COLOR_TINT, {2, 4}, {62, cmd_demo_view_cur_roll}},
        {2, "2-查询发票",   COLOR_TINT, {3, 4}, {62, cmd_demo_view_inv}},
        {3, "3-查询发票卷", COLOR_TINT, {4, 4}, {62, cmd_demo_view_inv_roll}},
    },
};

/*
 * 询问用户
 */
struct frame question_user_menu = {
    .id = 7,
    .item_num = 3,

    .items = {
        {0, "自定义标题",   COLOR_TINT, {1, 4}, {CLEAN_UI, NULL}},
        {1, "是",          COLOR_TINT, {3, 4}, {CLEAN_UI, SAY_YES}},
        {2, "否",          COLOR_TINT, {3, 8}, {CLEAN_UI, SAY_NO}},
    },
};
#endif 

struct frame main_menu[] = {
    {
        .id = 0,
        .item_num = 7,

        .items = {
            /* id title         color       pos     next */ 
            {0, "请选择操作", COLOR_TINT, {1, 4}, {0, NULL}},
            {1, "1-开票操作", COLOR_TINT, {2, 1}, {1, NULL}},
            {2, "2-商品管理", COLOR_TINT, {3, 1}, {2, NULL}},
            {3, "3-发票管理", COLOR_TINT, {4, 1}, {3, NULL}},
            {4, "4-税控管理", COLOR_TINT, {2, 7}, {4, NULL}},
            {5, "5-系统管理", COLOR_TINT, {3, 7}, {5, NULL}},
            {6, "6-其    他", COLOR_TINT, {4, 7}, {6, NULL}},
        },
    },
};

struct frame inv_ops[] = {
    {
        .id = 1,
        .item_num = 7,

        .items = {
            {0, "开票操作",     COLOR_TINT, {1, 5}, {1, NULL}},
            {1, "1-商品销售",   COLOR_TINT, {2, 1}, {11, NULL}},
            {2, "2-手工开票",   COLOR_TINT, {3, 1}, {1, cmd_man_issue_inv}},
            {3, "3-退票处理",   COLOR_TINT, {4, 1}, {1, cmd_return_inv}},
            {4, "4-作废当前票", COLOR_TINT, {2, 7}, {1, cmd_spoil_cur_inv}},
            {5, "5-作废剩余票", COLOR_TINT, {3, 7}, {1, cmd_spoil_left_inv}},
            {6, "6-区间废票",   COLOR_TINT, {4, 7}, {1, cmd_spoil_area_inv}},
        },
    },

    {
        .id = 11,
        .item_num = 3,

        .items = {
            {0, "商品销售",     COLOR_TINT, {1, 5}, {11, NULL}},
            {1, "1-按商品编号", COLOR_TINT, {2, 3}, {1, cmd_transact_by_num}},
            {2, "2-条形码",     COLOR_TINT, {3, 3}, {1, cmd_transact_by_barcode}},
        },
    },
};

struct frame comm_manage[] = {
    {
        .id = 2,
        .item_num = 7,

        .items = {
            {0, "商品管理",     COLOR_TINT, {1, 5}, {2, NULL}},
            {1, "1-部类管理",   COLOR_TINT, {2, 1}, {21, NULL}},
            {2, "2-添加商品",   COLOR_TINT, {3, 1}, {2, cmd_add_plu}},
            {3, "3-删除商品",   COLOR_TINT, {4, 1}, {2, cmd_del_plu}},
            {4, "4-修改商品",   COLOR_TINT, {2, 7}, {2, cmd_modify_plu}},
            {5, "5-查询商品",   COLOR_TINT, {3, 7}, {22, NULL}},
            {6, "6-进货管理",   COLOR_TINT, {4, 7}, {2, cmd_stock_manage}},
        },
    },

    {
        .id = 21,
        .item_num = 5,

        .items = {
            {0, "部类管理",     COLOR_TINT, {1, 5}, {21, NULL}},
            {1, "1-添加部类",   COLOR_TINT, {2, 1}, {2, cmd_add_dpt}},
            {2, "2-删除部类",   COLOR_TINT, {3, 1}, {2, cmd_del_dpt}},
            {3, "3-修改部类",   COLOR_TINT, {2, 7}, {2, cmd_modify_dpt}},
            {4, "4-查看部类",   COLOR_TINT, {3, 7}, {2, cmd_view_dpt}},
        },
    },

    {
        .id = 22,
        .item_num = 4,

        .items = {
            {0, "查询商品",   COLOR_TINT, {1, 5}, {22, NULL}},
            {1, "1-编  号",   COLOR_TINT, {2, 3}, {2, cmd_view_plu_by_num}},
            {2, "2-条形码",   COLOR_TINT, {3, 3}, {2, cmd_view_plu_by_barcode}},
            {3, "3-名  称",   COLOR_TINT, {4, 3}, {2, cmd_view_plu_by_name}},
        },
    },
};

struct frame inv_manage[] = {
    {
        .id = 3,
        .item_num = 6,

        .items = {
            {0, "发票管理",     COLOR_TINT, {1, 5}, {3, NULL}},
            {1, "1-发票分发",   COLOR_TINT, {2, 1}, {3, cmd_dist_inv}},
            {2, "2-发票装卷",   COLOR_TINT, {3, 1}, {3, cmd_mount_roll}},
            {3, "3-查看当前卷", COLOR_TINT, {4, 1}, {3, cmd_view_cur_roll}},
            {4, "4-读领购信息", COLOR_TINT, {2, 7}, {3, cmd_view_buyed_inv_info}},
            {5, "5-查已分发卷", COLOR_TINT, {3, 7}, {3, cmd_view_disted_roll}},
        },
    }, 
};

struct frame  fiscal_manage[] = {
    {
        .id = 4,
        .item_num = 7,

        .items = {
            {0, "税控管理",     COLOR_TINT, {1, 5}, {4,  NULL}},
            {1, "1-税控申报",   COLOR_TINT, {2, 1}, {41, NULL}},
            {2, "2-税控更新",   COLOR_TINT, {3, 1}, {4,  cmd_update_control}},
            {3, "3-税控稽查",   COLOR_TINT, {4, 1}, {42, NULL}},
            {4, "4-税控初始化", COLOR_TINT, {2, 7}, {0, cmd_fiscal_init}},
            {5, "5-停机过户",   COLOR_TINT, {3, 7}, {4, cmd_mach_transfer}},
            {6, "6-更新用户名", COLOR_TINT, {4, 7}, {4, cmd_update_taxpayer}},
        },
    },

    {
        .id = 41,
        .item_num = 4,

        .items = {
            {0, "税控申报",     COLOR_TINT, {1, 5}, {41, NULL}},
            {1, "1-正常申报",   COLOR_TINT, {2, 3}, {4,  cmd_normal_declare_duty}},
            {2, "2-自然月申报", COLOR_TINT, {3, 3}, {4,  cmd_month_declare_duty}},
            {3, "3-申报预览",   COLOR_TINT, {4, 3}, {4,  cmd_declare_preview}},
        },
    },

    {
        .id = 42,
        .item_num = 5,

        .items = {
            {0, "税控稽查",     COLOR_TINT, {1, 4}, {42, NULL}},
            {1, "1-发票明细",   COLOR_TINT, {2, 1}, {421, NULL}},
            {2, "2-日 汇 总",   COLOR_TINT, {3, 1}, {4, cmd_inspect_daily_collect}},
            {3, "3-申报数据",   COLOR_TINT, {2, 7}, {4, cmd_inspect_declare_data}},
            {4, "4-串口稽查",   COLOR_TINT, {3, 7}, {4, cmd_inspect_by_uart}},
        },
    },

    {
        .id = 421,
        .item_num = 3,

        .items = {
            {0, "发票明细稽查", COLOR_TINT, {1, 3}, {421, NULL}},
            {1, "1-按起止时间", COLOR_TINT, {2, 3}, {4, cmd_inspect_inv_by_date}},
            {2, "2-按发票号",   COLOR_TINT, {3, 3}, {4, cmd_inspect_inv_by_nb}},
        },
    },
};

struct frame system_manage[] = {
    {
        .id = 5,
        .item_num = 7,

        .items = {
            {0, "系统管理",     COLOR_TINT, {1, 5}, {5,  NULL}},
            {1, "1-用户管理",   COLOR_TINT, {2, 1}, {51, NULL}},
            {2, "2-系统信息",   COLOR_TINT, {3, 1}, {52, NULL}},
            {3, "3-设置时钟",   COLOR_TINT, {4, 1}, {5,  cmd_set_date_time}},
            {4, "4-软件升级",   COLOR_TINT, {2, 7}, {53, NULL}},
            {5, "5-系统设置",   COLOR_TINT, {3, 7}, {5,  cmd_system_setup}},
            {6, "6-查看时间",   COLOR_TINT, {4, 7}, {5,  cmd_view_date_time}},
        },
    },

    {
        .id = 51,
        .item_num = 5,

        .items = {
            {0, "用户管理",     COLOR_TINT, {1, 5}, {51, NULL}},
            {1, "1-添加用户",   COLOR_TINT, {2, 1}, {5,  cmd_add_user}},
            {2, "2-删除用户",   COLOR_TINT, {3, 1}, {5,  cmd_del_user}},
            {3, "3-修改用户",   COLOR_TINT, {2, 7}, {5,  cmd_modify_user}},
            {4, "4-查看用户",   COLOR_TINT, {3, 7}, {5,  cmd_view_user}},
        },
    },

    {
        .id = 52,
        .item_num = 7,

        .items = {
            {0, "系统信息",     COLOR_TINT, {1, 5}, {52, NULL}},
            {1, "1-机主信息",   COLOR_TINT, {2, 1}, {5, cmd_view_ower_info}},
            {2, "2-申报信息",   COLOR_TINT, {3, 1}, {5, cmd_view_declare_info}},
            {3, "3-机器信息",   COLOR_TINT, {4, 1}, {5, cmd_view_mach_info}},
            {4, "4-开票信息",   COLOR_TINT, {2, 7}, {5, cmd_view_issue_info}},
            {5, "5-税种信息",   COLOR_TINT, {3, 7}, {5, cmd_view_taxrate_info}},
            {6, "6-打印信息",   COLOR_TINT, {4, 7}, {5, cmd_view_print_info}},
        },
    },

    {
        .id = 53,
        .item_num = 5,

        .items = {
            {0, "软件升级",     COLOR_TINT, {1, 5}, {54,  NULL}},
            {1, "1-串口升级",   COLOR_TINT, {2, 1}, {5, cmd_sw_update_by_uart}},
            {2, "2-网口升级",   COLOR_TINT, {3, 1}, {5, cmd_sw_update_by_network}},
            {3, "3-USB 升级",   COLOR_TINT, {2, 7}, {5, cmd_sw_update_by_usb}},
            {4, "4-设置密钥",   COLOR_TINT, {3, 7}, {5, cmd_sw_update_set_keycode}},
        },
    },
};

struct frame others[] = {
    {
        .id = 6,
        .item_num = 5,

        .items = {
            {0, "其    他",     COLOR_TINT, {1, 5}, {6,  NULL}},
            {1, "1-查    询",   COLOR_TINT, {2, 1}, {61, NULL}},
            {2, "2-注销系统",   COLOR_TINT, {3, 1}, {CLEAN_UI, cmd_system_logout}},
            {3, "3-重新启动",   COLOR_TINT, {2, 7}, {CLEAN_UI, cmd_system_restart}},
            {4, "4-培训演示",   COLOR_TINT, {3, 7}, {62, NULL}},
            {5, "5-开发选项",   COLOR_TINT, {3, 7}, {6, cmd_develop_sys}},
        },
    },

    {
        .id = 61,
        .item_num = 7,

        .items = {
            {0, "查    询",     COLOR_TINT, {1, 5}, {61,  NULL}},
            {1, "1-单张发票",   COLOR_TINT, {2, 1}, {6, cmd_view_single_inv}},
            {2, "2-发票汇总",   COLOR_TINT, {3, 1}, {6, cmd_view_inv_collect}},
            {3, "3-发 票 卷",   COLOR_TINT, {4, 1}, {6, cmd_view_inv_roll}},
            {4, "4-日 汇 总",   COLOR_TINT, {2, 7}, {6, cmd_view_daily_collect}},
            {5, "5-申报数据",   COLOR_TINT, {3, 7}, {6, cmd_view_declare_data}},
            {6, "6-安全日志",   COLOR_TINT, {4, 7}, {6, cmd_view_safe_log}},
        },
    },

    {
        .id = 62,
        .item_num = 5,

        .items = {
            {0, "演示培训",     COLOR_TINT, {1, 5}, {62,  NULL}},
            {1, "1-初 始 化",   COLOR_TINT, {2, 1}, {62, cmd_demo_fiscal_init}},
            {2, "2-发票管理",   COLOR_TINT, {3, 1}, {621, NULL}},
            {3, "3-开票操作",   COLOR_TINT, {2, 7}, {622, NULL}},
            {4, "4-查    询",   COLOR_TINT, {3, 7}, {623, NULL}},
        },
    },

    {
        .id = 621,
        .item_num = 3,

        .items = {
            {0, "演示发票管理", COLOR_TINT, {1, 2}, {621, NULL}},
            {1, "1-分卷发票",   COLOR_TINT, {2, 4}, {62, cmd_demo_dist_roll}},
            {2, "2-装卷发票",   COLOR_TINT, {3, 4}, {62, cmd_demo_mount_roll}},
        },
    },

    {
        .id = 622,
        .item_num = 4,

        .items = {
            {0, "演示开票操作", COLOR_TINT, {1, 2}, {622, NULL}},
            {1, "1-开    票",   COLOR_TINT, {2, 4}, {62, cmd_demo_issue_inv}},
            {2, "2-退    票",   COLOR_TINT, {3, 4}, {62, cmd_demo_return_inv}},
            {3, "3-废    票",   COLOR_TINT, {4, 4}, {62, cmd_demo_spoil_inv}},
        },
    },

    {
        .id = 623,
        .item_num = 4,

        .items = {
            {0, "演示查询",     COLOR_TINT, {1, 4}, {623, NULL}},
            {1, "1-查看当前卷", COLOR_TINT, {2, 4}, {62, cmd_demo_view_cur_roll}},
            {2, "2-查询发票",   COLOR_TINT, {3, 4}, {62, cmd_demo_view_inv}},
            {3, "3-查询发票卷", COLOR_TINT, {4, 4}, {62, cmd_demo_view_inv_roll}},
        },
    },
};

struct frame ui_question_user[] = {
    {
        .id = 7,
        .item_num = 3,

        .items = {
            {0, "自定义标题",   COLOR_TINT, {1, 4}, {CLEAN_UI, NULL}},
            {1, "是",          COLOR_TINT, {3, 4}, {CLEAN_UI, SAY_YES}},
            {2, "否",          COLOR_TINT, {3, 8}, {CLEAN_UI, SAY_NO}},
        },
    },
};

/*
 * global ui menus 
 */
struct frame * g_ui_menus[] = {
    main_menu, 		//ID: 0
    inv_ops,    	//ID: 1
    comm_manage, 	//ID: ...
    inv_manage,
    fiscal_manage,
    system_manage,
    others,
    ui_question_user,
};

struct ui_stack {
    int top;
    int stack[10];
};

/*
 * global ui statck 
 */
struct ui_stack g_ui_stack = {
    .top = 0,
    .stack = {0},
};

#endif /* _UI_MENUS_ */
