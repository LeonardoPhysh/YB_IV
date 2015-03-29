/*
 * Key Board Funtion Base on Serial 
 *  - Define key board function code 
 *  
 * Author : Leonardo Physh 
 * Date   : 2014.9.10 Rev01
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

#define PORT_0      "/dev/ttySAC0"
#define PORT_1      "/dev/ttySAC1"
#define PORT_2      "/dev/ttySAC2"
#define PORT_3      "/dev/ttySAC3"

static int g_port = 0;

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
        usleep(10000);

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

        case INPUT_PINYIN:
            ptr = low_key_map[area];
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
    switch(g_port) {
        case 1:
            key_board = open_uart(PORT_0, O_RDONLY | O_NONBLOCK);
            if (key_board < 0)
                return (void*)0;
            break;

        case 2:
            key_board = open_uart(PORT_1, O_RDONLY | O_NONBLOCK);
            if (key_board < 0)
                return (void*)0;
            break;

        case 3:   
            key_board = open_uart(PORT_2, O_RDONLY | O_NONBLOCK);
            if (key_board < 0)
                return (void*)0;
            break;

        case 4:
            key_board = open_uart(PORT_3, O_RDONLY | O_NONBLOCK);
            if (key_board < 0)
                return (void*)0;
            break;

        default:
            exit(0);
            break;

    }

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
        
        debug_msg("Keythread running...\n");

        FD_ZERO(&read_fd);
        FD_SET(key_board, &read_fd);

        ret = select(key_board + 1, &read_fd, NULL, NULL, &time_out);
        if (ret == -1) {
            debug_msg("KEY_BOARD : select error\n");
            usleep(100000);
            continue;
        } else if (ret > 0) {
            if (FD_ISSET(key_board, &read_fd)) {
                debug_msg("Reading uart...\n");

                ret = read_uart(key_board, &c, 1);
                if (ret != SUCCESS) {
                    debug_msg("KEY_BOARD: read error\n");
                    usleep(100000);
                    continue;
                } 

                debug_msg("HW CODE : %d\n", (int)c);
                
                /*
                 * check return value with key code 
                 */
                key_value = decode_key(c);
                debug_msg("SW CODE : %d\n", key_value);

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

/*
 * following is keyboard test function
 */
int main(void)
{
    int fd;
    int ime;
    int ret;

    uchar ch;
    int key_code;

    printf("KEYBOARD TEST TOOL(2014-10-23)\n");
    printf("------------------\n");
    printf("1- %s\n", PORT_0);
    printf("2- %s\n", PORT_1);
    printf("3- %s\n", PORT_2);
    printf("4- %s\n", PORT_3);
    printf("------------------\n");
    printf("Keyboard Used which serial port?(1-4):");

    scanf("%d", &g_port);
    while ((ch = getchar()) != '\n' && ch != EOF);

    printf("------------------\n");
    printf("1- Input Low Case\n");
    printf("2- Input UP Case\n");
    printf("3- Input Sign\n");
    printf("4- Input Function Key\n");
    printf("------------------\n");
    printf("Choose IME(1-4) : ");

    scanf("%d", &ime);
    while ((ch = getchar()) != '\n' && ch != EOF);

    switch(ime) {
        case 1:
            set_ime_status(INPUT_LOW_CASE);
            break;

        case 2:
            set_ime_status(INPUT_UP_CASE);
            break;

        case 3:
            set_ime_status(INPUT_SIGN);
            break;

        case 4:
            set_ime_status(INPUT_FUNC);
            break;

        default:
            printf("BAD IME\n");
            return FAIL;
            break;
    }

    keyboard_init();

    clear_cache();
    while(1) {
        debug_msg("Getting keycode...\n");

        key_code = get_keycode();
        debug_msg("Preview Keycode : %d\n", key_code);

        if (is_func(key_code)) {
            debug_msg("SW CODE : func key\n");
        } else if (key_code == '0') {
            debug_msg("SW CODE : nul\n");
        } else {
            debug_msg("SW CODE : %c\n", (char)key_code);
        }
    }

    return SUCCESS;
}
