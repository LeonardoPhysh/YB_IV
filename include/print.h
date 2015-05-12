/*
 * Head file of Printer Subsystem
 *   for now, we only support two kind of printers, 
 *   if we have to support more printers in future,
 *   we can start the job from here.
 *
 * Author : Leonardo Physh <leonardo.physh@gmail.com> 
 * Date   : 2014.4.21
 */

#ifndef __PRINT_H__
#define __PRINT_H__

#include "config.h"
#include "tax_file_op.h"

enum print_type {
    FLAT_PRINT_TYPE,
    ROLL_PRINT_TYPE,
    MAX_PRINT_TYPE,
};

typedef int (*print_boot_check_t)(void);
typedef int (*print_prepare_t)(void);
typedef int (*print_self_check_t)(void);
typedef int (*print_mach_info_t)(struct machine_info_record *mach_info);
typedef int (*print_demo_invoice_t)(void);
typedef int (*print_roll_info_t)(struct tax_sys_used_roll_id_record * roll_id);
typedef int (*print_period_info_t)(struct tax_sys_period_collect_record * period_rec);
typedef int (*print_daily_info_t)(struct tax_sys_daily_collect_record * daily_rec);
typedef int (*print_declare_info_t)(struct tax_sys_declare_duty_record * declare_rec);
typedef int (*print_invoice_t)(struct tax_sys_invoice_detail_record *);
typedef int (*print_invoice_stub_t)(struct tax_sys_invoice_detail_record *);

/* printer operations set */
struct print_ops {
    print_boot_check_t  print_boot_check;
    print_prepare_t     print_prepare;
    print_self_check_t  print_self_check;
    print_mach_info_t       print_mach_info;
    print_demo_invoice_t    print_demo_invoice;
    print_roll_info_t       print_roll_info;
    print_period_info_t     print_period_info;
    print_daily_info_t      print_daily_info;
    print_declare_info_t    print_declare_info;
    print_invoice_t         print_invoice;
    print_invoice_stub_t    print_invoice_stub;
};

/* for printer driver */
struct printer_type {
    int id;

#define PRINT_DOWN    (1 << 0)
#define PRINT_UP      (1 << 1)
#define PRINT_CUR     (1 << 2)
    int state;
    char name[24];
    
    struct print_ops * print_ops;
};

struct print_sys {
    int print_nb;  // how many printers we get 
    int cur_print; // current printer id 

    struct print_ops *ops;
    struct printer_type * print_type;
    int (*print_sys_init)(struct print_sys * print_sys);
};

extern struct print_sys * get_print_sys(void);

#endif /* __PRINT_H__ */
