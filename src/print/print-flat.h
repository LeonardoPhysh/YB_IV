/*
 * print-flat.h 
 *  Header file of flat printer operations.
 *
 * Author : Leonardo.Physh <leonardo.physh@gmail.com>
 * Date   : 2014.1.20 
 */

#ifndef __PRINT_FLAT_H__
#define __PRINT_FLAT_H__
/*
 * CMD_PRINT_INIT -- ESC @: 0x1B 40 
 * CMD_SET_LEFT_MARGIN -- ESC 1 n: 0x1B 6C n 
 * CMD_SET_MOVE_UNIT -- ESC ( U n1 n2 m
 * CMD_SET_TOP_MARGIN -- ESC ( C n1 n2 t1 t2 b1 b2
 * CMD_SET_RIGHT_MARGIN -- ESC Q n: 0x1B 0x51 n
 * CMD_SET_LINE_SPACE -- ESC A n: 0x1B 0x30
 * CMD_CHECK_PAPER -- 
 * CMD_ENTER_CHINESE --
 * CMD_EXIT_CHIINESE -- 
 */ 
#define DEFAULT_LEFT_MARGIN  4
#define DEFAULT_RIGHT_MARGIN 30
#define DEFAULT_MOVE_UNIT    120     
#define DEFAULT_TOP_PARA_T1  60
#define DEFAULT_TOP_PARA_T2  1
#define DEFAULT_BOT_PARA_B1  6 
#define DEFAULT_BOT_PARA_B2  1

/*Printer Iintialize command */
#define PRINT_CMD           10
#define CMD_PRINTER_INIT        (PRINT_CMD + 1)
#define CMD_SET_LEFT_MARGIN     (PRINT_CMD + 2)
#define CMD_SET_RIGHT_MARGIN    (PRINT_CMD + 3)
#define CMD_SET_TOP_MARGIN      (PRINT_CMD + 4)
#define CMD_SET_LINE_SPACE      (PRINT_CMD + 5)
#define CMD_SET_MOVE_UNIT       (PRINT_CMD + 6)
#define CMD_ENTER_CHINESE       (PRINT_CMD + 7)
#define CMD_EXIT_CHINESE        (PRINT_CMD + 8)
#define CMD_CHECK_PAPER         (PRINT_CMD + 9)
#define CMD_NEXT_PAGE           (PRINT_CMD + 10)

/* Print Content Position */
#define INV_DATE_POS    6
#define INV_TYPE_POS    25
#define INV_NUM_POS     35
#define TAX_NUM_POS     30 
#define COMM_COUNT_POS   	22
#define COMM_UNIT_POS  	    30
#define COMM_MONNEY_POS     40
#define MONEY_HOR_POS       35
#define MONEY_VER_POS 	50
#define REG_NUM_POS     30

/* LINE　Buffer size */
#define CHINESE_CHAR_SIZE 	CONFIG_CHN_CHAR_SIZE
#define MAX_TITLE_LENGTH    (10 * CHINESE_CHAR_SIZE)
#define MAX_CHAR_PER_ITEM   (20 * CHINESE_CHAR_SIZE) 

#define MAX_NUM_LENGTH      25
#define MAX_DATE_LENGTH     15
#define MAX_MONNEY_LENGHT   10
#define MAX_COUNT_LENGHT    5

/*
 * Print content structure 
 */
struct print_title 
{
    char date[MAX_DATE_LENGTH + 1];
    char type[MAX_TITLE_LENGTH + 1];
    char invoice_num[MAX_NUM_LENGTH + 1];
};

struct print_payee_info
{
   char payee_title[MAX_TITLE_LENGTH + 1];
   char payee_item[MAX_CHAR_PER_ITEM + 1];
   char tax_title[MAX_TITLE_LENGTH + 1];
   char tax_num[MAX_NUM_LENGTH + 1];
};

struct print_buyer_info
{
    char buyer_title[MAX_TITLE_LENGTH + 1];
    char buyer_type[MAX_CHAR_PER_ITEM + 1];
};

struct print_comm_tile
{
    char type[MAX_TITLE_LENGTH + 1];
    char count[MAX_TITLE_LENGTH + 1];
    char price[MAX_TITLE_LENGTH + 1];
    char monney[MAX_TITLE_LENGTH + 1];
};

struct print_comm_item
{
    char item_name[MAX_CHAR_PER_ITEM];
    char item_count[MAX_NUM_LENGTH];
    char item_price[MAX_NUM_LENGTH];
    char item_monney[MAX_NUM_LENGTH];    
};

struct print_comm_monney
{
 	char up_case_title[MAX_TITLE_LENGTH];
 	char up_total[MAX_CHAR_PER_ITEM];
 	char low_case_title[MAX_TITLE_LENGTH]; 
 	char low_total[MAX_MONNEY_LENGHT];
};

struct print_reg_info
{
	char reg_title[MAX_TITLE_LENGTH];
	char reg_name[MAX_TITLE_LENGTH];
	char reg_num_title[MAX_TITLE_LENGTH];
	char reg_num[MAX_NUM_LENGTH]; 
};

/*
 * title: time, commodity type, invoice number
 * payee_info : payee's info, tax code 
 * buyyer_info: buyyer info
 * comm_title: commodity title
 * items: Commodity item 
 * monney: monney
 * reg_info: register information 
 */
struct print_frame
{
	struct print_title title;
	struct print_payee_info payee_info;
	struct print_buyer_info buyer_info;
	struct print_comm_tile  comm_title;
	
	/* commodity list */
    int comm_num;
	struct print_comm_item comm_items[10];
	
	struct print_comm_monney monney;
	struct print_reg_info reg_info;
};

#endif /* __PRINT_FLAT_H__ */

