/*
 * lcm-api.c - The user interface to show info on JLX19264C
 * Updata time: 2014-10-15
 * Copyright VIA 2014
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
 * lcm_open - Open the lcm device 
 * 
 * @return : fd
 */
int lcm_open(void)
{
    int fd = 0;
    fd = open(LCM_DEVICE, O_RDWR);
    
	if (fd < 0) {
        return FAIL;
    }

    return fd;
}

/*
 * create_data - generate lcm_data structure with paraments
 *  
 *  @return : status
 */
int create_data(int y, int x, int size, char *str, struct lcm_data *datap)
{
    struct lcm_data *dp = datap;
    
    dp->y = y;
    dp->x = x;
    dp->size = size;
    
	if (str != NULL)
        strcpy(dp->info, str);
    else 
        memset(dp->info, 0, sizeof(dp->info));
    
	return SUCCESS;
}

/**
 * lcm_clear - clear screen 
 *
 *  @return : status 
 */
int lcm_clear(void) 
{
    int fd = 0;
    int ret = 0;
    
	fd = lcm_open();
	if (fd < 0) {
		debug_msg("LCM : Open Failed.\n");
		return FAIL;
	}
    
	ret = ioctl(fd, LCM_CLEAR_CMD);
	if (ret < 0) {
		debug_msg("LCM : ioctl failed.\n");
		goto fail;
	}

    ret = SUCCESS;

fail:
    close(fd);
      
	return ret;
}

/*
 * lcm_charset - set lcm character set with CHN or ENG
 *  @set : character set 
 *  @return : status
 */
int lcm_charset(char set)
{
    int fd = 0;
    int ret = 0;
   
	fd = lcm_open();
	if (fd < 0) {
		debug_msg("LCM: open failed.\n");
		return FAIL;
	}

	switch (set) {
	case 'C':
		ret = ioctl(fd, LCM_CHAR_CHN);
		if (ret < 0) {
			debug_msg("LCM : ioctl failed.");
			goto fail;
		}
		break;

	case 'E':
		ret = ioctl(fd, LCM_CHAR_ENG);
		if (ret < 0) {
			debug_msg("LCM : ioctl failed");
			goto fail;
		}

		break;

	default:
		goto fail;
		break;
	}

    ret = SUCCESS;

fail:
	close(fd);

	return ret;
}

/*
 * lcm_set_cursor - ON/OFF the cursor
 *  @flag : ON/OFF
 *  @return : status 
 */
int lcm_set_cursor(int row, int col, int flag)
{
	int fd = 0;
	int ret = 0;
	
	fd = lcm_open();
	if (fd < 0) {
		debug_msg("LCM : open failed");
		return FAIL;
	}

	if (flag == ON) {
        if (row == 1 || row == 2) {
            ret = ioctl(fd, LCM_CURSOR_ON_IC1);
            if (ret < 0) {
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }
        } else {
            ret = ioctl(fd, LCM_CURSOR_ON_IC2);
            if (ret < 0) {
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }
        }
    } else if (flag == OFF) {
        if (row == 1 || row == 2) {
            ret = ioctl(fd, LCM_CURSOR_OFF_IC1);
            if (ret < 0) {
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }
        } else {
            ret = ioctl(fd, LCM_CURSOR_OFF_IC2);
            if (ret < 0) {
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }
        }
    } else {
        ret = FAIL;
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
    int fd = 0;
    int ret = 0;
    size_t count = 0;

    fd = lcm_open();
    if (fd < 0) {
        debug_msg("LCM : Open failed.");
        goto fail;
    }

    count = sizeof(struct lcm_data);
    ret = write(fd, (void *)datap, count);
    if (ret < 0 ) {
        debug_msg("LCM : write failed.\n");
        goto fail;
    }

    ret = SUCCESS;

fail:	
    close(fd);
    return ret;
}

#if 0
/*
 * __lcm_printf
 * show str at (col,row) on lcm
 *
 * NOTICE:the func won't save any info,
 * so save_screen() and resume_screen()
 * are not useful for the func.
 * Therefore, it is used alone,
 * or used to print err/warn info.
 */
int __lcm_printf(int row, int col, char *str)
{
    int size = 0;
    struct lcm_data data;
    int ret = 0;

    size = strlen(str);
    if (size > 24)
        size = 24;
    if (size < 0)
        size = 0;
    create_data(row, col, size, str, &data);

    ret = lcm_write(&data);
    screen_info[row - 1].row = row;
    return ret;
}
#endif 

/*
 * lcm_printf - show str at (col,row) on lcm
 *  @row, col: position 
 *  @str : content 
 *  @return : status
 */
int lcm_printf(int row, int col, char *str)
{
    int ret = 0;
    int size = 0;
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
        return FAIL;

    return SUCCESS;
}

#if 0
/*
 * save_screen
 * save all info that was showed on lcm.
 * In fact info has been saved, when call lcm_printf.
 * There should be take flag=1, means effective.
 */
int save_screen(void)
{
    int n;
    for (n = 0; n < 4; n++)
        screen_info[n].flag = 1;
    return 0;
}


/*
 * resume_screen
 * print again the screen that was saved.
 */
int resume_screen()
{
    int n;
    for (n = 0; n < 4; n++) {
        if (screen_info[n].flag == 1)
            lcm_printf(screen_info[n].row, screen_info[n].col, screen_info[n].buff);
        screen_info[n].flag = 0;
    }
    return 0;
}

#endif 

/*
 * lcm_rev_cursor - set cursor and reverse it
 *  @row, col : position
 *  @return : status 
 */
int lcm_rev_cursor(int row, int col)
{
    int fd = 0;
    int ret = 0;

    if (row != 0 && col != 0)
        lcm_printf(row, col, NULL);

    fd = lcm_open();
    if (fd < 0) {
        debug_msg("LCM : open failed.\n");
        return FAIL;
    }

    ret = ioctl(fd, LCM_REV_CURSOR);
    if (ret < 0) {
        debug_msg("LCM : iocal failed");
        goto fail;
    }

    ret = SUCCESS;

fail:
    close(fd);

    return ret;
}

/*
 * lcm_rev_line - reverse a line
 *  @num : lien 
 */
int lcm_rev_line(int row)
{
    int fd = 0;
    int ret = 0;

    lcm_printf(row, 1, NULL);

    fd = lcm_open();
    if (fd < 0) {
        debug_msg("LCM : open failed.\n");
        return FAIL;
    }

    switch (row) {
        case 1:
            ret = ioctl(fd, LCM_REV_LINE_1);
            if (ret < 0){
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }

            break;

        case 2:
            ret = ioctl(fd, LCM_REV_LINE_2);
            if (ret < 0){
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }

            break;

        case 3:
            ret = ioctl(fd, LCM_REV_LINE_3);
            if (ret < 0){
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }

            break;

        case 4:
            ret = ioctl(fd, LCM_REV_LINE_4);
            if (ret < 0){
                debug_msg("LCM : ioctl failed.\n");
                goto fail;
            }

            break;

        default:
            goto fail;
            break;
    }

    ret = SUCCESS;

fail:
    close(fd);
    return ret;
}

#if 1
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
    /*
    lcm_printf(1, 1, "A主界面B");
    lcm_printf(1, 6, "A B C D");
    lcm_printf(3, 1, "AB CD");
    lcm_printf(3, 6, "主A界B面C"); 
    */

    lcm_printf(3, 4, "YES");
    lcm_printf(3, 8, "NO");
    
    lcm_rev_cursor(3, 4);

    return 0;
}

#endif 
