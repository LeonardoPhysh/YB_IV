/*
 * error.c - provice error number operate functions
 *
 * Author : Leonardo Physh 
 * Date   : 2014.10.15 
 */

#include "error.h"

/*
 * global err_num 
 */
int err_num = 0;

/*
 *
 */
char *get_err_msg(int err)
{
#ifdef CONFIG_REV_BETA
    return DEFAUT_MSG;
#else 
	return DEFAUT_MSG;
#endif 
}
