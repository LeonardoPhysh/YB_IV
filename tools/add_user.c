
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

int tax_file_append_user(struct user * new_user)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_operate * file_ops = get_file_ops();

    struct user tmp_user;
      
    printf("Open file done\n");
    fp = fopen("user.dat", "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    printf("Read head done\n");
    ret = file_ops->read_head(fp, &head_node);
    if (ret < 0){
        fclose(fp);
        return ret;
    }

    if (head_node.record_total_count == head_node.max_record) {
        fclose(fp);
        return -EFILE_REC_FULL;
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
    
    printf("Add record done.\n");
    fclose(fp);

    return SUCCESS;
}


int main(void)
{
    int ret = 0;
    struct user new_user;
    struct file_operate * file_ops = get_file_ops();

    ret = file_ops->creat_file("user.dat", USER_FILE_MODE, USER_REC_NUM);
    if (ret < 0) {
        printf("Creat file failed\n");
        return 0;
    }

    memset(&new_user, 0, sizeof(new_user));
    new_user.id = 0;
    new_user.level = DEVELOP_USER;
    strcpy(new_user.name, "debug");
    strcpy(new_user.passwd, "789456");

    tax_file_append_user(&new_user);

    memset(&new_user, 0, sizeof(new_user));
    new_user.id = 1;
    new_user.level = MANAGER_USER;
    strcpy(new_user.name, "manager");
    strcpy(new_user.passwd, "789456");

    tax_file_append_user(&new_user);

    return 0;
}

