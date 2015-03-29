/*
 * file_core.c - main file of file operate core
 * -- define some common method to operate files
 * 
 * Author : Leonardo Physh 
 * Date :   2014.8.12
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "error.h"
#include "common.h"
#include "file_core.h"
#include "tax_file_op.h"

/* OPEN_FILE */
static FILE * file_ops_open_file(const char * filename)
{
    FILE * fp;

    fp = fopen(filename, "rb+");
    if (fp == NULL) {
        err_num = -EFILE_OPEN_FAIL;
        return NULL;
    }

    return fp;
}

/* CLOSE FILE */
static int file_ops_close_file(FILE *fp)
{
    int ret;

    ret = fclose(fp);
    if (ret < 0)
        return FAIL;

    return SUCCESS;
}

/* CREAT_DIR */
static int file_ops_creat_dir(const char *path)
{
	int ret;
	
	/* check path exist or not */
	ret = mkdir(path, CONFIG_DIR_MODE);
	if (ret < 0) {
		if (errno == EEXIST) {
			return SUCCESS;
		}
		
		return FAIL;
	}
	
	return SUCCESS;
}

/* DELETE_DIR */
static int file_ops_del_dir(const char *path)
{
    int ret ;
    
    ret = rmdir(path);
    if (ret < 0)
        return -EFILE_DEL_FAIL;

    return SUCCESS;
}

static int file_ops_force_del_dir(const char *path)
{
    int ret;
    char cmd[50];
    
    sprintf(cmd, "rm -rf %s", path);
    
    /*
     * system will creat a new thread to do cmd 
     * system is block until cmd is done.
     */
    ret = system(cmd);
    if (ret < 0)
        return -EFILE_DEL_FAIL;

    return SUCCESS;
}

