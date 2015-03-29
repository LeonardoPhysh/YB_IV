/*
 * print.c - Provide printer operations APIs
 *
 * Author: Leonardo Physh
 * Data: 2014.7.21 Rev 01
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
#include "tax_file_op.h"

/* global print buffer */
static struct print_frame * g_print_frame = NULL;

static int g_PRINT_FD_INIT = NEGATIVE;     /* Print ready Flag */
static int g_PRINT_UNIT_INIT = NEGATIVE;   /* Move unit ready flag */

struct print_frame * get_print_frame(void)
{
    if (g_print_frame == NULL) {
        g_print_frame = (struct print_frame *)malloc(sizeof(struct print_frame));
        if (g_print_frame == NULL)
            return NULL;
    } 

    return g_print_frame;
}


/*
 * print_conf - send print command to config printer 
 *  @print_fd : file describe
 *  @CMD : command 
 *  @return : status 
 */
static int print_conf(FILE *print_fp, int CMD)
{
    int i;
    int ret;
    int size;
    char buf[20];

    if (print_fp == NULL)
        return -EPRINT_NODEV;

    switch(CMD) {
        case CMD_PRINTER_INIT:
            ret = fprintf(print_fp, "%c%c", 0x1B, 0x40);
            if (ret != 2)
                return -EPRINT_WR_ERR;

            break;

        case CMD_SET_LEFT_MARGIN:
            ret = fprintf(print_fp, "%c%c%c", 0x1B, 0x6C, DEFAULT_LEFT_MARGIN);
            if (ret != 3)
                return -EPRINT_WR_ERR;

            break;

        case CMD_SET_RIGHT_MARGIN:
            ret = fprintf(print_fp, "%c%c%c", 0x1B, 0x51, DEFAULT_RIGHT_MARGIN);
            if (ret != 3)
                return -EPRINT_WR_ERR;

            break;

        case CMD_SET_MOVE_UNIT:
            if (g_PRINT_UNIT_INIT != POSITIVE) {
                ret = fprintf(print_fp, "%c%c%c%c%c%c", 0x1B, 0x28, 0x55, 1, 0, DEFAULT_MOVE_UNIT);
                if (ret != 6)
                    return -EPRINT_WR_ERR;

                g_PRINT_UNIT_INIT = POSITIVE;
            }

            break;

        case CMD_SET_TOP_MARGIN:
            if (g_PRINT_UNIT_INIT == 1) {
                ret = fprintf(print_fp, "%c%c%c%c%c%c%c%c%c", 0x1b, 0x28, 0x43, 4, 0, 
                        DEFAULT_TOP_PARA_T1, DEFAULT_TOP_PARA_T2, 
                        DEFAULT_BOT_PARA_B1, DEFAULT_BOT_PARA_B2);
                if (ret != 9)
                    return -EPRINT_WR_ERR;

            } else 
                return -EPRINT_NOMOVEUNIT;
            
            break;

        case CMD_SET_LINE_SPACE:
            ret = fprintf(print_fp, "%c%c", 0x1B, 0x32);
            if (ret != 2)
                return -EPRINT_WR_ERR;

            break;

        case CMD_ENTER_CHINESE: 
            ret = fprintf(print_fp, "%c%c", 0x1C, 0x26);
            if (ret != 2)
                return -EPRINT_WR_ERR;

            break;

        case CMD_EXIT_CHINESE:
            ret = fprintf(print_fp, "%c%c", 0x1C, 0x2E);
            if (ret != 2)
                return -EPRINT_WR_ERR;

            break;

        case CMD_CHECK_PAPER:
                ret = fprintf(print_fp, "%c%c%c", 0x1d, 0x72, 0x49);
                if (ret != 2)
                    return -EPRINT_WR_ERR;

                ret = fread(buf, 1, 1, print_fp);
                if (ret != 1)
                    return -EPRINT_RD_ERR;

                /* Paper in */
                if (buf[0] == 0)
                    return POSITIVE;
                else 
                    return -EPRINT_NOPAPER;
            break;

        case CMD_NEXT_PAGE:
            size = fprintf(print_fp, "%c", 0x0C);
            if (ret != 1)
                return -EPRINT_WR_ERR;

            break;

        default:
            return -EPRINT_NOCMD;
            break;
    }

    return SUCCESS;
}



