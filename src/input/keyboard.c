/*
 * Key Board Funtion Base on Serial 
 *  - Define key board function code 
 *  
 * Author : Leonardo Physh <leonardo.physh@gmail.com>
 * Date   : 2014.9.10 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "common.h"
#include "config.h"
#include "error.h"

#include "uart.h"
#include "input.h"
#include "keyboard.h"

uint nul_table[5] = {'\0', '\0', '\0', '\0', '\0',};

/* low-case */
uint a_e_table[5] = {'a', 'b', 'c', 'd', 'e'};
uint f_j_table[5] = {'f', 'g', 'h', 'i', 'j'};
uint k_j_table[5] = {'k', 'l', 'm', 'n', 'o'};
uint p_t_table[5] = {'p', 'q', 'r', 's', 't'};
uint u_y_table[5] = {'u', 'v', 'w', 'x', 'y'};
uint z_sp_table[5] = {BACK, ESC, SHIFT, ' ', 'z'};

/* UP-case */
uint A_E_table[5] = {'A', 'B', 'C', 'D', 'E'};
uint F_J_table[5] = {'F', 'G', 'H', 'I', 'J'};
uint K_J_table[5] = {'K', 'L', 'M', 'N', 'O'};
uint P_T_table[5] = {'P', 'Q', 'R', 'S', 'T'};
uint U_Y_table[5] = {'U', 'V', 'W', 'X', 'Y'};
uint Z_SP_table[5] = {BACK, ESC, SHIFT, ' ', 'Z'};

/* number table */
uint num_1_table[5] = {RIGHT, LEFT, '6', '5', '4'};
uint num_2_table[5] = {DOWN, UP, '9', '8', '7'};
uint num_3_table[5] = {'\0', '\0', '3', '2', '1'};
uint num_4_table[5] = {'\0', '\0', '.', D_ZERO, '0'};


/* sign table */
uint sign_1_table[5] = {'.', '/', '?', '=', '|'};
uint sign_2_table[5] = {'-', '+', '^', '%', '$'};
uint sign_3_table[5] = {']', '[', '(', '*', '&'};
uint sign_4_table[5] = {BACK, ESC, SHIFT, '\0', '\0'};
uint sign_5_table[5] = {'\0', '\0', '#', '@', '!'};
uint sign_6_table[5] = {'\0', '\0', '\0', '\\', '('};

/* function key table */
uint func_1_table[5] = {'\0', '\0', '\0', PAPER_IN, PAPER_OUT};
uint func_2_table[5] = {RIGHT, LEFT, '\0', '\0', '\0'};
uint func_3_table[5] = {DOWN, UP, '\0', '\0', '\0'};
uint func_4_table[5] = {BACK, ESC, SHIFT, HANG, VIEW};
uint func_5_table[5] = {TOTAL, '\0', '\0', '\0', '\0'};
uint func_6_table[5] = {ENTER, '\0', '\0', '\0', '\0'};

uint * low_key_map[] = {
/* low-case & number input */
    a_e_table,
    f_j_table,
    k_j_table,
    p_t_table,
    u_y_table,
    num_1_table,
    num_2_table,
    z_sp_table
};

uint * UP_key_map[] = {
/* UP-case & number input */
    A_E_table,
    F_J_table,
    K_J_table,
    P_T_table,
    U_Y_table,
    num_1_table,
    num_2_table,
    Z_SP_table
};

uint * sign_key_map[] = {
    /* sign input */
    nul_table,
    nul_table,
    nul_table,
    nul_table,
    sign_1_table,
    sign_2_table,
    sign_3_table,
    sign_4_table
};

uint * func_key_map[] = {
    /* function key */
    nul_table,
    nul_table,
    nul_table,
    nul_table,
    func_1_table,
    func_2_table,
    func_3_table,
    func_4_table
};

/*
 * Global valule 
 */

static pthread_t tid;
static pthread_mutex_t k_cache_lock;

static struct key_code g_key_code = {
    .flag = NEGATIVE,
    .end = 0,
    .key_value = {0},
};

static volatile int IME_STATUS = INPUT_LOW_CASE;

int get_ime_status(void)
{
    return IME_STATUS;
}

int set_ime_status(int mode)
{
    if (mode >= INPUT_FUNC && mode <= INPUT_SIGN) {
        IME_STATUS = mode;
        
        return SUCCESS;
    }

    return FAIL;
}

/*
 * get_key_code - get one decode key_value from 
 *                input buffer. 
 * 
 * @return : the last key value 
 */
int get_keycode(void)
{
    int key;
    int pos;
    
    while (g_key_code.flag != POSITIVE)
        usleep(1000);

    pthread_mutex_lock(&k_cache_lock);
    
    pos = g_key_code.end - 1;
    key = g_key_code.key_value[pos];
    g_key_code.end--;

    if (g_key_code.end == 0)
        g_key_code.flag = NEGATIVE;

    pthread_mutex_unlock(&k_cache_lock);

    return key;
}

/*
 * clear_cache - clear input buffer 
 */
int clear_cache(void)
{
    pthread_mutex_lock(&k_cache_lock);

    g_key_code.flag = NEGATIVE;
    g_key_code.end = 0;
    memset(g_key_code.key_value, 0, CACHE_SIZE);
    
    pthread_mutex_unlock(&k_cache_lock);

    return SUCCESS;
}


/*
 * decode_key - decode hw_code to sw_code
 *  @ch : hw_code
 *  @return :sw_code
 */
