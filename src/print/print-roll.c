/*
 * print.c - Provide printer operations APIs
 *
 * Author: Leonardo Physh <leonardo.physh@gmail.com>
 * Data: 2015.3.21
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
#include "print.h"

static int print_boot_check(void) 
{
    return SUCCESS;
}

struct print_ops roll_print_ops = {
   .print_boot_check = print_boot_check, 
};

struct printer_type roll_printer_type = {
    .id = ROLL_PRINT_TYPE,
    .state = PRINT_DOWN,
    .name = "unknow",
    .print_ops = &roll_print_ops,
};

struct printer_type * get_roll_printer(void) 
{
    return &roll_printer_type;
}

/* end of print-flat.c */
