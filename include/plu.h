/*
 * Head file of PLU File APIs 
 *
 * Author : Leonardo Physh <Leonardo.physh@yahoo.com.hk> 
 * Date   : 2014.9.15 
 */

#ifndef __PLU_H__
#define __PLU_H__

#include "file_core.h"

#define COM_PATH    "/usr/fcrcom/"

#define COM_CFG_FILE        COM_PATH"com_cfg.dat"
#define COM_CFG_FILE_MODE   DELETE_UNAVAIL_MODE
#define COM_CFG_REC_NUM     20

#define DPT_CFG_FILE        COM_PATH"dpt_cfg.dat"
#define DPT_CFG_FILE_MODE   DELETE_UNAVAIL_MODE
#define DPT_CFG_REC_NUM     20

#define DPT_FILE_PREFIX     "dpt_rec"
#define DPT_FILE_SUFFIX     ".dat"
#define DPT_FILE_MODE       DELETE_AVAIL_MODE
#define DPT_REC_NUM         1000

#define PLU_COUNT_FILE          COM_PATH"plu_count.dat"
#define PLU_COUNT_FILE_MODE     DELETE_UNAVAIL_MODE
#define PLU_COUNT_REC_NUM       1

#define PLU_BC_IDX_FILE         COM_PATH"bc_idx.dat"
#define PLU_BC_IDX_FILE_MODE    DELETE_UNAVAIL_MODE
#define PLU_BC_IDX_REC_NUM      20000

#define PLU_NAME_IDX_FILE       COM_PATH"name_idx.dat"
#define PLU_NAME_IDX_FILE_MODE  DELETE_UNAVAIL_MODE
#define PLU_NAME_IDX_REC_NUM    20000

#define DPT_NAME_LEN    30 
#define PLU_NAME_LEN    50

#define PLU_NO_LEN      5
#define PLU_STOCK_LEN   5
#define PLU_PRICE_LEN_INT 	4
#define PLU_PRICE_LEN_FLT 	2

#define PLU_BC_LEN      13

struct plu_item {
    /*
     * plu_num : dpt_num + plu number (1000 - 20999) 
     */
    ushort plu_num;
    
    char name[PLU_NAME_LEN + 1];
    char barcode[PLU_BC_LEN + 1];

    uchar tax_index;
    uint stock;
    uint price;
};

struct dpt_cfg_item {
#define BUSY_ITEM   1
#define FREE_ITEM   0
    int flag;
    
    /* 1 - 20*/
    int dpt_num;
};

struct dpt_item {
    /* 1 - 20*/
    int dpt_num;
    char name[DPT_NAME_LEN  + 1];
    char reserved[10];
};

struct plu_barcode_idx {
    uint crc32;
    uint plu_num;
};

struct plu_name_idx {
    uint crc32;
    uint plu_num;
};

struct plu_count {
    uint plu_count;
};

typedef int (*GET_FREE_DPT)(void);
typedef int (*IS_FREE_DPT)(int);
typedef int (*READ_DPT)(int, struct dpt_item *);
typedef int (*APPEND_DPT)(int, struct dpt_item *);
typedef int (*MODIFY_DPT)(int, struct dpt_item *);
typedef int (*DELETE_DPT)(int);

typedef int (*READ_PLU)(int, struct plu_item *);
typedef int (*GET_FREE_PLU)(int);
typedef int (*APPEND_PLU)(int, struct plu_item *);
typedef int (*MODIFY_PLU)(int, struct plu_item *);
typedef int (*DELETE_PLU)(int);
typedef int (*CREATE_FILE)(void);

typedef int (*INDEX_BY_NAME)(char *, struct plu_item *); 
typedef int (*INDEX_BY_BC)(char *, struct plu_item *);
typedef int (*GET_PLU_COUNT)(int *);

struct plu_operate 
{
    GET_FREE_DPT get_free_dpt;
    IS_FREE_DPT is_free_dpt;
    READ_DPT read_dpt;
    APPEND_DPT append_dpt;
    MODIFY_DPT modify_dpt;
    DELETE_DPT delete_dpt;

    READ_PLU read_plu;
    GET_FREE_PLU get_free_plu;
    APPEND_PLU append_plu;
    MODIFY_PLU modify_plu;
    DELETE_PLU delete_plu;
    INDEX_BY_NAME index_by_name;

#ifdef CONFIG_HPP    
    INDEX_BY_BC index_by_bc;
#endif     

    GET_PLU_COUNT get_plu_count;
    CREATE_FILE plu_init;
};

extern struct plu_operate * get_plu_ops(void);

#endif /* __PLU_H__ */