uint decode_key(int ch)
{
    uint * ptr;
    uint area = 0;
    uint post = 0;

    uint sw_code = 0;
    uint hw_code = (uint)ch;
    
    if (!(ch >= HW_KEY_CODE_START && ch <= HW_KEY_CODE_END))
        return FAIL;

    /* some hw code is irregular*/
    switch (hw_code) {
        case 41:
            sw_code = TOTAL;

            goto chk_sw_code;
            break;

        case 58:
            if (IME_STATUS == INPUT_SIGN)
                sw_code = '#';
            else if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = '3';

            goto chk_sw_code;
            break;

        case 59:
            if (IME_STATUS == INPUT_SIGN)
                sw_code = '@';
            else if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = '2';

            goto chk_sw_code;
            break;

        case 51:
            if (IME_STATUS == INPUT_SIGN)
                sw_code = '!';
            else if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = '1';

            goto chk_sw_code;
            break;

        case 49:
            sw_code = ENTER;

            goto chk_sw_code;
            break;

        case 50:
            if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = '.';

            goto chk_sw_code;
            break;

        case 42:
            if (IME_STATUS == INPUT_SIGN)
                sw_code = '\\';
            else if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = D_ZERO;

            goto chk_sw_code;
            break;

        case 43:
            if (IME_STATUS == INPUT_SIGN)
                sw_code = ')';
            else if (IME_STATUS == INPUT_FUNC)
                sw_code = '\0';
            else 
                sw_code = '0';

            goto chk_sw_code;
            break;

        default:
            break;
    }   
    
    /* below hw_code is regular */
    area = hw_code - 4;
    area = area / 8;

    post = hw_code -4;
    post = post % 8;

    switch (IME_STATUS) {
        case INPUT_FUNC:
            ptr = func_key_map[area];
            sw_code = ptr[post];
            goto chk_sw_code;
            break;

        case INPUT_PINYIN:
        case INPUT_LOW_CASE:
            ptr = low_key_map[area];
            sw_code = ptr[post];
            goto chk_sw_code;
            break;

        case INPUT_UP_CASE:
            ptr = UP_key_map[area];
            sw_code = ptr[post];
            goto chk_sw_code;
            break;

        case INPUT_SIGN:
            ptr = sign_key_map[area];
            sw_code = ptr[post];
            goto chk_sw_code;
            break;

        default:
            return FAIL;
            break;
    }

chk_sw_code:
#if 0
    if (IME_STATUS == INPUT_FUNC)
        if (!(is_func(sw_code)))
            return FAIL;

    if (IME_STATUS > INPUT_FUNC)
        if (!(is_ascii(sw_code)))
            return FAIL;
#endif

    return sw_code;
}


/*
 * keyboard_thread - run while system is on 
 *   - handle hw keycode and decode to sw_code
 *   then report to UI
 */
void * keyboard_thread(void *arg)
{
    int ret;
    int i, pos;
    int key_board;

    fd_set read_fd;
    struct timeval time_out;
    
    /*
     * keybard need to add O_NONBLOCK 
     */
    key_board = open_uart(KEYBD_DEV, O_RDONLY | O_NONBLOCK);
    if (key_board < 0)
        return (void*)0;
    
    ret = kb_uart_setup(key_board);
    if (ret != SUCCESS) {
        close(key_board);
        return (void*)0;
    }

    uchar c;
    uint key_value;
    while (1) {
        time_out.tv_sec = 2;
        time_out.tv_usec = 0;

        FD_ZERO(&read_fd);
        FD_SET(key_board, &read_fd);

        ret = select(key_board + 1, &read_fd, NULL, NULL, &time_out);
        if (ret == -1) {
            debug_msg("KEY_BOARD : select error\n");
            usleep(100000);
            continue;
        } else if (ret > 0) {
            if (FD_ISSET(key_board, &read_fd)) {
                ret = read_uart(key_board, &c, 1);
                if (ret != SUCCESS) {
                    debug_msg("KEY_BOARD: read error\n");
                    usleep(100000);
                    continue;
                }
                /*
                 * check return value with key code 
                 */
                key_value = decode_key(c);
                if (key_value >= 0) {
                    pthread_mutex_lock(&k_cache_lock);

                    pos = g_key_code.end;
                    if (pos == 10) {
                        for (i = 0; i < 9; i++)
                            g_key_code.key_value[i] = g_key_code.key_value[i + 1];

                        g_key_code.key_value[i] = key_value;
                    } else {
                        g_key_code.key_value[pos] = key_value;
                        g_key_code.end++;
                    }

                    if (g_key_code.flag == NEGATIVE)
                        g_key_code.flag = POSITIVE;

                    pthread_mutex_unlock(&k_cache_lock);
                }
            }
        } else if (ret == 0) {
            usleep(100000);
        }

        usleep(100000);
    }

    return (void*)0;
}

/*
 * create keyboard thread and detach it
 */
pthread_t keyboard_init(void) 
{
	int ret;

    /*
     * initialize cache lock
     */
    pthread_mutex_init(&k_cache_lock, NULL);

	ret = pthread_create(&tid, NULL, keyboard_thread, NULL);
	if (ret != 0) 
		return FAIL;
	else 
		pthread_detach(tid);
	
	return tid;
}

/*
 * keyboard_stop - destroy keyboard thread 
 *  @return : status 
 */
int keyboard_stop(void)
{
    pthread_mutex_destroy(&k_cache_lock);

    pthread_cancel(tid);

    return SUCCESS;
}

