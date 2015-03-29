/*
 * command.h - head file of command files 
 *   provice APIs about fiscal functions' callback
 *
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date : 2014.10.12
 */

#ifndef __COMMAND_HEAD__
#define __COMMAND_HEAD__

typedef int (*ACTION)(void);

extern int SAY_YES(void); 
extern int SAY_NO(void);

/*
 * invoice operations - "开票操作" 
 */
extern int cmd_resume_transact(void);
extern int cmd_transact_by_barcode(void);
extern int cmd_transact_by_num(void);
extern int cmd_man_issue_inv(void);
extern int cmd_spoil_cur_inv(void);
extern int cmd_spoil_left_inv(void);
extern int cmd_spoil_area_inv(void);
extern int cmd_return_inv(void);

/*
 * comm manage - "商品管理" 
 */
extern int cmd_add_plu(void);
extern int cmd_del_plu_by_num(void);
extern int cmd_del_plu_by_barcode(void);
extern int cmd_del_plu_by_name(void);
extern int cmd_modify_plu_by_num(void);
extern int cmd_modify_plu_by_barcode(void);
extern int cmd_modify_plu_by_name(void);
extern int cmd_view_plu_by_num(void);
extern int cmd_view_plu_by_barcode(void);
extern int cmd_view_plu_by_name(void);
extern int cmd_stock_manage(void);
extern int cmd_add_dpt(void);
extern int cmd_del_dpt(void);
extern int cmd_modify_dpt(void);
extern int cmd_view_dpt(void);

/*
 * invoice manage - "发票管理" 
 */
extern int cmd_dist_inv(void);
extern int cmd_mount_roll(void);
extern int cmd_view_cur_roll(void);
extern int cmd_view_buyed_inv_info(void);
extern int cmd_view_disted_roll(void);

/*
 * fiscal manage - "税控管理" 
 */
extern int cmd_update_control(void);
extern int cmd_fiscal_init(void);
extern int cmd_mach_transfer(void);
extern int cmd_update_taxpayer(void);
extern int cmd_normal_declare_duty(void);
extern int cmd_month_declare_duty(void);
extern int cmd_declare_preview(void);
extern int cmd_inspect_daily_collect(void);
extern int cmd_inspect_declare_data(void);
extern int cmd_inspect_by_uart(void);
extern int cmd_inspect_inv_by_date(void);
extern int cmd_inspect_inv_by_nb(void);

/*
 * system manage - "系统管理"
 */
extern int cmd_set_date_time(void);
extern int cmd_system_setup(void);
extern int cmd_view_date_time(void);
extern int cmd_add_user(void);
extern int cmd_del_user(void);
extern int cmd_modify_user(void);
extern int cmd_view_user(void);

extern int cmd_view_ower_info(void);
extern int cmd_view_declare_info(void);
extern int cmd_view_mach_info(void);
extern int cmd_view_issue_info(void);
extern int cmd_view_taxrate_info(void);
extern int cmd_view_print_info(void);

extern int cmd_sw_update_by_uart(void);
extern int cmd_sw_update_by_network(void);
extern int cmd_sw_update_by_usb(void);
extern int cmd_sw_update_set_keycode(void);

/*
 * others - "其他"
 */
extern int cmd_system_logout(void);
extern int cmd_system_restart(void);
extern int cmd_develop_sys(void);
extern int cmd_print_setup(void);

extern int cmd_view_single_inv(void);
extern int cmd_view_period_collect(void);
extern int cmd_view_inv_roll(void);
extern int cmd_view_daily_collect(void);
extern int cmd_view_declare_data(void);
extern int cmd_view_safe_log(void);

/*
 * demo - "演示培训" 
 */
extern int cmd_demo_fiscal_init(void);
extern int cmd_demo_dist_roll(void);
extern int cmd_demo_mount_roll(void);
extern int cmd_demo_issue_inv(void);
extern int cmd_demo_return_inv(void);
extern int cmd_demo_spoil_inv(void);
extern int cmd_demo_view_cur_roll(void);
extern int cmd_demo_view_inv(void);
extern int cmd_demo_view_inv_roll(void);


#endif  /* __COMMAND_HEAD__ */
