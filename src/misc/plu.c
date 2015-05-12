/*
 * plu.c - PLU file operate APIs 
 *   - implement PLU file_ops APIs 
 * 
 * Author : Leonardo Physh <leonardo.physh@gmail.com> 
 * Date   : 2014.9.15
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#include "error.h"
#include "common.h"
#include "config.h"
#include "file_core.h"
#include "plu.h"

/*
 * plu_file_idx_by_name - index a plu record by it's name 
 *  @name : plu name 
 *  @offset : receive record offset 
 *  @return : status 
 */
static int plu_file_idx_by_name(char *name, int *offset)
{
    int i, ret, total;
    uint len, crc32; 
    FILE *fp;
    struct file_head_node head_node;
    struct file_record_node record_node;
    struct plu_name_idx name_idx;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(name != NULL);
    assert(file_ops != NULL);
#endif 
    
    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));

    fp = fopen(PLU_NAME_IDX_FILE, "rb+");
    if(fp == NULL)
        return -EFILE_OPEN_FAIL;
    
    len = strlen(name);
    crc32 = crc32_chk((uchar *)name, len);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    total = head_node.record_total_count;
    for (i = 0; i < total; i++) {
        ret = file_ops->read_record(fp, i + 1, &record_node);
        if (ret < 0)
            goto fail;

        memcpy(&name_idx, record_node.data, sizeof(name_idx));

        if (record_node.flag == DELETED) {
            if (name_idx.crc32 == crc32) {
                *offset = i + 1;
                ret = -EPLU_DEL_NAME; 
                goto fail;
            }
            continue;       
        } 
        else {
            if (name_idx.crc32 != crc32)
                continue;
            else { 
                *offset = i + 1;
                ret = SUCCESS;
                break;
            }
        }
    }

    if (i == total)
        ret = -EPLU_NO_NAME;

fail: 
	fclose(fp);
    return ret;
}


/*
 * plu_file_read_name_idx - read the record assigned by offset 
 *  @offset : position of record 
 *  @name_idx : buffer 
 *  @return : status 
 */
