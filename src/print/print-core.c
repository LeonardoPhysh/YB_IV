/*
 * print-core.c 
 *  The core of print subsystem, provide the set of operations of printer 
 *  for upper layer. Hide the printer detail.
 *
 *  Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 *  Date : 2015.1.20
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "tax_file_op.h"

#include "print.h"

static struct printer_type * g_printer[MAX_PRINT_TYPE] = {0};

/*
 * Obsolated Design
 */
static int print_sys_boot_check(void)
{
    int ret;

    if (g_printer[FLAT_PRINT_TYPE] != NULL) { 
        if (g_printer[FLAT_PRINT_TYPE]->print_ops->print_boot_check != NULL) {
            ret = g_printer[FLAT_PRINT_TYPE]->print_ops->print_boot_check();
            if (ret < 0)
                return ret;
        }
    }
    
    if (g_printer[ROLL_PRINT_TYPE] != NULL) { 
        if (g_printer[ROLL_PRINT_TYPE]->print_ops->print_boot_check != NULL) {
            ret = g_printer[ROLL_PRINT_TYPE]->print_ops->print_boot_check();
            if (ret < 0)
                return ret;
        }
    }

    return SUCCESS;
}

static int print_sys_prepare(void)
{
    int ret;

    if (g_printer[FLAT_PRINT_TYPE] != NULL) { 
        if (g_printer[FLAT_PRINT_TYPE]->print_ops->print_prepare != NULL) {
            ret = g_printer[FLAT_PRINT_TYPE]->print_ops->print_prepare();
            if (ret < 0)
                return ret;
        }
    }

    if (g_printer[ROLL_PRINT_TYPE] != NULL) { 
        if (g_printer[ROLL_PRINT_TYPE]->print_ops->print_prepare != NULL) {
            ret = g_printer[ROLL_PRINT_TYPE]->print_ops->print_prepare();
            if (ret < 0)
                return ret;
        }
    }

    return SUCCESS;
}

static int print_sys_self_check(void)
{
    int ret;

    if (g_printer[FLAT_PRINT_TYPE] != NULL) { 
        if (g_printer[FLAT_PRINT_TYPE]->print_ops->print_self_check != NULL) {
            ret = g_printer[FLAT_PRINT_TYPE]->print_ops->print_self_check();
            if (ret < 0)
                return ret;
        }
    }

    if (g_printer[ROLL_PRINT_TYPE] != NULL) { 
        if (g_printer[ROLL_PRINT_TYPE]->print_ops->print_self_check != NULL) {
            ret = g_printer[ROLL_PRINT_TYPE]->print_ops->print_self_check();
            if (ret < 0)
                return ret;
        }
    }

    return SUCCESS;

}

static int print_sys_mach_info(struct machine_info_record * mach_info)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_mach_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_mach_info(mach_info);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_mach_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_mach_info(mach_info);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_demo_invoice(void)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_demo_invoice != NULL) {
        ret = g_printer[cur_print]->print_ops->print_demo_invoice();
        if (ret < 0)
            return ret;
       
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_demo_invoice != NULL) {
        ret = g_printer[cur_print]->print_ops->print_demo_invoice();
        if (ret < 0)
            return ret;
       
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_roll_info(struct tax_sys_used_roll_id_record *roll_rec)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_roll_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_roll_info(roll_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_roll_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_roll_info(roll_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_period_info(struct tax_sys_period_collect_record * period_rec)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_period_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_period_info(period_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_period_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_period_info(period_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_daily_info(struct tax_sys_daily_collect_record *daily_rec)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_daily_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_daily_info(daily_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_daily_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_daily_info(daily_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_declare_info(struct tax_sys_declare_duty_record *declare_rec)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_declare_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_declare_info(declare_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_declare_info != NULL) {
        ret = g_printer[cur_print]->print_ops->print_declare_info(declare_rec);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static int print_sys_invoice(struct tax_sys_invoice_detail_record *inv_detail)
{
    int ret;
    int cur_print;
    struct print_sys * print_sys = get_print_sys();

    cur_print = print_sys->cur_print;
    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_invoice != NULL) {
        ret = g_printer[cur_print]->print_ops->print_invoice(inv_detail);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }
    
    /* if we get here, current printer no print_mach_info 
     * callback, check another one
     */
    if (cur_print == FLAT_PRINT_TYPE)
        cur_print = ROLL_PRINT_TYPE;
    else 
        cur_print = FLAT_PRINT_TYPE;

    if (g_printer[cur_print] == NULL) 
        return FAIL;

    if (g_printer[cur_print]->print_ops->print_invoice != NULL) {
        ret = g_printer[cur_print]->print_ops->print_invoice(inv_detail);
        if (ret < 0)
            return ret;
        
        return SUCCESS;
    }

    return FAIL;
}

static struct print_ops print_sys_ops = {
    .print_boot_check = print_sys_boot_check,
    .print_prepare = print_sys_prepare,
    .print_self_check = print_sys_self_check,
    .print_mach_info = print_sys_mach_info,
    .print_demo_invoice = print_sys_demo_invoice,
    .print_roll_info = print_sys_roll_info,
    .print_period_info = print_sys_period_info,
    .print_daily_info = print_sys_daily_info,
    .print_declare_info = print_sys_declare_info,
    .print_invoice = print_sys_invoice,
};
/* Above is Obsolated Design */

extern struct printer_type * get_flat_printer(void);
extern struct printer_type * get_roll_printer(void);

/*
 * print_sys_init - check printer subsystem, initial 
 *                  printer present. 
 *
 *  @print_sys : printer subsystem
 */
static int print_sys_init(struct print_sys * print_sys)
{
    int ret;
    int fd;
    struct machine_info_record mach_rec;
    
    /* fisrt time booting machine, machine_info_record is 
     * not exsit */
    ret = tax_file_read_mach_info(&mach_rec);
    if (ret < 0) {
        if (ret == -EFILE_OPEN_FAIL) {
            return -EFUNC_FIRST_BOOT; 
        } else 
            return FAIL;

    }
    
    /* roll printer is build-in machine, do not need to check */ 
    if (mach_rec.cur_print == ROLL_PRINT_TYPE) {
        print_sys->cur_print = ROLL_PRINT_TYPE;
        print_sys->print_nb = mach_rec.print_nb;
        print_sys->print_type = get_roll_printer(); 
        print_sys->ops = get_roll_printer()->print_ops;

        return SUCCESS;
    }

    /* flat-printer checking... */
    fd = open(PRINT_DEV, O_RDWR);
    if (fd > 0) {
        print_sys->cur_print = FLAT_PRINT_TYPE;
        print_sys->print_nb = mach_rec.print_nb;
        print_sys->print_type = get_flat_printer(); 
        print_sys->ops = get_flat_printer()->print_ops;
        
        close(fd);
        return SUCCESS;
    } else {
        /* flat-printer not ready, if we get a roll printer, 
         * switch to roll printer 
         */
        if (mach_rec.print_nb == 2) {
            print_sys->cur_print = ROLL_PRINT_TYPE;
            print_sys->print_nb = mach_rec.print_nb;
            print_sys->print_type = get_roll_printer(); 
            print_sys->ops = get_roll_printer()->print_ops;

            return SUCCESS;
        }
    }

    return -EPRINT_NO_PRINTER;
}

static struct print_sys g_print_sys = {
    .print_sys_init = print_sys_init,
};

struct print_sys * get_print_sys(void) 
{
    return &g_print_sys;
}

/* end of print-core.c */