/* READ FILE HEAD NOTE */
static int file_ops_read_head(FILE *stream, struct file_head_node  *head_node)
{
    int ret;
    int size;

    size = sizeof(struct file_head_node);

    /* seet to head of file */
    ret = fseek(stream, 0, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    /* read a record node from file */
    ret = fread((void *)head_node, size, 1, stream);
    if (ret != 1) {
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_RD_FAIL;
    }
    
    return SUCCESS;
}


/* WRITE FILE HEAD NOTE  */
static int file_ops_write_head(FILE *stream, struct file_head_node *head_node)
{
    int ret;
    int size;

    size = sizeof(struct file_head_node);

    /* seet to head of file */
    ret = fseek(stream, 0, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    /* write a head node to file */
    ret = fwrite(head_node, size, 1, stream);
    if (ret != 1) {
		if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_RD_FAIL;
    }

    return SUCCESS;
}

/* CREAT_FILE */
static int file_ops_creat_file(const char *filename, int mode, int record_num)
{
    int ret;
    int fd;
    FILE *fp;
    
    struct file_head_node head_node;

    /* check file exist or not */
    fd = open(filename, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, 0666);
    if (fd < 0) {
        if (errno != EEXIST)
            return -EFILE_OPEN_FAIL; 
        else {
			ret = truncate(filename, 0);
			if (ret < 0)
				return FAIL;
		}
    }

    /* fill head node up */
    memset(&head_node, 0, sizeof(struct file_head_node));
    memcpy(head_node.name, filename, strlen(filename));
    head_node.mode = mode;
	head_node.max_record = record_num;
	
#ifdef CONFIG_LIMIT_DEL
    if (mode == DELETE_AVAIL_MODE) 
        head_node.max_del_record = MAX_DEL_RECORD_NUM;
#endif 
	
	close(fd);
	
	fp = fopen(filename, "rb+");
	if (fp == NULL) 
		return -EFILE_OPEN_FAIL;
		
    /* write head node */
    ret = file_ops_write_head(fp, &head_node);
    if (ret < 0)
        return ret;
	
	fclose(fp);
	
    return SUCCESS;
}

/* DELETE FIEL */
static int file_ops_del_file(const char *filename)
{
    int ret;

    ret = unlink(filename);
    if (ret < 0)
        return -EFILE_DEL_FAIL;

    return SUCCESS;
}


/* CLEAR FILE */
static int file_ops_clear_record(const char *filename)
{
    int ret;
    FILE * fp;

    struct file_head_node head_node;
    struct file_head_node tmp_head_node;

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret  = file_ops_read_head(fp, &head_node);
    if (ret < 0)
        return ret;

    ret = truncate(filename, 0);
    if (ret < 0) {
        return FAIL;
    }

    fclose(fp);
    
    /* rewrite the file head */
    memset(&tmp_head_node, 0, sizeof(tmp_head_node));
    memcpy(tmp_head_node.name, filename, strlen(filename) + 1);
    tmp_head_node.mode = head_node.mode;
    tmp_head_node.max_record = head_node.max_record;

#ifdef CONFIG_LIMIT_DEL
    tmp_head_node.max_del_record = head_node.max_del_record;
#endif 

    fp = fopen(filename, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    ret = file_ops_write_head(fp, &tmp_head_node);
    if (ret < 0)
        return ret;

    fclose(fp);

    return SUCCESS;
}

/* READ FILE RECORD */
static int file_ops_read_record(FILE *stream, int offset, struct file_record_node  *record_buf)
{
    int ret;
    int size;
    long pos_offset;

    struct file_head_node head_node;
    ret = file_ops_read_head(stream, &head_node);
    if (ret < 0)
        return ret;

    if (offset > head_node.record_total_count)
        return -EFILE_OFFSET_OVER_FLOW;
    
    size = sizeof(struct file_record_node);
    pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (offset - 1);

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    ret = fread(record_buf, size, 1, stream);
    if (ret != 1) {
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_RD_FAIL;
    }

    return SUCCESS;
}

/* Random Write a record */
static int file_ops_write_record(FILE *stream, int offset, struct file_record_node  *record_node)
{
#if 0
    int ret;
    int size;
    long pos_offset;
	
	int cur_add_node;
	int prev_vaild_node;
	
    struct file_head_node head_node;
    struct file_record_node prev_record_node;

    size = sizeof(struct file_record_node);

    ret = file_ops_read_head(stream, &head_node);
    if (ret < 0)	
        return ret;
        
	if (offset > head_node.record_total_count) 
        return -EFILE_OFFSET_OVER_FLOW;
     
	if (head_node.max_record == head_node.record_total_count)
        return -EFILE_REC_FULL;

    pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (offset - 1);

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    ret = fread(&prev_record_node, size, 1, stream);
    if (ret ! = 1){
		if (feof(stream) != 0)
			return -EFILE_EOF;

		if (ferror(stream) != 0)
			return -EFILE_WR_FAIL;
	}
	
    /* update record information */ 
    record_node->flag = IN_USING;
    
    if (prev_record_node.flag != DELETED) {
        record_node->prev_del_record = prev_record_node.prev_del_record;
        record_node->prev_vaild_record = head_node.record_total_count;
	} else {
		record_node->prev_vaild_record = prev_record_node.prev_vaild_record; 
        record_node->prev_del_record = head_node.record_total_count;
	}
        
    cur_add_node = head_node.record_total_count + 1;
    prev_vaild_node = prev_record_node.prev_vaild_record;
        
    if (prev_vaild_node == 0)
		prev_vaild_node = 1;
			
	while (prev_vaild_node < cur_add_node) {
		pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_vaild_node - 1);
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}

		ret = fread(&prev_record_node, size, 1, stream);
		if (ret != 1) {
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_WR_FAIL;
		}
		
		pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_vaild_node - 1);
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}

		prev_record_node.next_vaild_record = cur_add_node;
		ret = fwrite(&prev_record_node, size, 1, stream);
		if (ret != 1) {
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_RD_FAIL;
		}

		prev_vaild_node ++;
	}

    ret = fseek(stream, 0, SEEK_END);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    ret = fwrite((void *)record_node, size, 1, stream);
    if (ret != 1) {
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_RD_FAIL;
    }

    /* update file head node */
    head_node.record_total_count ++;
    head_node.record_vaild_count ++;
    
    if (record_node->prev_vaild_record == 0)
		head_node.first_vaild_record = cur_add_node;
	
    head_node.last_vaild_record = cur_add_node;
	
    ret = file_ops_write_head(stream, &head_node);
    if (ret < 0)
        return ret;

