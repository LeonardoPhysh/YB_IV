
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

int err_num;


int tax_file_save_sys_cfg(struct tax_sys_config_record * sys_cfg) 
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

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


int main(void)
{
    int ret = 0;
    struct tax_sys_config_record sys_cfg;
    struct file_operate * file_ops = get_file_ops();

    ret = file_ops->creat_file("sys_config.dat", SYS_CFG_FILE_MODE, SYS_CFG_REC_NUM);
    if (ret < 0) {
        printf("Creat file failed\n");
        return 0;
    }

    memset(&sys_cfg, 0, sizeof(sys_cfg));
    
    printf("Saving Config...");
    tax_file_save_sys_cfg(&sys_cfg);
    printf("Done.\n");

    return 0;
}