static int plu_file_read_name_idx(int offset, struct plu_name_idx *name_idx)
{
    int ret;
    FILE *fp;

    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    fp = fopen(PLU_NAME_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;
    
    memcpy(name_idx, record_node.data, sizeof(*name_idx));
    
    ret = SUCCESS;
fail:    
	fclose(fp);
    return ret;
}


/*
 * No need 
 */
static int plu_file_modify_name_idx(int offset, struct plu_name_idx * name_idx)
{
    return SUCCESS;
}


/*
 * plu_file_append_name_idx - append a name index record 
 *  @name : plu name 
 *  @plu_num : plu_num 
 *  @return : status 
 */
static int plu_file_append_name_idx(char *name, int plu_num)
{
    int ret;
    int len;
    int offset = 0;

    FILE *fp;
    struct file_record_node record_node;
    struct plu_name_idx name_idx;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(name != NULL);
    assert(file_ops != NULL);
#endif

    memset(&record_node, 0, sizeof(record_node));
    memset(&name_idx, 0, sizeof(name_idx));
    
    len = strlen(name);
    name_idx.crc32 = crc32_chk((uchar *)name, len);
    name_idx.plu_num = plu_num;
    memcpy(record_node.data, &name_idx, sizeof(name_idx));

    fp = fopen(PLU_NAME_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = plu_file_idx_by_name(name, &offset);
    if (ret == SUCCESS) {
        /* name is exsit */ 
        ret = -EPLU_NAME_EXSIT;
        goto fail;
    } else {
        /* name is not exsit */ 
        if (ret == -EPLU_DEL_NAME) {
            ret = file_ops->modify_record(fp, offset, &record_node);
            if (ret < 0)
                goto fail;
        } else if (ret == -EPLU_NO_NAME) {
            ret = file_ops->append_record(fp, &record_node);
            if (ret < 0)
                goto fail;
        }
    }

    ret = SUCCESS;
fail: 
	fclose(fp);
    return SUCCESS;
}

/*
 * plu_file_delete_name_idx - delete a name index record 
 *  @name : assigned the record which will be deleted 
 *  @return : status 
 */
static int plu_file_delete_name_idx(char * name)
{
    int ret;
    int offset;

    FILE *fp;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(name != NULL);
    assert(file_ops != NULL);
#endif

    fp = fopen(PLU_BC_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = plu_file_idx_by_name(name, &offset);
    if (ret == SUCCESS) {
        /* name is exsit */ 
        ret = file_ops->delete_record(fp, offset);
        if (ret < 0)
            goto fail;
    }

    ret = SUCCESS;
fail: 
	fclose(fp);
    return ret;
}


#ifdef CONFIG_HHP
/*
 * plu_file_idx_by_barcord - find the corresponding plu_num by barcode
 *  @barcode : 13bytes barcode
 *  @return : status 
 */
static int plu_file_idx_by_barcord(char *barcode, int *offset)
{
    int i, ret, total;
    uint crc32;
    FILE *fp;
    struct file_head_node head_node;
    struct file_record_node record_node;
    struct plu_barcode_idx barcode_idx;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(barcode != NULL);
    assert(file_ops != NULL);
#endif 

    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));

    fp = fopen(PLU_BC_IDX_FILE, "rb+");
    if(fp == NULL)
        return -EFILE_OPEN_FAIL;

    crc32 = crc32_chk(barcode, 12);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    total = head_node.record_total_count;
    for (i = 0; i < total; i++) {
        ret = file_ops->read_record(fp, i + 1, &record_node);
        if (ret < 0)
            goto fail;

        memcpy(&barcode_idx, record_node.data, sizeof(barcode_idx));

        if (record_node.flag == DELETED) {
            if (barcode_idx.crc32 == crc32) {
                *offset = i + 1;
                ret = -EPLU_DEL_BARCODE;
                goto fail;
            }

            continue;       
        } 
        else {
            if (barcode_idx.crc32 != crc32)
                continue;
            else { 
                *offset = i + 1;
                ret = SUCCESS;
                break;
            }
        }
    }
    
    if (i == total)
        ret = -EPLU_NO_BARCODE;

fail:    
	fclose(fp);
    return ret;
}
#endif 

#ifdef CONFIG_HHP

static int plu_file_read_barcord_idx(int offset, struct plu_barcode_idx * bar_idx)
{
    int ret;
    FILE *fp;

    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    fp = fopen(PLU_BC_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;
    
    memcpy(bar_idx, record_node.data, sizeof(*bar_idx));
    
    ret = SUCCESS;
fail:    
	fclose(fp);
    return ret;
}
#endif /* CONFIG_HHP */ 

#ifdef CONFIG_HHP
/*
 * No need 
 */
static int plu_file_modify_barcode_idx(int offset, int plu_num)
{
    return SUCCESS;
}
#endif /* CONFIG_HHP */ 

#ifdef CONFIG_HHP
/*
 * plu_file_append_barcord_idx - append a barcode index record to bc_idx file 
 *  @barcode - 13bits barcode
 *  @plu_num - corresponding plu_num
 *  @return - status  
 */
static int plu_file_append_barcord_idx(char *barcode, int plu_num)
{
    int ret;
    int offset;
    FILE *fp;
    struct file_record_node record_node;
    struct plu_barcode_idx barcode_idx;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(barcode != NULL);
    assert(file_ops != NULL);
#endif

    memset(&record_node, 0, sizeof(record_node));
    memset(&barcode_idx, 0, sizeof(barcode_idx));

    barcode_idx.crc32 = crc32_chk(barcode, 12);
    barcode_idx.plu_num = plu_num;
    memcpy(record_node.data, &barcode_idx, sizeof(barcode_idx));

    fp = fopen(PLU_BC_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = plu_file_idx_by_barcord(barcode, &offset);
    if (ret == SUCCESS) {
        /* barcode is exsit */ 
        ret = -EPLU_BARCODE_EXSIT;
        goto fail;
    } else {
        /* barcode is not exsit */ 
        if (ret == -EPLU_DEL_BARCODE) {
            ret = file_ops->modify_record(fp, offset, &record_node);
            if (ret < 0)
                goto fail;
        } else if (ret == -EPLU_NO_BARCODE) {
            ret = file_ops->append_record(fp, &record_node);
            if (ret < 0)
                goto fail;
        }
    }
    
    ret = SUCCESS; 
fail:
    fclose(fp);
    return ret;
}
#endif 


#ifdef CONFIG_HHP
/*
 * plu_file_delete_barcode_idx - delete a barcode index record 
 *  @barcode : 13bytes barcode
 *  @return : status 
 */
static int plu_file_delete_barcode_idx(char *barcode)
{
    int ret;
    int offset;
    FILE *fp;
    struct file_operate *file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(barcode != NULL);
    assert(file_ops != NULL);
#endif

    fp = fopen(PLU_BC_IDX_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = plu_file_idx_by_barcord(barcode, &offset);
    if (ret == SUCCESS) {
        /* barcode is exsit */ 
        ret = file_ops->delete_record(fp, offset);
        if (ret < 0)
            goto fail;

        ret = SUCCESS;
    }
    /* barcode is not exsit or error occur */
fail: 
	fclose(fp);
    return FAIL;
}
#endif 


/*
 * plu_file_read_dpt_cfg - read one record frome DPT_CFG_FILE
 *  
 *  @dtp_num : dpt number 
 *  @dpt_cfg : receive buffer
 *  @return : status 
 */
static int plu_file_read_dpt_cfg(int dpt_num, struct dpt_cfg_item * dpt_cfg)
{
    int ret;
    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_cfg != NULL);
    assert(dpt_num >= 1 && dpt_num <= 20);
#endif 

    fp = fopen(DPT_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, dpt_num, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(dpt_cfg, record_node.data, sizeof(*dpt_cfg));

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_append_dpt_cfg - add a new dpt_cfg_item to end of file 
 *  @dpt_cfg : item will added to file 
 *  @return : status 
 */
static int plu_file_append_dpt_cfg(struct dpt_cfg_item * dpt_cfg)
{
    int ret;
    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_cfg != NULL);
    assert(file_ops != NULL);
#endif 

    fp = fopen(DPT_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    record_node.flag = IN_USING;
    memcpy(record_node.data, dpt_cfg, sizeof(struct dpt_cfg_item));

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_modify_dpt_cfg - modify a dpt_cfg_item 
 *  @dpt_num : dpt number 
 *  @dpt_cfg : data will writen to file 
 *  @return : status 
 */
static int plu_file_modify_dpt_cfg(int dpt_num, struct dpt_cfg_item * dpt_cfg)
{
    int ret;
    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_cfg != NULL);
    assert(dpt_num >= 1 && dpt_num <= 20);
#endif 

    fp = fopen(DPT_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, dpt_num, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(record_node.data, dpt_cfg, sizeof(struct dpt_cfg_item));

    ret = file_ops->modify_record(fp, dpt_num, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);
    return SUCCESS;
}

/*
 * plu_file_get_free_dpt - get the first available dpt_num 
 *  @return : free dpt number 
 */
static int plu_file_get_free_dpt(void)
{
    int ret;
    int index; 
    struct dpt_cfg_item dpt_cfg;
    memset(&dpt_cfg, 0, sizeof(dpt_cfg));

    index = 0;
    for (index = 0; index < 20; index++) {
        ret = plu_file_read_dpt_cfg(index + 1, &dpt_cfg);
        if (ret < 0)
            return ret;

        if (dpt_cfg.flag == FREE_ITEM)
            break;
    }

    return index + 1;
}

/*
 * plu_file_get_dpt_amount - get the amount of DPT 
 *  @return : amount of dpt 
 */
static int plu_file_get_dpt_amount(void)
{
    int ret;
    int index, amount;
    struct dpt_cfg_item dpt_cfg;

    memset(&dpt_cfg, 0, sizeof(dpt_cfg));

    amount = 0;
    index = 0;
    for (index = 0; index < 20; index++) {
        ret = plu_file_read_dpt_cfg(index + 1, &dpt_cfg);
        if (ret < 0)
            return ret;

        if (dpt_cfg.flag == BUSY_ITEM)
            amount++;
    }

    return amount;
}

/*
 * plu_file_is_free_dpt - judge wether a dpt_num is free 
 *  @dpt_num : dpt_num to be judged
 *  @return : N/P
 */
static int plu_file_is_free_dpt(int dpt_num)
{
    int ret;
    struct dpt_cfg_item cfg_item;

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
#endif 

    memset(&cfg_item, 0, sizeof(cfg_item));

    ret = plu_file_read_dpt_cfg(dpt_num, &cfg_item);
    if (ret < 0)
        return ret;

    if (cfg_item.flag == FREE_ITEM)
        return POSITIVE;
    else 
        return NEGATIVE;
}

/*
 * plu_file_read_dpt - read dpt_item record 
 *  @dpt_num : assigned dpt_num 
 *  @dpt_item :receive buffer 
 *  @return : status 
 */
static int plu_file_read_dpt(int dpt_num, struct dpt_item * dpt_item)
{ 
    int ret;
    char name[MAX_FILE_NAME_SIZE + 1] = {0}; 
    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
    assert(dpt_item != NULL);
    assert(file_ops != NULL);
#endif 

    /* shouldn't be free */
    ret = plu_file_is_free_dpt(dpt_num);
    if (ret != NEGATIVE)
        return -EPLU_DPT_IS_FREE;

    memset(&record_node, 0, sizeof(record_node));
    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(dpt_item, record_node.data, sizeof(*dpt_item));

    ret = SUCCESS;
fail:
    fclose(fp);	
    return ret;
}


/*
 * plu_file_append_dpt - append a dpt_item record to DPT_FILE 
 *  @dpt_num : get from plu_file_get_free_dpt 
 *  @dpt_item : write data  
 *  @return : status 
 */
static int plu_file_append_dpt(int dpt_num, struct dpt_item * dpt_item)
{
    int ret;
    char name[MAX_FILE_NAME_SIZE] = {0};
    FILE *fp;
    struct file_record_node record_node;
    struct dpt_cfg_item dpt_cfg_item;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
    assert(dpt_item != NULL);
    assert(file_ops != NULL);
#endif 

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret != POSITIVE)
        return -EPLU_DPT_IS_BUSY;

    memset(&record_node, 0, sizeof(record_node));
    memset(&dpt_cfg_item, 0, sizeof(dpt_cfg_item));

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /*
     * DPT is free means that dpt file has been cleared 
     */
    dpt_item->dpt_num = dpt_num;
    record_node.flag = IN_USING;
    memcpy(record_node.data, dpt_item, sizeof(struct dpt_item));

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;

    /*
     * take DPT numer 
     */
    ret = plu_file_read_dpt_cfg(dpt_num, &dpt_cfg_item);
    if (ret < 0)
        goto fail;

    dpt_cfg_item.flag = BUSY_ITEM;

    ret = plu_file_modify_dpt_cfg(dpt_num, &dpt_cfg_item);
    if (ret < 0)
        goto fail;

fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_modify_dpt - modify information of DPT 
 *  @dpt_num : DPT number 
 *  @dpt_item : data 
 *  @return : status 
 */
static int plu_file_modify_dpt(int dpt_num, struct dpt_item *dpt_item)
{
    int ret;
    char name[MAX_FILE_NAME_SIZE + 1] = {0};

    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
    assert(dpt_item != NULL);
    assert(file_ops != NULL);
#endif 

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret != NEGATIVE)
        return -EPLU_DPT_IS_FREE;

    memset(&record_node, 0, sizeof(record_node));

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(record_node.data, dpt_item, sizeof(struct dpt_item));
    ret = file_ops->modify_record(fp, 1, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:    
    fclose(fp);
    return ret;
}

/*
 * plu_file_delete_dpt - delete a dpt record and clear all 
 *                       records in file 
 *  @dpt_num : DPT number
 *  @return : status  
 */
static int plu_file_delete_dpt(int dpt_num)
{
    int ret;
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_head_node head_node;
    struct file_record_node record_node;
    struct dpt_cfg_item dpt_cfg_item;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
    assert(file_ops != NULL);
#endif 

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret != NEGATIVE)
        return -EPLU_DPT_IS_FREE;

    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));
    memset(&dpt_cfg_item, 0, sizeof(dpt_cfg_item));

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    /*
     * DPT is not empty, quit 
     */
    if (head_node.record_vaild_count > 1) {
        ret = -EPLU_DPT_NOT_EMPTY;
        goto fail;
    }

    /* DPT is empty, clear all records */ 
    ret = file_ops->clear_record(name);
    if (ret < 0)
        goto fail;

    /*
     * free DPT number
     */
    ret = plu_file_read_dpt_cfg(dpt_num, &dpt_cfg_item);
    if (ret < 0)
        goto fail;

    dpt_cfg_item.flag = FREE_ITEM;

    ret = plu_file_modify_dpt_cfg(dpt_num, &dpt_cfg_item);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_read_plu - read a plu_item 
 *  @plu_num : PLU number
 *  @plu_item : receive buffer  
 *  @return : status 
 */
static int plu_file_read_plu(int plu_num, struct plu_item * plu_item)
{
    int ret;
    int dpt_num;
    int plu_offset; 
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    memset(&record_node, 0, sizeof(record_node));

    dpt_num = plu_num / 1000;
    plu_offset = plu_num - dpt_num * 1000;
    plu_offset += 2;

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1&& dpt_num <= 20);
    assert(file_ops != NULL);
#else 
    if (dpt_num < 1 || dpt_num > 20)
        return -EPLU_BAD_DPT_NUM;
#endif

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret == POSITIVE)
        return -EPLU_DPT_IS_FREE;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_record(fp, plu_offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(plu_item, record_node.data, sizeof(*plu_item));

    ret = SUCCESS;
fail:
    fclose(fp);	
    return ret;
}

/*
 * plu_file_get_free_plu - get a available record number 
 *  @dpt_num : DPT number which new plu belong to 
 *  @return : free_plu
 */
static int plu_file_get_free_plu(int dpt_num)
{
    int ret;
    int free_plu;
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_head_node head_node;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1 && dpt_num <= 20);
    assert(file_ops != NULL);
#endif

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret == POSITIVE)
        return -EPLU_DPT_IS_FREE;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    } 
    fclose(fp);

    free_plu = dpt_num * 1000;

    /* if there is a deleted record */
    if (head_node.first_del_record != 0)
        free_plu += head_node.first_del_record;
    else 
        free_plu += head_node.record_total_count + 1;

    /*
     * plu offset =  record offset - 2
     */ 
    free_plu -= 2;
    return free_plu;
}