#endif 
    return SUCCESS;
}

/* 
 * APPEND RECORD 
 *  this method will not check file head node, just append a record 
 *  and raise the file head record. useit carefuly.
 */
static int file_ops_append_record(FILE *stream, struct file_record_node *record_node)
{
    int ret;
    int size;
    int offset;
    long pos_offset;
	
	int cur_add_node;
	int prev_vaild_node;
	
    struct file_head_node head_node;
    struct file_record_node prev_record_node;

    size = sizeof(struct file_record_node);
    
    memset(&head_node, 0, sizeof(head_node));
    memset(&prev_record_node, 0, sizeof(prev_record_node));

    ret = file_ops_read_head(stream, &head_node);
    if (ret < 0) {
		return ret;
    }
	
	/* new file */
	if (head_node.record_total_count == 0) {
		record_node->flag = IN_USING;
		record_node->prev_vaild_record = 0;
		record_node->prev_del_record = 0;
		record_node->next_vaild_record = 0;
		record_node->next_del_record = 0;
		
		cur_add_node = 1;
		goto write_data;
	}
	
	if (head_node.max_record == head_node.record_total_count)
        return -EFILE_REC_FULL;

    offset = head_node.record_total_count;
    pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (offset - 1);

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    ret = fread(&prev_record_node, size, 1, stream);
    if (ret != 1){
		if (feof(stream) != 0)
			return -EFILE_EOF;

		if (ferror(stream) != 0)
			return -EFILE_WR_FAIL;
	}	
	
    /* update record information */ 
    record_node->flag = IN_USING;
    record_node->next_vaild_record = 0; 
    record_node->next_del_record = 0;
    
    if (prev_record_node.flag != DELETED) {
        record_node->prev_del_record = prev_record_node.prev_del_record;
        record_node->prev_vaild_record = head_node.record_total_count;
	} else {
		record_node->prev_vaild_record = prev_record_node.prev_vaild_record; 
        record_node->prev_del_record = head_node.record_total_count;
	}
        
    cur_add_node = head_node.record_total_count + 1;
    prev_vaild_node = record_node->prev_vaild_record;
        
    if (prev_vaild_node == 0)
		prev_vaild_node = 1;
			
	while (prev_vaild_node < cur_add_node) {
		pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_vaild_node - 1);
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}

		ret = fread(&prev_record_node, size, 1, stream);
		if (ret != 1) {
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_WR_FAIL;
		}
		
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}

		prev_record_node.next_vaild_record = cur_add_node;
		ret = fwrite(&prev_record_node, size, 1, stream);
		if (ret != 1){
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_RD_FAIL;
		}
		prev_vaild_node ++;
	}

write_data:
    ret = fseek(stream, 0, SEEK_END);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    ret = fwrite((void *)record_node, size, 1, stream);
    if (ret != 1) {
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_RD_FAIL;
    }

    /* update file head node */
    head_node.record_total_count ++;
    head_node.record_vaild_count ++;
    
    if (record_node->prev_vaild_record == 0)
		head_node.first_vaild_record = cur_add_node;
	
    head_node.last_vaild_record = cur_add_node;
	
    ret = file_ops_write_head(stream, &head_node);
    if (ret < 0)
        return ret;

    return SUCCESS;
}

