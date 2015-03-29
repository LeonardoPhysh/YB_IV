/*
 * Provide APIs to operate fiscal control files
 *  these file store with special structure, for detail, 
 *  refre to GB 18240.2-2003
 *
 * Leonardo Physh 2014.8.12 Rev01
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "error.h"
#include "tax_file_op.h"
#include "tax_system.h"
#include "file_core.h"

/*
 * NOTICE: 
 *   create : creat file with a head node 
 *   save   : for one record file, rewrite record, update head
 *   append : for mul-record file, append a record to end of file  
 *   read   : for all mode file, read a specified record from file 
 *   clear  : for all mode file, clear all records of file 
 *   modify : for mul-record file, modify a record
 *   delete : for delete availd file, set a record node as deleted 
 * --
 */

/*
 * tax_file_get_real_offset - for delete availd mode file 
 *  @filename : the file name 
 *  @offset   : app offset 
 *  @return   : real offset
 */
int tax_file_get_real_offset(FILE * fp, int offset)
{
    int ret;
    int i, next_pos, real_offset;

    struct file_head_node head_node;
    struct file_record_node record_node; 

    struct file_operate * file_ops = get_file_ops();

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        return ret;
    
    i = 0;
    next_pos = head_node.first_vaild_record;
    while (next_pos != 0){
        ret = file_ops->read_record(fp, next_pos, &record_node);
        if (ret < 0)
            return ret;

        i++;
        if (i == offset) 
            return next_pos;

        next_pos = record_node.prev_del_record;
    }
    
    return FAIL;

#if 0
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        return ret;
    }

    if (record_node.prev_del_record == 0) {
        return offset;
    } 

    i = 0;
    next_pos = record_node.prev_del_record;  

    while (next_pos != 0){
        ret = file_ops->read_record(fp, next_pos, &record_node);
        if (ret < 0)
            return ret;

        i++;
        next_pos = record_node.prev_del_record;
    }

    /*
     * i do not need to dec 1, i be 1 at least
     */
    real_offset = offset + i;

    fclose(fp);

    return real_offset;
#endif
}

/*
 * tax_file_read_user - read the user node record which asigned 
 *  @offset : postion of record node 
 *  @user : receive buffer
 */