/*
 * print_init - printer intialize 
 */
static int print_init(void)
{
    FILE *print_fp;

    struct print_frame * frame;

    if (g_PRINT_FD_INIT != 1) {
        print_fp = fopen(PRINT_DEV, "w+");
        if (print_fp == NULL)
            return -EPRINT_NODEV;

        print_conf(print_fp, CMD_PRINTER_INIT);
        //print_conf(print_fp, CMD_SET_MOVE_UNIT);
        print_conf(print_fp, CMD_SET_LINE_SPACE);

        //print_conf(print_fp, CMD_SET_LEFT_MARGIN);
        //print_conf(print_fp, CMD_SET_RIGHT_MARGIN);
        //print_conf(print_fp, CMD_SET_TOP_MARGIN);
        
        g_PRINT_FD_INIT = 1;

        fclose(print_fp);
    }
    
    frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    memset(frame, 0, sizeof(*frame));

    return SUCCESS;
}


/* 
 * print_set_hor_pos - set horizontal print position 
 *  @return : status 
 */
static int print_set_hor_pos(FILE *fp, char pos)
{
    if (pos == 0) {
        fprintf(fp, "%c%c%c", 0x1B, 0x44, 0);
        return SUCCESS;
    }

    fprintf(fp, "%c%c%c%c", 0x1B, 0x44, pos, 0);
    fprintf(fp, "%c", 0x09);

    return SUCCESS;
}

/* 
 * print_set_ver_pos - set vertical position 
 *  @return : status 
 */
static int print_set_ver_pos(FILE *fp, char pos)
{
    if (pos == 0) {
        fprintf(fp, "%c%c%c", 0x1B, 0x42, 0);
        return SUCCESS;    
    }

    fprintf(fp, "%c%c%c%c", 0x1B, 0x42, pos, 0);
    fprintf(fp, "%c", 0x0B);

    return SUCCESS;
}

/*
 * print_space - this is a abandoned APIs
 */
static int print_space(char *buf, int num)
{
    int i = 0;
    int size = 0;
    for (i = 0; i < num; i++)
        size += sprintf(buf + size, " ");

    return size;
}

/*
 * Following are print frame setup functions  
 */
static int print_set_title(char *date, char *type, int invoice_num)
{
    char *tmp;

    struct print_frame * frame = get_print_frame();
    if (frame == NULL) 
        return -EPRINT_NOMEM;

    tmp = frame->title.date;
    memset(tmp, 0, MAX_DATE_LENGTH + 1);
    snprintf(tmp, MAX_DATE_LENGTH, "%s", date);

    tmp = frame->title.type;
    memset(tmp, 0, MAX_TITLE_LENGTH + 1);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", type);

    tmp = frame->title.invoice_num;
    memset(tmp, 0, MAX_NUM_LENGTH);
    snprintf(tmp, MAX_NUM_LENGTH, "%012d", invoice_num);

    return SUCCESS;
}

static int print_set_payee_info(char *payee, char *taxnum)
{
    char *tmp;

    struct print_frame * frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->payee_info.payee_title;
    memset(tmp, 0, MAX_TITLE_LENGTH + 1);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "收款单位：");

    tmp = frame->payee_info.payee_item;
    memset(tmp, 0, MAX_CHAR_PER_ITEM + 1);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", payee);

    tmp = frame->payee_info.tax_title;
    memset(tmp, 0, MAX_TITLE_LENGTH + 1);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "税控管理码：");

    tmp = frame->payee_info.tax_num;
    memset(tmp, 0, MAX_NUM_LENGTH + 1);
    snprintf(tmp, MAX_NUM_LENGTH, "%s", taxnum);

    return SUCCESS;
}