/*
 * plu_file_inc_count - increase plu item count 
 *  @return : status  
 */
static int plu_file_inc_count(void)
{
    int ret;
    FILE *fp; 
    struct plu_count plu_count;
    struct file_record_node record_node;
    struct file_head_node head_node;
    struct file_operate *file_ops = get_file_ops();

    fp = fopen(PLU_COUNT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&plu_count, 0, sizeof(plu_count));
    memset(&record_node, 0, sizeof(record_node));
    memset(&head_node, 0, sizeof(head_node));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    /* fisrt do this, there is no record in file, add one */ 
    if (head_node.record_vaild_count == 0) {
        plu_count.plu_count = 1; 
        memcpy(record_node.data, &plu_count, sizeof(plu_count));
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        ret = file_ops->read_record(fp, 1, &record_node);
        if (ret < 0)
            goto fail;

        memcpy(&plu_count, record_node.data, sizeof(plu_count));

        plu_count.plu_count ++;

        memcpy(record_node.data, &plu_count, sizeof(plu_count));

        ret = file_ops->modify_record(fp, 1, &record_node);
        if (ret < 0)
            goto fail;
    }

    ret = SUCCESS;

fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_inc_count - decrease plu item count 
 *  @return : status 
 */
static int plu_file_dec_count(void)
{
    int ret;
    FILE *fp;

    struct plu_count plu_count;
    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    fp = fopen(PLU_COUNT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(&plu_count, record_node.data, sizeof(plu_count));

    plu_count.plu_count --;

    memcpy(record_node.data, &plu_count, sizeof(plu_count));

    ret = file_ops->modify_record(fp, 1, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;

fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_get_plu_count - get plu item count 
 *  @count : receive buffer 
 *  @return : status
 */
static int plu_file_get_plu_count(int *count)
{
    int ret;
    FILE *fp;
    struct plu_count plu_count;
    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    fp = fopen(PLU_COUNT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        if (ret == -EFILE_OFFSET_OVER_FLOW) {
            *count = 0; 
            ret = SUCCESS;
            goto fail;
        }
    }
    memcpy(&plu_count, record_node.data, sizeof(plu_count));

    *count =  plu_count.plu_count;

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_append_plu - append_record a new plu_item to file 
 *  @free_plu : PLU number 
 *  @plu_item : data 
 *  @return : status 
 */
static int plu_file_append_plu(int free_plu, struct plu_item * plu_item)
{
    int ret;
    int dpt_num;
    int plu_offset; 
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));

    dpt_num = free_plu / 1000;
    plu_offset = free_plu - dpt_num * 1000;
    plu_offset += 2;

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1&& dpt_num <= 20);
    assert(file_ops != NULL);
#endif

#ifdef CONFIG_HHP
    /* check barcode */
    int len;
    len = strlen(plu_item->barcode);
    if (len == PLU_BC_LEN) {
        ret = plu_file_append_barcord_idx(plu_item->barcode, free_plu);
        if (ret != SUCCESS)
            return ret;
    }
#endif 

    /* check name */
    ret = plu_file_append_name_idx(plu_item->name, free_plu);
    if (ret != SUCCESS)
        return ret;

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret == POSITIVE)
        return -EPLU_DPT_IS_FREE;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    plu_item->plu_num = free_plu;
    record_node.flag = IN_USING;
    memcpy(record_node.data, plu_item, sizeof(*plu_item));

    /* add a new record */
    if (head_node.record_total_count < plu_offset) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        /* modify a exsit but deleted record */
        ret = file_ops->modify_record(fp, plu_offset, &record_node);
        if (ret < 0)
            goto fail;
    } 

    ret = plu_file_inc_count();
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_modify_plu - modify PLU information
 *  @plu_num : PLU number 
 *  @plu_item : data that will write to file 
 *  @return : status 
 */
static int plu_file_modify_plu(int plu_num, struct plu_item * plu_item)
{
    int ret;
    int dpt_num;
    int plu_offset; 
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    memset(&record_node, 0, sizeof(record_node));

    dpt_num = plu_num / 1000;
    plu_offset = plu_num - dpt_num * 1000;
    plu_offset += 2;

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1&& dpt_num <= 20);
    assert(file_ops != NULL);
#endif

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret == POSITIVE)
        return -EPLU_DPT_IS_FREE;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memcpy(record_node.data, plu_item, sizeof(*plu_item));

    ret = file_ops->modify_record(fp, plu_offset, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * plu_file_del_plu - delete a plu_item 
 *  @plu_num : PLU number 
 *  @return : status 
 */
static int plu_file_del_plu(int plu_num)
{
    int ret;
    int dpt_num;
    int plu_offset;
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;
    struct file_record_node record_node;
    struct plu_item plu_item;
    struct file_operate * file_ops = get_file_ops();

    memset(&plu_item, 0, sizeof(plu_item));
    memset(&record_node, 0, sizeof(record_node));

    dpt_num = plu_num / 1000;
    plu_offset = plu_num - dpt_num * 1000;
    plu_offset += 2;

#ifdef CONFIG_DEBUG
    assert(dpt_num >= 1&& dpt_num <= 20);
    assert(file_ops != NULL);
#endif

    ret = plu_file_is_free_dpt(dpt_num);
    if (ret == POSITIVE)
        return -EPLU_DPT_IS_FREE;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
            dpt_num, DPT_FILE_SUFFIX); 

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_record(fp, plu_offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(&plu_item, record_node.data, sizeof(plu_item));

#ifdef CONFIG_HHP
    /* check barcode */
    int len;
    len = strlen(plu_item.barcode);
    if (len == PLU_BC_LEN) {
        ret = plu_file_delete_barcode_idx(plu_item.barcode);
        if (ret != SUCCESS)
            goto fail;
    }
#endif 

    /* check name */
    ret = plu_file_delete_name_idx(plu_item.name);
    if (ret != SUCCESS)
        goto fail;

    memset(record_node.data, 0, MAX_RECORD_SIZE);

    ret = file_ops->modify_record(fp, plu_offset, &record_node);
    if (ret < 0)
        goto fail;

    ret = file_ops->delete_record(fp, plu_offset);
    if (ret < 0)
        goto fail;

    ret = plu_file_dec_count();
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);	
    return ret;
}

