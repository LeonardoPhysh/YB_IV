/*
 * lcm-api.c 
 *  The APIs for ui_api to operate JLX19264C.
 *
 * Author : Leonardo Physh & Aningsk.Ning
 * Date   : 2014-10-15
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "common.h"
#include "lcm_api.h"

/*
 * lcm_open - Open the LCM device  
 *  @return : fd
 */
int lcm_open(void)
{
    int fd;

    fd = open(LCM_DEVICE, O_RDWR); 
	if (fd < 0) { 
        debug_msg("LCM : Open failed.");
        return -ELCM_OPEN_FAIL;
    }

    return fd;
}

/*
 * create_data - generate lcm_data structure with paraments
 *  @row, col: position
 *  @size : date size 
 *  @datap : data buffer 
 *  @return : state
 */
int create_data(int row, int col, int size, char *str, struct lcm_data *datap)
{
    struct lcm_data *dp = datap;
    
    dp->row = row;
    dp->col = col;
    dp->size = size;
    
	if (str != NULL)
        strcpy(dp->info, str);
    else 
        memset(dp->info, 0, sizeof(dp->info));
    
	return SUCCESS;
}

/*
 * lcm_clear - clear screen 
 *  @return : state 
 */
int lcm_clear(void) 
{
    int fd, ret;
    
	fd = lcm_open();
	if (fd < 0) {
		return fd;
	}
    
	ret = ioctl(fd, LCM_CLEAR_CMD);
	if (ret < 0) {
		debug_msg("LCM : ioctl failed.\n");
		ret = -ELCM_IOCTL_FAIL;
        goto fail;
	}

    ret = SUCCESS;
fail:
    close(fd);
	return ret;
}

/*
 * lcm_write - get user data and write to lcm19264
 *  @datap : data pointer
 *  @return : status 
 */
int lcm_write(struct lcm_data *datap)
{
    int fd, ret, count;

    fd = lcm_open();
    if (fd < 0) {
        return fd;
    }

    count = sizeof(struct lcm_data);
    ret = write(fd, (void *)datap, count);
    if (ret < 0 ) {
        debug_msg("LCM : write failed.\n");
        ret = -ELCM_WR_FAIL;
        goto fail;
    }

    ret = SUCCESS;
fail:	
    close(fd);
    return ret;
}


/*
 * lcm_set_cursor - ON/OFF the cursor
 *  @row, col: position to set cursor 
 *  @cursor : cursor ON/OFF flag 
 *  @blink  : blink ON/OFF flag 
 *  @return : status 
 */
int lcm_set_cursor(int row, int col, int cursor, int blink)
{
	int ret;
	int fd, args = 0;

	fd = lcm_open();
	if (fd < 0) {
		return fd;
	}
    
    args |= (cursor & 0x01) << 10;
    args |= (blink & 0x01) << 9;
    args |= (row & 0xff) << 8;
    args |= (col & 0xff);

    ret = ioctl(fd, LCM_CURSOR_CMD, args);
    if (ret < 0) {
        ret = -ELCM_IOCTL_FAIL;
        goto fail;
    }
    
    ret = SUCCESS;
fail:
    close(fd);
    return ret;
}

/*
 * lcm_rev_line - reverse a line
 *  @row : line to reverse 
 */
int lcm_rev_line(int row)
{
    int ret;
    int fd, args;

    fd = lcm_open();
    if (fd < 0) {
        return fd;
    }
    
    args = row;
    ret = ioctl(fd, LCM_REVERSE_CMD, args);
    if (ret < 0) {
        ret = -ELCM_IOCTL_FAIL;
        goto fail;
    }
    
    ret = SUCCESS;
fail:
    close(fd);
    return ret;
}


/*
 * lcm_printf - show str at (col,row) on lcm
 *  @row, col: position 
 *  @str : content 
 *  @return : status
 */
int lcm_printf(int row, int col, char *str)
{
    int ret;
    int size;
    struct lcm_data data;
    
    memset(&data, 0, sizeof(data));

    if (str != NULL) {
        size = strlen(str);
        if (size > 24)
            size = 24;
        if (size < 0)
            size = 0;
    } else 
        size = 0;

    create_data(row, col, size, str, &data);

    ret = lcm_write(&data);
    if (ret != SUCCESS)	
        return ret;

    return SUCCESS;
}


#if 0
int main(void)
{
    int fd = 0;
    int ret = 0;

#if 0
    fd = lcm_open();
    if (fd < 0) {
        debug_msg("LCM : open failed.\n");
        return FAIL;
    }

    while (1) {
        ret = ioctl(fd, LCM_DB5_L);
        if (ret < 0){
            debug_msg("LCM : ioctl failed.\n");
        }

        usleep(10000);

        ret = ioctl(fd, LCM_DB5_H);
        if (ret < 0) {
            debug_msg("LCM : ioctl failed.\n");
        }
    }
#endif

    lcm_printf(1, 1, "ABCD");
    lcm_printf(1, 6, "ABCD");
    lcm_printf(3, 1, "ABCD");
    lcm_printf(3, 6, "ABCD"); 

    return 0;
}

#endif