static int print_set_buyer_info(char *buyer)
{
    char *tmp;
    
    struct print_frame *frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->buyer_info.buyer_title;
    memset(tmp, 0, MAX_TITLE_LENGTH + 1);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "付款单位：");

    tmp = frame->buyer_info.buyer_type;
    memset(tmp, 0, MAX_CHAR_PER_ITEM + 1);
    snprintf(tmp, MAX_CHAR_PER_ITEM, "%s", buyer);

    return SUCCESS;

}

static int print_set_comm_title(void)
{
    char *tmp;

    struct print_frame * frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->comm_title.type;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "经营项目");

    tmp = frame->comm_title.count;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "数量");

    tmp = frame->comm_title.price;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "单价");

    tmp = frame->comm_title.monney;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "金额");

    return SUCCESS;
}

static int print_set_comm_info(int num, struct tax_sys_comm_item *comm_info)
{
    char *tmp;
    struct print_frame * frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->comm_items[num].item_name;
    memset(tmp, 0, MAX_CHAR_PER_ITEM);
    snprintf(tmp, MAX_CHAR_PER_ITEM, "%s", comm_info->comm_plu_name);

    tmp = frame->comm_items[num].item_count;
    memset(tmp, 0, MAX_NUM_LENGTH);
    snprintf(tmp, MAX_NUM_LENGTH, "%.2f", (float)comm_info->num);
    
    tmp = frame->comm_items[num].item_price;
    memset(tmp, 0, MAX_NUM_LENGTH);
    snprintf(tmp, MAX_NUM_LENGTH, "%.2f", (float)comm_info->comm_price);

    tmp = frame->comm_items[num].item_count;
    memset(tmp, 0, MAX_NUM_LENGTH);
    snprintf(tmp, MAX_NUM_LENGTH, "%.2f", (float)comm_info->amount);

    return SUCCESS;
}

static int print_set_monney(char *chinese, int amount)
{
    char *tmp;

    struct print_frame * frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->monney.up_case_title;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "大写（RMB）：");

    tmp = frame->monney.up_total;
    memset(tmp, 0, MAX_CHAR_PER_ITEM);
    snprintf(tmp, MAX_CHAR_PER_ITEM, "%s", chinese);

    tmp = frame->monney.low_case_title;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "小写：");

    tmp = frame->monney.low_total;
    memset(tmp, 0, MAX_MONNEY_LENGHT);
    snprintf(tmp, MAX_MONNEY_LENGHT, "%.2f", (float)amount);

    return SUCCESS;
}

static int print_set_reg_info(char *reg_name, char *reg_num)
{
    char *tmp;
    int i;

    struct print_frame * frame = get_print_frame();
    if (frame == NULL)
        return -EPRINT_NOMEM;

    tmp = frame->reg_info.reg_title;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "开票人：");

    tmp = frame->reg_info.reg_name;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", reg_name);

    tmp = frame->reg_info.reg_num_title;
    memset(tmp, 0, MAX_TITLE_LENGTH);
    snprintf(tmp, MAX_TITLE_LENGTH, "%s", "注册号：");


    tmp = frame->reg_info.reg_num;
    memset(tmp, 0, MAX_NUM_LENGTH);
    /*
     * register number is taxpayer ID for now
     */
    for (i = 0; i < 8; i++)
        snprintf(tmp + i * 2, MAX_NUM_LENGTH, "%02x", reg_num[i]);

    return SUCCESS;
}

/* 
 * print_done_clean - clean up after print job done 
 *  @return : status 
 */
static int print_done_clean(FILE *fp, struct print_frame * cur_frame)
{
    /* Back peper */
    fprintf(fp, "%c", 0x0c);

    g_PRINT_FD_INIT = 0;
    g_PRINT_UNIT_INIT = 0;

    print_set_hor_pos(fp, 0);
    print_set_ver_pos(fp, 0);
    
    //memset(cur_frame, 0, sizeof(*cur_frame));
    
    fclose(fp);

    return SUCCESS;
}