int tax_file_read_user(int offset, struct user * read_user)
{
    int ret;
    int real_offset;

    FILE * fp;

    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(USER_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    real_offset = tax_file_get_real_offset(fp, offset);
    if (real_offset < 0) {
        fclose(fp);
        return FAIL; 
    }

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(read_user, record_node.data, sizeof(*read_user));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_user - find the user record node 
 *  @user_name : the username 
 *  @return : record node's offset 
 */
int tax_file_find_user(char *user_name, struct user * find_user)
{
    int ret;
    int next_pos;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    struct user tmp_user;

    fp = fopen(USER_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    next_pos = head_node.first_vaild_record;

    while(next_pos != 0) {
        ret = file_ops->read_record(fp, next_pos, &record_node);
        if (ret < 0) {
            fclose(fp);
            return ret;
        }

        memcpy(&tmp_user, record_node.data, sizeof(tmp_user));
        if (memcmp(user_name, tmp_user.name, strlen(user_name)) == 0) {
            *find_user = tmp_user;

            fclose(fp);
            return next_pos;
        }

        next_pos = record_node.next_vaild_record;
    }

    fclose(fp);

    return FAIL; 
}


/* 
 * tax_file_append_user - append a user record to file 
 *  @new_user : new user record will append to file
 */
int tax_file_append_user(struct user * new_user)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    struct user tmp_user;

    fp = fopen(USER_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0){
        fclose(fp);
        return ret;
    }

    if (head_node.record_total_count == head_node.max_record) {
        fclose(fp);
        return -EFILE_REC_FULL;
    }

    ret = tax_file_find_user(new_user->name, &tmp_user);
    if (ret == SUCCESS) {
        fclose(fp);
        return - ETAX_USERNAME_IMPLICT;
    }

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, new_user, sizeof(*new_user));

    record_node.flag = IN_USING; 

    if (head_node.first_del_record == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            return ret;
    } else {
        ret = file_ops->modify_record(fp, head_node.first_del_record, &record_node);
        if (ret < 0)
            return ret;
    }

    fclose(fp);

    return SUCCESS;
}

/* 
 * tax_file_modify_user - modify a user record 
 *  @offset : which record 
 *  @user : new user  
 */
int tax_file_modify_user(int offset, struct user *user)
{
    int ret;
    FILE * fp;

    int real_offset;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(USER_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    real_offset = tax_file_get_real_offset(fp, offset);
    if (real_offset < 0) {
        fclose(fp);
        return FAIL;
    }

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return FAIL;
    }

    memcpy(record_node.data, user, sizeof(*user));

    ret = file_ops->modify_record(fp, real_offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_mach_info - save new mach_info to file 
 *  @mach_info : new mach_info will save to file
 */
int tax_file_save_mach_info(struct machine_info_record * mach_info)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct machine_info_record * gp_mach_info = get_mach_info();

    /* sync global varibal */
    *gp_mach_info = *mach_info;

    fp = fopen(MACH_INFO_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memcpy(record_node.data, (void *)mach_info, MACH_INFO_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_mach_info - read machine information from file
 *  @mach_info : receive buffer
 *  @return : status 
 */
int tax_file_read_mach_info(struct machine_info_record * mach_info)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(MACH_INFO_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0){
        fclose(fp);
        return ret;
    }

    memcpy(mach_info, record_node.data, sizeof(*mach_info));

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_sys_cfg - save system config to file 
 *  @sys_cfg : new system config 
 */  
int tax_file_save_sys_cfg(struct tax_sys_config_record * sys_cfg) 
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct tax_sys_config_record * gp_sys_cfg = get_sys_config();

    /* sync global varibal */
    *gp_sys_cfg = *sys_cfg;

    fp = fopen(SYS_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)sys_cfg, SYS_CONFIG_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        /* there is no record in file */
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_sys_cfg - read the system config from file
 *  @sys_cfg : receive buffer
 */
int tax_file_read_sys_cfg(struct tax_sys_config_record * sys_cfg)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(SYS_CFG_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0){
        fclose(fp);
        return ret;
    }

    memcpy(sys_cfg, record_node.data, sizeof(*sys_cfg));

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_fis_cfg - save the new fiscal config to file
 *  @fis_cfg : new fiscal config
 */ 
int tax_file_save_fis_cfg(struct tax_sys_fis_config_record * fis_cfg)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct tax_sys_fis_config_record * gp_fis_cfg = get_fis_config();

    /* sync global varibal */
    *gp_fis_cfg = *fis_cfg;

    fp = fopen(FIS_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)fis_cfg, FIS_CONFIG_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_fis_cfg - read the fiscal config from file
 *  @fis_cfg : receive buffer 
 */
int tax_file_read_fis_cfg(struct tax_sys_fis_config_record * fis_cfg)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(FIS_CFG_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(fis_cfg, record_node.data, sizeof(*fis_cfg));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_protect - save a new protect record to file
 *  @protect_rec : new protect record 
 */
int tax_file_save_protect(struct tax_sys_protect_record * protect_rec)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct tax_sys_protect_record * gp_protect_rec = get_protect_record();

    /* sync global varibal */
    *gp_protect_rec = *protect_rec;

    fp = fopen(PROTECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /* fill up record node */
    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)protect_rec, PROTECT_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_protect - read the protect record from file 
 *  @protect_rec : receive buffer 
 */
int tax_file_read_protect(struct tax_sys_protect_record * protect_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(PROTECT_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0){
        fclose(fp);
        return ret;
    }

    memcpy(protect_rec, record_node.data, sizeof(*protect_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_read_pin - read the pin record 
 *  @pin : receive buffer
 */
int tax_file_read_pin(struct tax_sys_pin_record * pin)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(PIN_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(pin, record_node.data, sizeof(*pin));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_pin - save a new pin to file 
 *  @pin : new pin will save to file
 */
int tax_file_save_pin(struct tax_sys_pin_record *pin)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;
    struct tax_sys_pin_record ori_pin;

    struct file_operate * file_ops = get_file_ops();
    struct tax_sys_pin_record * gp_pin = get_pin();

    /* sync global varibal */
    *gp_pin = *pin;

    fp = fopen(PIN_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /* fill up record node */
    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, pin, sizeof(*pin));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        /* sync origin pin  */
        ret = tax_file_read_pin(&ori_pin);
        if (ret < 0)
            goto fail;

        ret = tax_file_save_origin_pin(&ori_pin);
        if (ret < 0)
            goto fail;

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
 * tax_file_origin_pin - read the origin pin record 
 *  @pin :receice buffer
 */
int tax_file_save_origin_pin(struct tax_sys_pin_record *pin)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct tax_sys_pin_record * gp_origin_pin;
    gp_origin_pin = get_origin_pin();

    /* sync global varibal */
    *gp_origin_pin = *pin;

    fp = fopen(ORIGIN_PIN_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /* fill up record node */
    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, pin, 8);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        /* there is no record in file */
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        ret = file_ops->modify_record(fp, 0, &record_node);
        if (ret < 0)
            goto fail;
    }

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * tax_file_read_origin_pin - read the origin pin record 
 *  @origin_pin : receive_buffer
 */
int tax_file_read_origin_pin(struct tax_sys_pin_record * origin_pin)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(ORIGIN_PIN_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(origin_pin, record_node.data, sizeof(*origin_pin));

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_app_info - save the application information to file 
 *  @app_info : new application information  
 */
int tax_file_save_app_info(struct tax_sys_app_info * app_info)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    struct tax_sys_app_info * gp_app_info = get_sys_app_info();

    /* sync global varibal */
    *gp_app_info = *app_info;

    fp = fopen(APP_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /* fill up record node */
    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)app_info, APP_INFO_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        /* there is no record in file */
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        ret = file_ops->modify_record(fp, 0, &record_node);
        if (ret < 0)
            goto fail;
    }

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;	
}

/*
 * tax_file_read_app_info - read the application information from file
 *  @app_info : receive buffer
 */
int tax_file_read_app_info(struct tax_sys_app_info * app_info)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(APP_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(app_info, record_node.data, sizeof(*app_info));

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_his_app_info - save history application information 
 *  @his_app_info : new history applicatin information
 */
int tax_file_save_his_app_info(struct tax_sys_his_app_info * his_app_info)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(HISTORY_APP_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    /* fill up record node */
    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)his_app_info, HIS_APP_INFO_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        /* there is no record in file */
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
        ret = file_ops->modify_record(fp, 0, &record_node);
        if (ret < 0)
            goto fail;
    }
    
    ret = SUCCESS;

fail:
    fclose(fp);

    return ret;
} 


/*
 * tax_file_read_his_app_info - read the application information 
 *  @his_app_info : receive buffer
 */
int tax_file_read_his_app_info(struct  tax_sys_his_app_info * his_app_info)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(APP_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(his_app_info, record_node.data, sizeof(*his_app_info));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_amount - save the new amount record to file
 *  @amount_rec : new amount record will save to file
 */
int tax_file_save_amount(struct tax_sys_amount_record * amount_rec)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(AMOUNT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, (void *)amount_rec, AMOUNT_INFO_RECORD_SIZE);

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_amount - read the amount record from file
 *  @amount_rec : receive buffer
 */
int tax_file_read_amount(struct tax_sys_amount_record * amount_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(AMOUNT_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(amount_rec, record_node.data, sizeof(amount_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_buyed_roll - save a new buyed roll record to file 
 *  @buyed_roll : new buyed_roll record 
 */
int tax_file_save_buyed_roll(struct tax_sys_buy_roll_record * buyed_roll)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(BUYED_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, buyed_roll, sizeof(*buyed_roll));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_buyed_roll - read the buyed roll record 
 *  @buyed_roll : receive buffer   
 */
int tax_file_read_buyed_roll(struct tax_sys_buy_roll_record * buyed_roll)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(AMOUNT_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(buyed_roll, record_node.data, sizeof(*buyed_roll));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_last_dist - save last time distribute record 
 *  @dist_roll_rec : new last dist_roll_rec 
 */
int tax_file_save_last_dist(struct tax_sys_invoice_roll_record * dist_roll_rec)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(LAST_DIST_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, dist_roll_rec, sizeof(*dist_roll_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_save_last_dist - read last time distribute record 
 *  @dist_roll_rec : receive buffer
 */
int tax_file_read_last_dist(struct tax_sys_invoice_roll_record * dist_roll_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(AMOUNT_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(dist_roll_rec, record_node.data, sizeof(*dist_roll_rec));

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_save_cur_roll - save the current roll record to file 
 *  @current roll record : new current roll record to save
 */
int tax_file_save_cur_roll(struct tax_sys_buy_roll_record * cur_roll_record)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CUR_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, cur_roll_record, sizeof(*cur_roll_record));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_cur_roll - read the current roll record from file 
 *  @current roll record : receive buffer
 */
int tax_file_read_cur_roll(struct tax_sys_buy_roll_record * cur_roll_record)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CUR_ROLL_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(cur_roll_record, record_node.data, sizeof(*cur_roll_record));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_crln - save new current roll left number record to file 
 *  @cur_roll_left : new record will save file
 */
int tax_file_save_crln(struct tax_sys_cur_roll_left_record *cur_roll_left)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CRLN_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, cur_roll_left, sizeof(*cur_roll_left));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_crln - read the current roll left number 
 *  @cur_roll_left : receive buffer
 */
int tax_file_read_crln(struct tax_sys_cur_roll_left_record * cur_roll_left)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CRLN_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(cur_roll_left, record_node.data, sizeof(*cur_roll_left));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_invoice_cfg - save new invoice config to file 
 *  @ invoice_cfg_rec : new invoice config record will save to  file
 */
int tax_file_save_invoice_cfg(struct tax_sys_id_cfg_record * invoice_cfg_rec)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(INVOICE_ID_CFG_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, invoice_cfg_rec, sizeof(*invoice_cfg_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_vaild_count == 0) {
        ret = file_ops->append_record(fp, &record_node);
        if (ret < 0)
            goto fail;
    } else {
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
 * tax_file_read_invoice_cfg - save the invoice config to file 
 *  @invoice_cfg_rec : the new invoice config record 
 */
int tax_file_read_invoice_cfg(struct tax_sys_id_cfg_record * invoice_cfg_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(INVOICE_ID_CFG_FILE, "rb+");
    if (fp == NULL)
        return EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, 1, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(invoice_cfg_rec, record_node.data, sizeof(*invoice_cfg_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_save_sec_invoice - save new second invoice record to file 
 *  @
 */
int tax_file_save_sec_invoice()
{
    /*
     * Do not know what for 
     */
    return SUCCESS;
}

/*
 * tax_file_read_sec_invoice - read a second invoice record from file
 *  @
 */
int tax_file_read_sec_invoice()
{
    /*
     * Do not know what for 
     */

    return SUCCESS;
}


/**
 * ATTENTION:
 *    DAILY_COLLECT_FILE has 3000 items, can save daily collect 
 *    record for 8 years, but machine should be lock down after 
 *    5 years, so we do not need to sava daily record as roll 
 *
 * tax_file_append_daily_collect - append a new daily collect record to file
 *  @daily_collect_rec : new daily collect record  
 */
int tax_file_append_daily_collect(struct tax_sys_daily_collect_record * daily_collect_rec)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DAILY_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, daily_collect_rec, sizeof(*daily_collect_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record) {
        ret -EFILE_REC_FULL;
        goto fail;
    }
    
    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_modify_daily_collect - modify daily collect record node 
 *  @offset : which record 
 *  @daily_collect_rec : new daily collect reacord 
 */
int tax_file_modify_daily_collect(int offset, 
        struct tax_sys_daily_collect_record * daily_collect_rec)
{
    int ret;
    FILE * fp;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DAILY_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(record_node.data, daily_collect_rec, sizeof(*daily_collect_rec));

    ret = file_ops->modify_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}


/*
 * tax_file_read_dail_collect - read a daily collect record from file 
 *  @offset : record position offset
 *  @daily_collect_rec : receive buffuer
 */
int tax_file_read_daily_collect(int offset, 
        struct tax_sys_daily_collect_record * daily_collect_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DAILY_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(daily_collect_rec, record_node.data, sizeof(*daily_collect_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_daily_collect - find the daily collect record 
 *                               sign by date. 
 *  @date : index    
 *  @return : offset                          
 */
int tax_file_find_daily_collect(struct bcd_date * date, 
        struct tax_sys_daily_collect_record * daily_collect_rec)
{
    int ret;
    int i, total_rec;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct tax_sys_daily_collect_record tmp_rec;

    struct rt_operate * rt_ops = get_rt_ops();
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DAILY_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node)); 
    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        return ret;

    total_rec = head_node.record_vaild_count;

    for (i = 0; i < total_rec; i++) {
        ret = file_ops->read_record(fp, i + 1, &record_node);
        if (ret < 0)
            return ret;

        memcpy(&tmp_rec, record_node.data, sizeof(*daily_collect_rec));
        if (rt_ops->cmp_bcd_date(date, &tmp_rec.cur_date) == 0) {
            *daily_collect_rec = tmp_rec;

            fclose(fp);
            return i + 1; 
        }
    }

    fclose(fp);

    return FAIL;
}

/*
 * tax_file_append_declare_duty - append a new declare duty record to file 
 *  @declare_duty_rec : new declare duty record will appended to 
 */
int tax_file_append_declare_duty(struct tax_sys_declare_duty_record * declare_duty_rec)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DECLARE_DUTY_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, declare_duty_rec, sizeof(*declare_duty_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record)
        return -EFILE_REC_FULL;

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;
    
    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_modify_declare_duty - modify the record signed by offset
 *  @offset : which record 
 *  @declare_duty_rec : new record
 */
int tax_file_modify_declare_duty(int offset, struct tax_sys_declare_duty_record * declare_duty_rec)
{
    /*
     * DECLARE_DUTY_FILE is not allowed to be modifyed
     */
    return SUCCESS;
}

/*
 * tax_file_read_declare_duty - read the record signed by offset
 *  @offset : position of record 
 *  @declare_duty_rec : receive buffer
 */
int tax_file_read_declare_duty(int offset, 
        struct tax_sys_declare_duty_record * declare_duty_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DAILY_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(declare_duty_rec, record_node.data, sizeof(*declare_duty_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_declare_duty - find the record sign by date 
 *  @data : index 
 *  @return : offset 
 */
int tax_file_find_declare_duty(struct bcd_date * date, 
        struct tax_sys_declare_duty_record * declare_duty_rec)
{
    int ret;
    int i, total_rec;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct tax_sys_declare_duty_record tmp_rec;

    struct rt_operate * rt_ops = get_rt_ops();
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DECLARE_DUTY_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node)); 
    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    total_rec = head_node.record_vaild_count;

    for (i = 0; i < total_rec; i++) {
        ret = file_ops->read_record(fp, i + 1, &record_node);
        if (ret < 0)
            goto fail;

        memcpy(&tmp_rec, record_node.data, sizeof(*declare_duty_rec));
        if (rt_ops->cmp_bcd_date(date, &tmp_rec.cur_date) == 0) {
            *declare_duty_rec = tmp_rec;

            fclose(fp);
            return i + 1; 
        }
    }

    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}


/*
 * tax_file_append_dist_roll - append a record to distribute roll file 
 *  @dist_roll_record : the record will append to file 
 */
int tax_file_append_dist_roll(struct tax_sys_invoice_roll_record *dist_roll_rec)
{
    int ret;
    FILE * fp;
    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DIST_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, dist_roll_rec, sizeof(*dist_roll_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    if (head_node.record_total_count == head_node.max_record)
        return -EFILE_REC_FULL;

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return SUCCESS;
}


/*
 * tax_file_modify_dist_roll - modify the record indicated by index 
 *  @index : the position of record at
 *  @dist_roll_record : the new content to write to file 
 */
int tax_file_modify_dist_roll(int index, 
        struct tax_sys_invoice_roll_record* dist_roll_rec)
{
    /*
     * DIST_ROLL_FILE is not allowed to be modified
     */
    return SUCCESS;
}

/*
 * tax_file_read_dist_roll - read the record indicated by index 
 *  @index : the position of record 
 *  @dist_roll_record : receive buffer
 */
int tax_file_read_dist_roll(int offset, 
        struct tax_sys_invoice_roll_record* dist_roll_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(DIST_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(dist_roll_rec, record_node.data, sizeof(*dist_roll_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_append_invoice_detail - append a new invoice detail record to file
 *  @index : which file to save this record
 *  @inv_detail_rec : new invoice detail record to save to file 
 */
int tax_file_append_invoice_detail(struct tax_sys_invoice_detail_record * inv_detail_rec)
{
    int ret;
    int index;

    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();
    
    struct tax_sys_id_cfg_record cfg_record;

    ret = tax_file_read_invoice_cfg(&cfg_record);
    if (ret < 0)
        return ret;
      
    index = cfg_record.id_index;

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", INVOICE_DETAIL_PATH, 
            INVOICE_DETAIL_PREFIX, index, INVOICE_DETAIL_SUFFIX);

    fp = fopen(name, "rb+");
    if (fp == NULL) {
        fclose(fp);
        return -EFILE_OPEN_FAIL;
    }

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record) {
        if (index < 10) 
            index++;
        else 
            index = 1;
        
        cfg_record.id_index = index; 
        
        ret = tax_file_save_invoice_cfg(&cfg_record);
        if (ret < 0)
            goto fail;

        memset(name, 0, MAX_FILE_NAME_SIZE);
        snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", INVOICE_DETAIL_PATH, 
                INVOICE_DETAIL_PREFIX, index, INVOICE_DETAIL_SUFFIX);

        tax_file_clear(name);

        fp = fopen(name, "rb+");
        if (fp == NULL) {
            fclose(fp);
            return -EFILE_OPEN_FAIL;
        }
    }

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, inv_detail_rec, sizeof(*inv_detail_rec));

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;

    ret = SUCCESS;

fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_modify_invoice_detail - modify a record of invoice detail file
 *  @index : while file need to be modified 
 *  @offset : which record need to be modified
 *  @inv_detail_rec : new record will save to file 
 */
int tax_file_modify_invoice_detail(int index, int offset,
        struct tax_sys_invoice_detail_record * inv_detail_rec)
{
#if 0
    int ret;
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;

    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", INVOICE_DETAIL_PATH, 
            INVOICE_DETAIL_PREFIX, index, INVOICE_DETAIL_SUFFIX);

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    memcpy(record_node.data, inv_detail_rec, sizeof(inv_detail_rec));

    ret = file_ops->modify_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);
#endif

    return SUCCESS;
}

/*
 * tax_file_modify_invoice_detail - modify a record of invoice detail file
 *  @index : while file need to be read 
 *  @offset : which record need to be read
 *  @inv_detail_rec : receive buffer
 */
int tax_file_read_invoice_detail(int index, int offset,
        struct tax_sys_invoice_detail_record * inv_detail_rec)
{
    int ret;
    char name[MAX_FILE_NAME_SIZE] = {0};

    FILE *fp;

    struct file_record_node record_node;
    struct file_operate *file_ops = get_file_ops();

    snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", INVOICE_DETAIL_PATH, 
            INVOICE_DETAIL_PREFIX, index, INVOICE_DETAIL_SUFFIX);

    fp = fopen(name, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node)); 
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }
     
    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_invoice_detial - find invoice detail record by 
 *                                invoice_num
 */
int tax_file_find_invoice_detial(uint inv_nb, 
        struct tax_sys_invoice_detail_record *detail_rec)
{
    /*
     * searching for high-efficiency algorithm
     */
    return SUCCESS; 
}

/*
 * tax_file_append_today_id - append a new today invoice detail record to file 
 *  @today_id_rec : new today invoice detail 
 */
int tax_file_append_today_id(struct tax_sys_today_id_record * today_id_rec)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(TODAY_ID_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, today_id_rec, sizeof(*today_id_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record)
        return -EFILE_REC_FULL;

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;
    
    ret = SUCCESS;

fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_modify_today_id - modify today invoice detail record 
 *  @offset : which record to be read
 *  @today_id_rec : new record
 */
int tax_file_modify_today_id(int offset, struct tax_sys_today_id_record * today_id_rec)
{
    /*
     * not allowed to be modified
     */
    return SUCCESS;
}

/*
 * tax_file_read_today_id - read a today invoice detail record 
 *  @offset : which record to be read
 *  @today_id_rec : receive buffer
 */
int tax_file_read_today_id(int offset, struct tax_sys_today_id_record * today_id_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(TODAY_ID_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }
        

    memcpy(today_id_rec, record_node.data, sizeof(today_id_rec));

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_today_id - find the record sign by 
 *  @date : index
 *  @return : offset  
 */
int tax_file_find_today_id(uint invoice_num, struct tax_sys_today_id_record *today_id_rec)
{
    /*
     * No need 
     */
    return SUCCESS;
}

/*
 * tax_file_append_cur_roll_id - append a current roll invoice detail record to file 
 *  @cur_roll_id_rec : new current roll invoice detail record 
 */
int tax_file_append_cur_roll_id(struct tax_sys_cur_roll_id_record * cur_roll_id_rec)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CUR_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, cur_roll_id_rec, sizeof(*cur_roll_id_rec));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record)
        return -EFILE_REC_FULL;

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;
    
    ret = SUCCESS;

fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_modify_cur_roll_id - modify the current roll invoice detial record signed by offset 
 *  @offset : which record to be modify
 *  @cur_roll_id_rec : new current roll invoice detail record 
 */
int tax_file_modify_cur_roll_id(int offset, 
        struct tax_sys_cur_roll_id_record * cur_roll_id_rec)
{
    /*
     * not allowed to be modify
     */
    return SUCCESS;
}

/*
 * tax_file_read_cur_roll_id - read the current roll invoice detial record signed by offset 
 *  @offset : which record to be read
 *  @cur_roll_id_rec : receive buffer
 */
int tax_file_read_cur_roll_id(int offset, 
        struct tax_sys_cur_roll_id_record * cur_roll_id_rec)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CUR_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_find_cur_roll_id - find the records signed by date 
 *  @date : index 
 *  @return : offset 
 */
int tax_file_find_cur_roll_id(uint invoice_num, 
        struct tax_sys_cur_roll_id_record *cur_roll_rec)
{
    /*
     * No need
     */
    return SUCCESS;
}

/*
 * tax_file_append_used_roll - append a new used roll record to file 
 *  @used_roll_record : new used roll detail record 
 */
int tax_file_append_used_roll_id(struct tax_sys_used_roll_id_record *used_roll_record)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(ROLL_COLLECT_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    memcpy(record_node.data, used_roll_record, sizeof(*used_roll_record));

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    if (head_node.record_total_count == head_node.max_record)
        return -EFILE_REC_FULL;

    ret = file_ops->append_record(fp, &record_node);
    if (ret < 0)
        goto fail;

fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_read_used_roll - read the record signed by offset
 *  @offset : position of record 
 *  @used_roll_record : receive buffer
 */
int tax_file_read_used_roll_id(int offset, 
        struct tax_sys_used_roll_id_record *used_roll_record)
{
    int ret;
    FILE * fp;

    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    fp = fopen(CUR_ROLL_FILE, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    return SUCCESS;
}

/*
 * tax_file_modify_used_roll - modify the record signed by offset
 *  @offset : position of record 
 *  @used_roll_record : new record 
 */
int tax_file_modify_used_roll_id(int offset, 
        struct tax_sys_used_roll_id_record *used_roll_record)
{
    /*
     * not allowed to be modified 
     */ 
    return SUCCESS;
}

/*
 * tax_file_find_used_roll_id - find the records signed by date 
 *  @date : index 
 *  @return : offset 
 */
int tax_file_find_used_roll_id(uint invoice_num, 
        struct tax_sys_used_roll_id_record * used_roll_record)
{
    /*
     * No Need
     */
    return SUCCESS;
}


/*
 * tax_file_crete_fiscal_file - creat fiscal system file with 
 *                              a file head node . 
 */
int tax_file_creat_fiscal_file(void)
{
    int ret;
    int index;
    char name[MAX_FILE_NAME_SIZE] = {0};

    struct file_operate * file_ops = get_file_ops();

    ret = file_ops->creat_dir(FIS_FILE_PATH);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(PROTECT_FILE, PROTECT_FILE_MODE, PROTECT_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_dir(INVOICE_DETAIL_PATH);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(INVOICE_ID_CFG_FILE, INVOICE_ID_CFG_FILE_MODE, INVOICE_ID_CFG_REC_NUM);
    if (ret < 0)
        return ret;
    
    struct tax_sys_id_cfg_record cfg_record;
    cfg_record.id_index = 1;
    ret = tax_file_save_invoice_cfg(&cfg_record);
    if (ret < 0) {
        return ret;
    }

    for (index = 0; index < MAX_ID_FILE_NUM; index++) {
        snprintf(name, MAX_FILE_NAME_SIZE, "%s%s%d%s", INVOICE_DETAIL_PATH, 
                INVOICE_DETAIL_PREFIX, index, INVOICE_DETAIL_SUFFIX);
        ret = file_ops->creat_file(name, INVOICE_DETAIL_FILE_MODE, INVOICE_DETAIL_REC_NUM);
        if (ret < 0)
            return ret;
    }

    ret = file_ops->creat_file(DAILY_COLLECT_FILE, DAILY_COLLECT_FILE_MODE, DAILY_COLLECT_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(ROLL_COLLECT_FILE, ROLL_COLLECT_FILE_MODE, ROLL_COLLECT_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(PIN_FILE, PIN_FILE_MODE, PIN_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(ORIGIN_PIN_FILE, ORIGIN_PIN_FILE_MODE, ORIGIN_PIN_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(APP_FILE, APP_FILE_MODE, APP_FILE_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(HISTORY_APP_FILE, HISTORY_APP_FILE_MODE, HISTORY_APP_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(CUR_ROLL_FILE, CUR_ROLL_FILE_MODE, CUR_ROLL_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(DIST_ROLL_FILE, DIST_ROLL_FILE_MODE, DIST_ROLL_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(LAST_DIST_FILE, LAST_DIST_FILE_MODE, LAST_DIST_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(TODAY_ID_FILE, TODAY_ID_FILE_MODE, TODAY_ID_REC_NUM);
    if (ret < 0)	
        return ret;

    ret = file_ops->creat_file(ROLL_ID_FILE, ROLL_ID_FILE_MODE, ROLL_ID_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(DECLARE_DUTY_FILE, DECLARE_DUTY_FILE_MODE, DECLARE_DUTY_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(FIS_CFG_FILE, FIS_CFG_FILE_MODE, FIS_CFG_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(CRLN_FILE, CRLN_FILE_MODE, CRLN_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(SEC_INVOICE_FILE, SEC_INVOICE_FILE_MODE, SEC_INVOICE_REC_NUM);
    if (ret < 0)
        return ret;

    ret = file_ops->creat_file(AMOUNT_FILE, AMOUNT_FILE_MODE, AMOUNT_FILE_REC_NUM);
    if (ret < 0)
        return ret;

    return SUCCESS;
}


/*********************************************
 * Common operate methods 
 *  -depend on file core methods   
 *********************************************/

/*
 * tax_file_read_last_record - read the last record 
 *  @filename : file name 
 *  @buf : receive buffer 
 *  @size : size of buffer 
 *  @return : status 
 */
int tax_file_read_last_record(const char *filename, uchar *buf, int size)
{
    int ret;
    int offset;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    offset = head_node.last_vaild_record;

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(buf, record_node.data, size);

    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}


/*
 * tax_file_modify_last_record - modify the last vaild record 
 *  @filename : file name 
 *  @buf : data 
 *  @size : size of data 
 *  @return :status 
 */
int tax_file_modify_last_record(const char *filename, uchar *buf, int size)
{
    int ret;
    int offset;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    offset = head_node.last_vaild_record;

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(record_node.data, buf, size);

    ret = file_ops->modify_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;
    
    ret = SUCCESS;
fail:
    fclose(fp);

    return ret;
}

/*
 * tax_file_read_first_recard - read the first record 
 *  @filename : file name 
 *  @buf : receive buffer 
 *  @size : size of buf 
 *  @return : status 
 */
int tax_file_read_first_record(const char *filename, uchar *buf, int size)
{
    int ret;
    int offset;

    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0)
        goto fail;

    offset = head_node.first_vaild_record;

    ret = file_ops->read_record(fp, offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(buf, record_node.data, size);

    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * tax_file_get_rec_num : get the count of vaild record 
 *  @return : count of vaild record
 */
int tax_file_get_rec_num(const char *filename)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    return head_node.record_vaild_count;
}

/*
 * tax_file_del_record - set a record node as deleted 
 *  @filename : while file 
 *  @offset : which record node 
 */
int tax_file_del_record(const char *filename, int offset)
{
    int ret;
    int rel_offset = 0;

    FILE * fp;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    rel_offset = tax_file_get_real_offset(fp, offset);

    ret = file_ops->delete_record(fp, rel_offset);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);
    return SUCCESS;
}

/*
 * tax_file_clear - clear all record nodes of file 
 *  @filename : which file neeeds to be clear 
 */
int tax_file_clear(const char *filename)
{
    int ret;

    struct file_operate * file_ops = get_file_ops();

    /* call to file core method */
    ret = file_ops->clear_record(filename);
    if (ret < 0)
        return FAIL;

    return SUCCESS;
}

/*
 * tax_file_is_empty - check file named by filename is empty or not 
 *  @filename : the file name 
 *  @return : state
 */
int tax_file_is_empty(const char *filename)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }

    fclose(fp);

    if (head_node.record_vaild_count == 0)
        return POSITIVE;
    else 
        return NEGATIVE;
}

/*
 * tax_file_is_full - check file named by filename is full or not 
 *  @filename : the file name 
 *  @return : state  
 */
int tax_file_is_full(const char *filename)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;

    struct file_operate * file_ops = get_file_ops();

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0) {
        fclose(fp);
        return ret;
    }
    
    fclose(fp);

    if (head_node.record_total_count == head_node.max_record)
        return POSITIVE;
    else 
        return NEGATIVE;
}

/*
 * tax_file_read_prev_record - read the previous record refer to offset
 *  @fp : file pointer 
 *  @offset ：refer offset 
 *  @buf : receive buffer 
 *  @size : buffer size
 *  @return : status 
 */
int tax_file_read_prev_record(FILE *fp, int offset, uchar *buf, int size)
{
    int ret;
    int real_offset;
    struct file_record_node record_node;

    struct file_operate *file_ops = get_file_ops();

    real_offset = tax_file_get_real_offset(fp, offset);

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0)
        goto fail;

    real_offset = record_node.prev_vaild_record;

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(buf, record_node.data, size);
    
    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/*
 * tax_file_read_prev_record - read the previous record refer to offset
 *  @fp : file pointer 
 *  @offset ：refer offset 
 *  @buf : receive buffer 
 *  @size : buffer size
 *  @return : status 
 */
int tax_file_read_next_record(FILE *fp, int offset, uchar *buf, int size)
{
    int ret;
    int real_offset;

    struct file_record_node record_node;

    struct file_operate *file_ops = get_file_ops();

    real_offset = tax_file_get_real_offset(fp, offset);

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0)
        goto fail;

    real_offset = record_node.next_vaild_record;

    ret = file_ops->read_record(fp, real_offset, &record_node);
    if (ret < 0)
        goto fail;

    memcpy(buf, record_node.data, size);
    
    ret = SUCCESS;
fail:
    fclose(fp);
    return ret;
}

/* beacause of there is  too many methods, open these to global */

/*
 * following code is used for generate record 
 * which need by rootfs
 */
#if 0
int main(void)
{
    int end = 0;
    struct user new_user;

    memset(&new_user, 0, sizeof(new_user));
    new_user.id = 0;
    new_user.level = DEVELOP_USER;
    strcpy(new_user.name, "debug");
    strcpy(new_user.passwd, "789456");
    
    tax_file_append_user(&new_user);
    
    memset(&new_user, 0, sizeof(new_user));
    new_user.id = 0;
    new_user.level = DEVELOP_USER;
    strcpy(new_user.name, "manager");
    strcpy(new_user.passwd, "789456");

    tax_file_append_user(&new_user);
    
    return 0;
}
#endif 