/*
 * plu_file_index_by_name - helper function to index plu by name 
 *  @name : plu name
 *  @item : receiver plu_item 
 */
static int plu_file_index_by_name(char *name, struct plu_item *item)
{
    int ret;
    int offset = 0;
    struct plu_name_idx name_idx; 
    
    ret = plu_file_idx_by_name(name, &offset);
    if (ret != SUCCESS) {
        return ret;
    }

    ret = plu_file_read_name_idx(offset, &name_idx);
    if (ret < 0) {
        return ret;
    }

    ret = plu_file_read_plu(name_idx.plu_num, item);
    if (ret < 0) {
        return ret;
    }
    
    return SUCCESS;
}

#ifdef CONFIG_HHP
/*
 * plu_file_index_by_bc - helper function to index plu by barcode 
 *  @name : plu barcode
 *  @item : receiver plu_item 
 */
static int plu_file_index_by_bc(char *bc, struct plu_item * item)
{
    int ret;
    int offset = 0;
    struct plu_barcode_idx bc_idx; 
    
    ret = plu_file_idx_by_barcord(bc, &offset);
    if (ret != SUCCESS) {
        return ret;
    }

    ret = plu_file_read_barcord_idx(offset, &bc_idx);
    if (ret < 0) {
        return ret;
    }

    ret = plu_file_read_plu(bc_idx.plu_num, item);
    if (ret < 0) {
        return ret;
    }

    return SUCCESS;
}
#endif 