/* 
 * print_check - check printer status 
 *  @return : status
 */
static int print_check(void)
{
    FILE *print_fp;

    int ret = SUCCESS;

    print_fp = fopen(PRINT_DEV, "w+");
    if (print_fp == NULL)
        return -EPRINT_NODEV;

    /* Check paper */
    ret = print_conf(print_fp, CMD_CHECK_PAPER);
    if (ret != POSITIVE)
        ret =  -EPRINT_NOPAPER;

    fclose(print_fp);

    return ret; 
}

/*
 * do_print_invoke - API that actually do print job 
 *  @return : status 
 */
static int do_print_invoke(void)
{
    int i, item_num;

    struct print_frame * cur_frame = get_print_frame();
    if (cur_frame == NULL)
        return -EPRINT_NOMEM;

    /* Open printer */
    FILE *fp = fopen(PRINT_DEV, "w+");
    if (fp == NULL)
        return -EPRINT_NODEV;

    /* Title */
    print_set_hor_pos(fp, INV_DATE_POS);
    fprintf(fp, "%s", cur_frame->title.date);

    print_set_hor_pos(fp, INV_TYPE_POS);
    fprintf(fp, "%s", cur_frame->title.type);

    print_set_hor_pos(fp, INV_NUM_POS);
    fprintf(fp, "%s\n", cur_frame->title.invoice_num);

    /* clear horizontal positioning */
    print_set_hor_pos(fp, 0); 

    /* Payee information */
    fprintf(fp, "%s", cur_frame->payee_info.payee_title);
    fprintf(fp, "%s", cur_frame->payee_info.payee_item);

    print_set_hor_pos(fp, TAX_NUM_POS);

    fprintf(fp, "%s", cur_frame->payee_info.tax_title);
    fprintf(fp, "%s\n", cur_frame->payee_info.tax_num);

    print_set_hor_pos(fp, 0); 

    /* Buyer info*/
    fprintf(fp, "%s", cur_frame->buyer_info.buyer_title);
    fprintf(fp, "%s\n", cur_frame->buyer_info.buyer_type);

    /* Commidy title*/
    fprintf(fp, "%s", cur_frame->comm_title.type);
    print_set_hor_pos(fp, COMM_COUNT_POS);

    fprintf(fp, "%s", cur_frame->comm_title.count);
    print_set_hor_pos(fp, COMM_UNIT_POS);

    fprintf(fp, "%s", cur_frame->comm_title.price);
    print_set_hor_pos(fp, COMM_MONNEY_POS);

    fprintf(fp, "%s\n", cur_frame->comm_title.monney);

    print_set_hor_pos(fp, 0); 

    /* Commidy items */
    item_num = cur_frame->comm_num;
    
    for (i = 0; i < item_num; i++) {
        fprintf(fp, "%s", cur_frame->comm_items[i].item_name);
        print_set_hor_pos(fp, COMM_COUNT_POS);

        fprintf(fp, "%s", cur_frame->comm_items[i].item_count);
        print_set_hor_pos(fp, COMM_UNIT_POS);

        fprintf(fp, "%s", cur_frame->comm_items[i].item_price);
        print_set_hor_pos(fp, COMM_MONNEY_POS);

        fprintf(fp, "%s\n", cur_frame->comm_items[i].item_monney);
        print_set_hor_pos(fp, 0); 
    }

     /* set vertical positioning */
    print_set_ver_pos(fp, MONEY_VER_POS);

    /* Commidy monney */
    fprintf(fp, "%s", cur_frame->monney.up_case_title); 
    fprintf(fp, "%s", cur_frame->monney.up_total);

    print_set_hor_pos(fp, MONEY_HOR_POS);

    fprintf(fp, "%s", cur_frame->monney.low_case_title);
    fprintf(fp, "%s\n", cur_frame->monney.low_total);

    print_set_hor_pos(fp, 0);

    /* register information */
    fprintf(fp, "%s", cur_frame->reg_info.reg_title);
    fprintf(fp, "%s", cur_frame->reg_info.reg_name);

    print_set_hor_pos(fp, REG_NUM_POS);

    fprintf(fp, "%s", cur_frame->reg_info.reg_num_title);
    fprintf(fp, "%s\n", cur_frame->reg_info.reg_num);


    print_set_hor_pos(fp, 0);
    print_set_ver_pos(fp, 0);

    /* Clean */
    print_done_clean(fp, cur_frame);

    return SUCCESS;
}

