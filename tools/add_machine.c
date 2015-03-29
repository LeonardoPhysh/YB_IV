
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


int main(void)
{
    int ret = 0;
    uint dec_data;
    uchar bcd_data;

    uchar tmp[3] = {0};    
    char mach_nb[16 + 1] = {0};

    uint y, m, d;

    struct machine_info_record mach_info;
    struct file_operate * file_ops = get_file_ops();

    ret = file_ops->creat_file(MACH_INFO_FILE, MACH_INFO_FILE_MODE, MACH_INFO_REC_NUM);
    if (ret < 0) {
        printf("Creat file failed\n");
        return 0;
    }

    memset(&mach_info, 0, sizeof(mach_info));

    /* get machine number */
    printf("Input Machine number(16bits): ");
    scanf("%s", mach_nb);

    int i;
    for (i = 0; i < 8; i++) { 
        memcpy(tmp, mach_nb + 2 * i, 2);
        dec_data = atoi(tmp);
        dec_to_bcd(&dec_data, &bcd_data);
        mach_info.machine_nb[i] = bcd_data;
    }

    while ((ret = getchar()) != EOF && ret != '\n');

    /* get product date*/
    greg_to_bcd(&mach_info.produce_date, 2014, 11, 10);     

    strcpy(mach_info.hw_version, "DVT_2014.11.10");
    strcpy(mach_info.sw_version, "REV01_2014.11.10");

    printf("Saving machine info...");
    tax_file_save_mach_info(&mach_info);
    printf("Done.\n");

    return 0;
}

