/*
 * start_main.c - main function defined here
 *  entry of tax system, you can read the code to get detail. 
 * 
 * Author : Leonardo Physh
 * Data   : 2014.10.12
 */

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "uart.h"
#include "input.h"
#include "print.h"
#include "tax_system.h"
#include "ui_api.h"
#include "plu.h"

/*
 * global keyboard pthread ID
 */
static pthread_t tid = 0;

int system_init(void) 
{
    int ret;
    struct print_sys * print_sys = get_print_sys();
    struct tax_system * tax_system = get_tax_system();

    display_info("正在进行硬件自检，请稍后...");
    
    /* 
     * fisrt time boot up, there is no printer information
     * skip printer checking.
     */
    ret = print_sys->print_sys_init(print_sys);
    if (ret != SUCCESS) {
        if (ret != -EFUNC_FIRST_BOOT) {
            display_err_msg(ret, "打印机未就绪！");
            return FAIL;
        }
    } else {
        ret = print_sys->ops->print_boot_check();
        if (ret != SUCCESS) {
            display_err_msg(ret, "打印机未就绪！");
            return FAIL;
        }
    }

    /* create the keyboard pthread */
    tid = keyboard_init();
    if (tid < 0) {
        display_err_msg(ret, "键盘未就绪！");
        return FAIL; 
    }

    /* initial card reader */
    ret = tax_system->card_init();
    if (ret < 0) {
        display_err_msg(ret, "税控卡未就绪！");
        return FAIL;
    }

    /*
     * if fiscal system is not starting use,
     * we supposed to do nothing.
     */
    ret = tax_system->is_fiscal_init();
    if (ret == POSITIVE) {
        ret = tax_system->power_on_check();
        if (ret != SUCCESS) {
            display_err_msg(ret, "上电自检失败！");
            return FAIL;
        }
    }

    sleep(1);
    return SUCCESS;
}

void sign_handle(int sig_num)
{
    int ret;
    ret = keyboard_stop();
    if (ret < 0)
        debug_msg("Keyboard thread cancel failed.\n");

    exit(0);
}

int main(void)
{
    int ret;
    int key;

    signal(SIGINT, sign_handle);

    ret = system_init();
    if (ret != SUCCESS) {
        debug_msg("HW INIT: failed.\n");

        if (tid != 0)
            keyboard_stop();

        exit(0);
    }

    while (check_shutdown() == NEGATIVE) {
        log_in();

        while (check_logout() == NEGATIVE) {
            ui_get_keycode(&key);
            ui_handle_keycode(key);
        }
        log_out();
    }
    shutdown();

    return 0;
}

