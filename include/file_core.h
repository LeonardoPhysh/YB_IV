/*
 * Head file of file operation core
 *  the main callback functions of  file operation core
 * 
 * Auhor : Leonardo Physh <Leonardo.physh@yahoo.com.hk> 
 * Date  : 2014.8.12
 */ 
 
#ifndef __FILE_CORE_HEAD__
#define __FILE_CORE_HEAD__
 
#include <stdio.h>
 
#ifdef CONFIG_MAX_REC_SIZE
#define MAX_RECORD_SIZE     CONFIG_MAX_REC_SIZE
#else
#define MAX_RECORD_SIZE 	1010
#endif  

#define MAX_FILE_NAME_SIZE  48
#define MAX_RESERVED_SIZE   12
#define MAX_DEL_RECORD_NUM 	300

/* FILE HEAD NOTE */
#define file_head_node_SIZE   (sizeof(struct file_head_node))
struct file_head_node
{
    char name[MAX_FILE_NAME_SIZE];
    
#define DELETE_AVAIL_MODE    1
#define DELETE_UNAVAIL_MODE  0
    int mode;
 
    int max_record;
#ifdef CONFIG_LIMIT_DEL
    int max_del_record;
#endif

    int record_total_count;
    int record_vaild_count;
    int record_del_count;
   
    /* just for delete available mode */
    int first_vaild_record; 
    int last_vaild_record;

    int first_del_record;
    int last_del_record;

    uchar reserved[MAX_RESERVED_SIZE];
};

/* FILE RECORD NOTE */
#define FILE_NOTE_SIZE  (sizeof(struct file_record_node))
struct file_record_node
{
#define IN_USING  1
#define DELETED   0
    int flag;    

    int prev_vaild_record;
    int next_vaild_record;

    int prev_del_record;
    int next_del_record;

    /* detail record date */
    uchar data[MAX_RECORD_SIZE];
};

typedef int (*CREAT_DIR)(const char *path);
typedef int (*CREAT_FILE)(const char *filename, int mode, int record_num);
typedef int (*DEL_FILE)(const char *filename);
typedef FILE * (*OPEN_FILE)(const char *filename);
typedef int (*CLOSE_FILE)(FILE *stream);
typedef int (*READ_HEAD)(FILE * stream, struct file_head_node *head_note);
typedef int (*FILE_READ_RECORD)(FILE * stream, int offset, struct file_record_node *record_buf);
typedef int (*CLEAR_RECORD)(const char *filename);
typedef int (*WRITE_HEAD)(FILE * stream, struct file_head_node *head_noete);
typedef int (*APPEND_RECORD)(FILE * stream, struct file_record_node *record_buf);
typedef int (*MODIFY_RECORD)(FILE * stream, int offset, struct file_record_node * record_buf);
typedef int (*DEL_RECORD)(FILE * stream, int offset);

struct file_operate
{
	CREAT_DIR 	creat_dir;
    CREAT_DIR   remove_dir;
    CREAT_DIR   remove_dir_f;

	CREAT_FILE  creat_file;
    DEL_FILE    delete_file;

	OPEN_FILE 	open_file;
    CLOSE_FILE  close_file;

	READ_HEAD 	read_head;
	FILE_READ_RECORD read_record;
	CLEAR_RECORD clear_record;
	WRITE_HEAD  write_head;
	APPEND_RECORD append_record;
	MODIFY_RECORD modify_record;
	DEL_RECORD delete_record;
};

extern struct file_operate * get_file_ops(void);

#endif 	/* __FILE_CORE_HEAD__  */