/*
 * plu_file_create_file - create PLU files 
 *  @return : status 
 */
static int plu_file_create_file(void)
{
    int ret;
    char name[MAX_FILE_NAME_SIZE] = {0};
    struct dpt_cfg_item dpt_cfg;
    struct file_operate * file_ops = get_file_ops();

#ifdef CONFIG_DEBUG
    assert(file_ops != NULL);
#endif 

    debug_msg("DOING PLU CREATE FILES.\n");
    
    ret = file_ops->creat_dir(COM_PATH);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(COM_CFG_FILE, COM_CFG_FILE_MODE, COM_CFG_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(DPT_CFG_FILE, DPT_CFG_FILE_MODE, DPT_CFG_REC_NUM);
    if (ret < 0)
        return ret;

    /*
     * there 20 DPT files
     */
    int index = 1;
    for (index = 1; index <= DPT_CFG_REC_NUM; index++) {
        snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", COM_PATH, DPT_FILE_PREFIX, 
                index, DPT_FILE_SUFFIX); 

        ret = file_ops->creat_file(name, DPT_FILE_MODE, DPT_REC_NUM);
        if (ret < 0)
            return ret;
    }

    ret = file_ops->creat_file(PLU_BC_IDX_FILE, PLU_BC_IDX_FILE_MODE, PLU_BC_IDX_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(PLU_NAME_IDX_FILE, PLU_NAME_IDX_FILE_MODE, PLU_NAME_IDX_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(PLU_COUNT_FILE, PLU_COUNT_FILE_MODE, PLU_COUNT_REC_NUM);
    if (ret < 0)
        return ret;

    for (index = 0; index < DPT_CFG_REC_NUM; index++) {
        dpt_cfg.flag = FREE_ITEM;
        dpt_cfg.dpt_num = index + 1; 

        ret = plu_file_append_dpt_cfg(&dpt_cfg);
        if (ret < 0)
            return ret;
    } 

    /*
     * if create directory failed, no need to delete dir/file 
     * that have been created, try agian next time.
     */
    return SUCCESS;
}

/*
 * API for other modules 
 */
struct plu_operate g_plu_ops = {
    .get_free_dpt = plu_file_get_free_dpt,
    .is_free_dpt = plu_file_is_free_dpt,
    .read_dpt = plu_file_read_dpt,
    .append_dpt = plu_file_append_dpt,
    .modify_dpt = plu_file_modify_dpt,
    .delete_dpt = plu_file_delete_dpt,

    .read_plu = plu_file_read_plu,
    .get_free_plu = plu_file_get_free_plu,
    .append_plu = plu_file_append_plu,
    .modify_plu = plu_file_modify_plu,
    .delete_plu = plu_file_del_plu,

    .index_by_name = plu_file_index_by_name,
#ifdef CONFIG_HHP
    .index_by_bc = plu_file_index_by_bc,
#endif 

    .get_plu_count = plu_file_get_plu_count,
    .plu_init = plu_file_create_file,
};

struct plu_operate * get_plu_ops(void)
{
    return &g_plu_ops;
}

/* end of plu.c */


#if 0
/* FOR DEBUG, Will remove */
int err_num;

int main(void)
{
    int ret;

    struct plu_item tst_plu;
    struct dpt_item tst_dpt;  
    struct plu_name_idx name_idx; 
    struct plu_barcode_idx bc_idx; 

    char *dpt_name = "Book";
    char *plu_name = "Cook Book";
    char *tst_barcode = "6954562351233";

    int dpt_num;
    int plu_num;
    int offset;

    printf("Adding...\n");
    printf("DPT name is %s\n", dpt_name);
    printf("PLU name is %s\n", plu_name);
    printf("PLU barcode is %s\n", tst_barcode);

    ret = plu_file_create_file();
    if (ret < 0) {
        printf("create file error\n");
        return 0;
    }

    sync();

    printf("create file done\n");

    memset(&tst_plu, 0, sizeof(tst_plu));
    memset(&tst_dpt, 0, sizeof(tst_dpt));

    memcpy(tst_dpt.name, dpt_name, strlen(dpt_name));
    memcpy(tst_plu.name, plu_name, strlen(plu_name));
    memcpy(tst_plu.barcode, tst_barcode, strlen(tst_barcode));

    dpt_num = plu_file_get_free_dpt();
    ret = plu_file_append_dpt(dpt_num, &tst_dpt);
    if (ret < 0) {
        printf("append dpt error\n");
        return 0;
    }

    memset(&tst_dpt, 0, sizeof(tst_dpt));
    ret = plu_file_read_dpt(dpt_num, &tst_dpt);
    if (ret < 0) {
        printf("read dpt error\n");
        return 0;
    }

    printf("free dpt_num is %d\n", dpt_num);
    printf("dpt name is %s\n", tst_dpt.name);

    plu_num = plu_file_get_free_plu(dpt_num);
    ret = plu_file_append_plu(plu_num, &tst_plu);
    if (ret < 0) {
        printf("append plu error\n");
        return 0;
    }

    sync();

    printf("free plu_num is %d\n", plu_num);

    ret = plu_file_idx_by_name(plu_name, &offset);
    if (ret != SUCCESS) {
        printf("index by name failed\n");
        return 0;
    }

    ret = plu_file_read_name_idx(offset, &name_idx);
    if (ret < 0) {
        printf("read name index failed \n");
        return 0;
    }

    printf("name_idx barcode is %d\n", name_idx.crc32);
    printf("name_idx plu_num is %d\n", name_idx.plu_num);

    ret = plu_file_read_plu(name_idx.plu_num, &tst_plu);
    if (ret < 0) {
        printf("read plu failed\n");
        return 0;
    }

    printf("plu item name is %s\n", tst_plu.name);
    printf("plu item barcode is %s\n", tst_plu.barcode);

    ret = plu_file_idx_by_barcord(tst_barcode, &offset);
    if (ret != SUCCESS) {
        printf("index by name failed\n");
        return 0;
    }

    ret = plu_file_read_barcord_idx(offset, &bc_idx);
    if (ret < 0) {
        printf("read name index failed \n");
        return 0;
    }

    printf("name_idx crc32 is %ld\n", bc_idx.crc32);
    printf("name_idx plu_num is %d\n", bc_idx.plu_num);

    ret = plu_file_read_plu(bc_idx.plu_num, &tst_plu);
    if (ret < 0) {
        printf("read plu failed\n");
        return 0;
    }

    printf("plu item name is %s\n", tst_plu.name);
    printf("plu item barcode is %s\n", tst_plu.barcode);

    return 0;
}
#endif