/* MODIFY RECORD */
static int file_ops_modify_record(FILE *stream, int offset, struct file_record_node *record_buf)
{
    int ret;
    int size;
    long pos_offset = 0;
    
    int cur_del_node;
    int prev_vaild_node;
    int next_vaild_node;
    int prev_del_node;
    int next_del_node;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_record_node prev_record_node;
    struct file_record_node next_record_node;
    
    memset(&head_node, 0, sizeof(head_node));
    memset(&record_node, 0, sizeof(record_node));
    memset(&prev_record_node, 0, sizeof(prev_record_node));
    memset(&next_record_node, 0, sizeof(next_record_node));
     
    ret = file_ops_read_head(stream, &head_node);
    if (ret < 0)
        return ret;

    if (offset > head_node.record_total_count) {
        return -EFILE_OFFSET_OVER_FLOW;
    }

    size = sizeof(struct file_record_node);
    pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (offset - 1);

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }
	
	/* check if the record is a deleted record */
	ret = fread(&record_node, size, 1, stream);
	if (ret != 1) {
		if (feof(stream) != 0)
			return -EFILE_EOF;

		if (ferror(stream) != 0)
			return -EFILE_WR_FAIL;
	}
    
    memcpy(record_node.data, record_buf->data, MAX_RECORD_SIZE);

	if (record_node.flag != DELETED){
		/* normal record */
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}
	    
		ret = fwrite(&record_node, size, 1, stream);
		if (ret != 1) {
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_WR_FAIL;
		}
	
		return SUCCESS;	
	} else {
		/* reused a deleted record */
		ret = fseek(stream, pos_offset, SEEK_SET);
		if (ret < 0) {
			return -EFILE_SEEK_FAIL;
		}
		
		record_node.flag = IN_USING;
		ret = fwrite(&record_node, size, 1, stream);
		if (ret != 1) {
			if (feof(stream) != 0)
				return -EFILE_EOF;

			if (ferror(stream) != 0)
				return -EFILE_WR_FAIL;
		}

        /* update other node's vaild node info */
        cur_del_node = offset;
        prev_vaild_node = record_node.prev_vaild_record;
        next_vaild_node = record_node.next_vaild_record;

        if (prev_vaild_node == 0)
            prev_vaild_node = 1;

        if (next_vaild_node == 0)
            next_vaild_node = head_node.record_total_count;

        /* update prev_vaild_node to current node  */
        while (prev_vaild_node < cur_del_node) {
            pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_vaild_node - 1);
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            ret = fread(&prev_record_node, size, 1, stream);
            if (ret != 1) {
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            prev_record_node.next_vaild_record = offset;
            ret = fwrite(&prev_record_node, size, 1, stream);
            if (ret != 1){
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            prev_vaild_node ++;
        }

        /* update current to next_vaild_node */
        cur_del_node += 1;
        while (cur_del_node <= next_vaild_node) {
            pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (cur_del_node - 1);
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            ret = fread(&next_record_node, size, 1, stream);
            if (ret != 1){
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            next_record_node.prev_vaild_record = offset;
            ret = fwrite(&prev_record_node, size, 1, stream);
            if (ret != 1){
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            cur_del_node ++;
        }

        /* update other node's del_node_msg */
        cur_del_node = offset;
        prev_del_node = record_node.prev_del_record;
        next_del_node = record_node.next_del_record;

        if (prev_del_node == 0)
            prev_del_node = 1;

        if (next_del_node == 0)
            next_del_node = head_node.record_total_count;

        /* update prev_del_node to current node  */
        while (prev_del_node < cur_del_node) {
            pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_del_node - 1);
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            ret = fread(&prev_record_node, size, 1, stream);
            if (ret != 1) {
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }
            
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            prev_record_node.next_del_record = record_node.next_del_record;
            ret = fwrite(&prev_record_node, size, 1, stream);
            if (ret != 1){
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            prev_del_node ++;
        }

        /* update current to next_del_node */
        cur_del_node += 1;
        while (cur_del_node <= next_del_node) {
            pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (cur_del_node - 1);
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            ret = fread(&next_record_node, size, 1, stream);
            if (ret != 1) {
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }
            pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (cur_del_node - 1);
            ret = fseek(stream, pos_offset, SEEK_SET);
            if (ret < 0) {
                return -EFILE_SEEK_FAIL;
            }

            next_record_node.prev_del_record = record_node.next_del_record;
            ret = fwrite(&prev_record_node, size, 1, stream);
            if (ret != 1) {
                if (feof(stream) != 0)
                    return -EFILE_EOF;

                if (ferror(stream) != 0)
                    return -EFILE_WR_FAIL;
            }

            cur_del_node ++;
        }

        /* update head at the end */
        head_node.record_vaild_count ++;
        head_node.record_del_count --;

        /* is first del record */
        if (record_node.prev_del_record == 0) {
            head_node.first_del_record = record_node.next_del_record;
        }

        /* is last del record */
        if (record_node.next_del_record == 0) {
            head_node.last_del_record = record_node.prev_del_record;
        }

        /* is first vaild record */
        if (record_node.prev_vaild_record == 0)
            head_node.first_vaild_record = offset;

        /* is last vaild record */
        if (record_node.next_vaild_record == 0)
            head_node.last_vaild_record = offset;

        ret = file_ops_write_head(stream, &head_node);
        if (ret < 0)
            return ret;

        return SUCCESS;	
    }
}

/**
 * DELETE RECORD - delete the record lacated in offset position 
 */
static int file_ops_del_record(FILE *stream, int offset)
{
    int ret;
    int size;	
    long pos_offset;

    int cur_del_node;
    int prev_vaild_node;
    int next_vaild_node;
    int prev_del_node;
    int next_del_node;

    struct file_head_node head_node;
    struct file_record_node record_node;
    struct file_record_node prev_record_node;
    struct file_record_node next_record_node;

    ret = file_ops_read_head(stream, &head_node);
    if (ret < 0)
        return ret;

    if (head_node.mode != DELETE_AVAIL_MODE)
        return -EFILE_DEL_UNAVAIL;

#ifdef CONFIG_LIMIT_DEL
    if (head_node.record_del_count == max_del_record)
        return -EFILE_FULL_DEL_RECORD;
#endif 

    if (head_node.record_del_count == head_node.record_total_count)
        return -EFILE_REC_HAS_DEL;

    if (offset > head_node.record_total_count) {
        return -EFILE_OFFSET_OVER_FLOW;
    }

    size = sizeof(struct file_record_node);
    pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (offset - 1);

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    memset(&prev_record_node, 0, size);
    ret = fread(&record_node, size, 1, stream);
    if (ret != 1) {
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_WR_FAIL;
    }

    if (record_node.flag == DELETED)
        return -EFILE_REC_HAS_DEL;

    ret = fseek(stream, pos_offset, SEEK_SET);
    if (ret < 0) {
        return -EFILE_SEEK_FAIL;
    }

    record_node.flag = DELETED;
    ret = fwrite(&record_node, size, 1, stream);
    if (ret != 1){
        if (feof(stream) != 0)
            return -EFILE_EOF;

        if (ferror(stream) != 0)
            return -EFILE_WR_FAIL;
    }

    /* update other node's vld_node_msg */
    cur_del_node = offset;
    prev_vaild_node = record_node.prev_vaild_record;
    next_vaild_node = record_node.next_vaild_record;

    if (prev_vaild_node == 0)
        prev_vaild_node = 1;

    if (next_vaild_node == 0)
        next_vaild_node = head_node.record_total_count;

    /* update prev_vaild_node to current node  */
    while (prev_vaild_node < cur_del_node) {
        pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_vaild_node - 1);
        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        memset(&prev_record_node, 0, size);
        ret = fread(&prev_record_node, size, 1, stream);
        if (ret != 1) {
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        prev_record_node.next_vaild_record = record_node.next_vaild_record;
        ret = fwrite(&prev_record_node, size, 1, stream);
        if (ret != 1){
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        prev_vaild_node ++;
    }

    /* update current to next_vaild_node */
    cur_del_node += 1;
    while (cur_del_node <= next_vaild_node) {
        pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (cur_del_node - 1);
        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        memset(&next_record_node, 0, size);
        ret = fread(&next_record_node, size, 1, stream);
        if (ret != 1) {
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        next_record_node.prev_vaild_record = record_node.prev_vaild_record;
        ret = fwrite(&next_record_node, size, 1, stream);
        if (ret != 1){
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        cur_del_node ++;
    }

    /* update other node's del_node_msg */
    cur_del_node = offset;
    prev_del_node = record_node.prev_del_record;
    next_del_node = record_node.next_del_record;

    if (prev_del_node == 0)
        prev_del_node = 1;

    if (next_del_node == 0)
        next_del_node = head_node.record_total_count;

    /* update prev_del_node to current node  */
    while (prev_del_node < cur_del_node) {
        pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (prev_del_node - 1);
        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        memset(&prev_record_node, 0, size);
        ret = fread(&prev_record_node, size, 1, stream);
        if (ret != 1) {
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        prev_record_node.next_del_record = offset;
        ret = fwrite(&prev_record_node, size, 1, stream);
        if (ret != 1){
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        prev_del_node ++;
    }

    /* update current to next_del_node */
    cur_del_node += 1;
    while (cur_del_node <= next_del_node) {
        pos_offset = sizeof(struct file_head_node) + sizeof(struct file_record_node) * (cur_del_node - 1);
        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        memset(&next_record_node, 0, size);
        ret = fread(&next_record_node, size, 1, stream);
        if (ret != 1) {
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        ret = fseek(stream, pos_offset, SEEK_SET);
        if (ret < 0) {
            return -EFILE_SEEK_FAIL;
        }

        next_record_node.prev_del_record = offset;
        ret = fwrite(&next_record_node, size, 1, stream);
        if (ret != 1){
            if (feof(stream) != 0)
                return -EFILE_EOF;

            if (ferror(stream) != 0)
                return -EFILE_WR_FAIL;
        }

        cur_del_node ++;
    }

    /* update head at the end */
    head_node.record_vaild_count --;
    head_node.record_del_count ++;

    /* is first vaild record */
    if (record_node.prev_vaild_record == 0)
        head_node.first_vaild_record = record_node.next_vaild_record;

    /* is last vaild record */
    if (record_node.next_vaild_record == 0)
        head_node.last_vaild_record = record_node.prev_vaild_record;

    /* is first del record */	
    if (record_node.prev_del_record == 0) {
        head_node.first_del_record = offset;
    }

    /* is last del record */
    if (record_node.next_del_record == 0) {
        head_node.last_del_record = offset;
    }

    ret = file_ops_write_head(stream, &head_node);
    if (ret < 0)
        return ret;
    
    sync();

    return SUCCESS;
}

struct file_operate g_file_ops = {
    .creat_dir = file_ops_creat_dir,
    .remove_dir = file_ops_del_dir,
    .remove_dir_f = file_ops_force_del_dir,

    .creat_file = file_ops_creat_file,
    .delete_file = file_ops_del_file,

    .open_file = file_ops_open_file,
    .close_file = file_ops_close_file,

    .read_head = file_ops_read_head,
    .read_record = file_ops_read_record,
    .clear_record = file_ops_clear_record,
    .write_head = file_ops_write_head,
    .append_record = file_ops_append_record,
    .modify_record = file_ops_modify_record,
    .delete_record = file_ops_del_record,
};

/* API for other modules */
struct file_operate * get_file_ops(void)
{
    return &g_file_ops;
}


/* 
 * following is test function 
 */
#if 1
int err_num = 0;

#if 0
int main(void)
{
    int ret;
    FILE *fp;

    char * dir = "target_tst/";
    char * file = "./target_tst/tst_file";

    struct file_head_node head_node;
    struct file_record_node record_node;

    ret = file_ops_creat_dir(dir);
    if (ret < 0) {
        printf("creat dir failed\n");
        return 0;
    }

    ret = file_ops_creat_file(file, 1, 10);
    if (ret < 0) {
        printf("creat file failed\n");
        return 0;
    }

    fp = fopen(file, "rb+");
    if (fp == NULL)
        return -EFILE_OPEN_FAIL;

    memset(&head_node, 0, sizeof(head_node));
    ret = file_ops_read_head(fp, &head_node);
    if (ret < 0) {
        printf("read head failed\n");
        return 0;
    }

    printf("head_node.name is %s\n", head_node.name);
    printf("head_node.mode is %d\n", head_node.mode);
    printf("head_mode.max_record is %d\n", head_node.max_record);
    printf("head_node.record total count is %d\n", head_node.record_total_count);
    printf("head_mode.record vaild count is %d\n", head_node.record_vaild_count);
    printf("head_node.record del count is %d\n", head_node.record_del_count);

    printf("\nAppend record 1,2...\n");

    memset(&record_node, 0, sizeof(record_node));
    sprintf(record_node.data, "%s", "abcdedf");

    ret = file_ops_append_record(fp, &record_node);
    if (ret < 0) {
        printf("append record failed\n");
        return 0;
    }

    memset(&record_node, 0, sizeof(record_node));
    sprintf(record_node.data, "%s", "ABCDEDF");

    ret = file_ops_append_record(fp, &record_node);
    if (ret < 0) {
        printf("append record failed\n");
        return 0;
    }

    memset(&head_node, 0, sizeof(head_node));
    ret = file_ops_read_head(fp, &head_node);
    if (ret < 0) {
        printf("read head failed\n");
        return 0;
    }

    printf("\nhead_node.record total count is %d\n", head_node.record_total_count);
    printf("head_mode.record vaild count is %d\n", head_node.record_vaild_count);
    printf("head_node.record del count is %d\n", head_node.record_del_count);

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 1, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 1...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);


    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 2, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 2...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);

    printf("\nModify Record 1...\n");
    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 1, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    sprintf(record_node.data, "%s", "leonardo physh");

    ret = file_ops_modify_record(fp, 1, &record_node);
    if (ret < 0) {
        printf("modify record failed\n");
        return 0;
    }

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 1, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 1...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 2, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 2...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);

    printf("\nDeleted Record 1...\n\n");

    ret = file_ops_del_record(fp, 1);
    if (ret < 0) {
        printf("delelte record failed\n");
        return 0;
    }

    memset(&head_node, 0, sizeof(head_node));
    ret = file_ops_read_head(fp, &head_node);
    if (ret < 0) {
        printf("read head failed\n");
        return 0;
    }

    printf("head_node.first vaild record is %d\n", head_node.first_vaild_record);
    printf("head_node.last vaild record is %d\n", head_node.last_vaild_record);
    printf("head_mode.first del record %d\n", head_node.first_del_record);
    printf("head_mode.last del record %d\n", head_node.last_del_record);

    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 1, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 1...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);


    memset(&record_node, 0, sizeof(record_node));
    ret = file_ops_read_record(fp, 2, &record_node);
    if (ret < 0) {
        printf("read record failed\n");
        return 0;
    }

    printf("\nRecord 2...\n");
    printf("record.flag is %d\n", record_node.flag);
    printf("record.prev vld rec is %d\n", record_node.prev_vaild_record);
    printf("record.next vld rec is %d\n", record_node.next_vaild_record);
    printf("record.prev del rec is %d\n", record_node.prev_del_record);
    printf("record.next del rec is %d\n", record_node.next_del_record);
    printf("record.data is %s\n", record_node.data);

    memset(&head_node, 0, sizeof(head_node));
    ret = file_ops_read_head(fp, &head_node);
    if (ret < 0) {
        printf("read head failed\n");
        return 0;
    }

    printf("\nhead_node.record total count is %d\n", head_node.record_total_count);
    printf("head_node.record vaild count is %d\n", head_node.record_vaild_count);
    printf("head_node.record del count is %d\n", head_node.record_del_count);

    fclose(fp);

    printf("\nClear Record...\n\n");

    file_ops_clear_record(file);

    fopen(file, "rb+");

    memset(&head_node, 0, sizeof(head_node));
    ret = file_ops_read_head(fp, &head_node);
    if (ret < 0) {
        printf("read head failed\n");
        return 0;
    }

    printf("head_node.record total count is %d\n", head_node.record_total_count);
    printf("head_mode.record vaild count is %d\n", head_node.record_vaild_count);

    fclose(fp);

    return 0;
}
#endif

int main(void)
{
    int ret;

    printf("Creating...\n");
    
    ret = file_ops_creat_file(PROTECT_FILE, PROTECT_FILE_MODE, PROTECT_REC_NUM);
    if (ret < 0)
        goto fail;

    ret = file_ops_creat_file(DIST_ROLL_FILE, DIST_ROLL_FILE_MODE, DIST_ROLL_REC_NUM);
    if (ret < 0)
        goto fail;
    
    ret = file_ops_creat_file(LAST_DIST_FILE, LAST_DIST_FILE_MODE, LAST_DIST_REC_NUM);
    if (ret < 0)
        goto fail;
    
    printf("Done!\n");
    return ret;

fail:
    printf("Creat file fail!\n");
    return ret;
}
#endif