/*
 * print_invoice - API for print a invoice from invoice detail record 
 *  @inv_detail : invoice detail message 
 *  @return : status 
 */
static int print_invoice(struct tax_sys_invoice_detail_record * inv_detail)
{
    int ret, i, item_num;
    uint y, m, d;
    char buf[20 + 1] = {0};
    
    debug_msg("[Printer]: print_init...");
    g_PRINT_FD_INIT = 0;
    ret = print_init();
    if (ret < 0)
        return -EPRINT_INITFAIL;

    debug_msg("done\n");

    debug_msg("[Printer]: print_check...");
    ret = print_check();
    if (ret < 0)
        return -EPRINT_NOPAPER;
    
    debug_msg("done\n");

    bcd_to_greg(&inv_detail->invoice.date, &y, &m, &d);
    sprintf(buf, "%d-%02d-%02d", y, m, d);
    
    print_set_title(buf, "服务业", inv_detail->detail_inv_num); 
    print_set_payee_info(inv_detail->tax_payee, (char *)inv_detail->detail_fiscal_code);
    print_set_buyer_info(inv_detail->payer_name);
    print_set_comm_title();
    
    item_num = inv_detail->item_num;
    for (i = 0; i < item_num; i++) {
        print_set_comm_info(i, &inv_detail->item[i]);
    }

    memset(buf, 0, 20 + 1);
    chinese_fee((float)inv_detail->detail_amt_total, buf);

    print_set_monney(buf, inv_detail->detail_amt_total);
    print_set_reg_info(inv_detail->drawer, (char *)inv_detail->register_num);

    ret = do_print_invoke();
    if (ret != SUCCESS)
        return ret;

    return SUCCESS;
}

/*
 * print_boot_check - power on check 
 *  @return : status 
 */
static int print_boot_check(void)
{
    int print_fd = open(PRINT_DEV, O_RDWR | O_NONBLOCK);
    if (print_fd < 0)
        return -EPRINT_NODEV; 

    close(print_fd);

    return SUCCESS;
}  

struct print_ops g_print_ops = {
    .boot_check = print_boot_check,
    .print_inv = print_invoice,
    .print_prepare = print_check,  
;

struct print_ops * get_print_ops(void) 
{
    return &g_print_ops;
}

#if 1
int main(void)
{
    int ret;
    char *a = "部类信息";
    char *b = "部类信息";

    FILE *fp;

    g_PRINT_FD_INIT = 0;
    
    ret = print_init();
    if (ret < 0)
        return -EPRINT_INITFAIL;

    debug_msg("print_init done\n");

    fp = fopen(PRINT_DEV, "w+") ;
    if (fp == NULL)
        return -1;

    debug_msg("open printer done\n");

    fprintf(fp, "%s", a);
    print_set_hor_pos(fp, 10);
    fprintf(fp, "%s", b);
    print_set_hor_pos(fp, 20);
    fprintf(fp, "%s\n", b);
 
    print_set_hor_pos(fp, 0);

    print_set_ver_pos(fp, 5);
    fprintf(fp, "%s\n", a); 
    print_set_ver_pos(fp, 10);
    fprintf(fp, "%s\n", a); 

    print_set_ver_pos(fp, 0);
    
    print_done_clean(fp, NULL);

    //write(fd, str, strlen(str));

    debug_msg("write printer done\n");
    
    return 0;
}

#endif 
