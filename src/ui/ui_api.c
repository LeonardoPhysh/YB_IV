/* 
 * ui_api.c - packaged APIs to show something on screen 
 *  - avoiding other modules operate LCM directly 
 *  - inline is vain, lcm_xxx is hiden from other modules, 
 *    compliler will not generate inline code.
 * 
 * Author : Leonrdo Physh <leonardo.physh@gmail.com> 
 * Date   : 2014.10.14
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include "config.h"
#include "error.h"
#include "common.h"
#include "command.h"
#include "tax_file_op.h"
#include "keyboard.h"
#include "input.h"
#include "ui.h"
#include "lcm_api.h"
#include "ui_api.h"

/*
 * global variables 
 */
static int g_cur_frame_id = 0;
static int g_cur_item_id = 0;

static int g_logout_flag = NEGATIVE;
static int g_shutdown_flag = NEGATIVE;

static struct user login_user;

/*
 * get_cur_user - get current login user  
 */
struct user * get_cur_user(void)
{
    return &login_user;    
}

/*
 * check_userlevel - check current user permession level is 
 *                   higher than level or not.
 *  @level  : level  
 *  @return : POSITIVE if current user's level higher than/equal to  level 
 *            NEGATIVE if lower than level
 */
int check_userlevel(int level) 
{
    struct user * user = get_cur_user();

    if (user->level >= level)
        return POSITIVE;
    else 
        return NEGATIVE;
}

int set_logout_flag(int flag)
{
    if (flag == NEGATIVE || flag == POSITIVE)
        g_logout_flag = flag;
    else 
        return FAIL;

    return SUCCESS;
}

int set_shutdown_flag(int flag)
{
    if (flag == NEGATIVE || flag == POSITIVE)
        g_shutdown_flag = flag;
    else 
        return FAIL;

    return SUCCESS;
}

int check_logout(void)
{
    return g_logout_flag == POSITIVE ? POSITIVE : NEGATIVE;
}

int check_shutdown(void)
{
    return g_shutdown_flag == POSITIVE ? POSITIVE : NEGATIVE;
}

inline static int get_cur_frame_id(void)
{
    return g_cur_frame_id;
}

inline static int set_cur_frame_id(int id)
{
    g_cur_frame_id = id;
    return SUCCESS;
}

inline static int push_ui_statck(int id) 
{
    int i;
    int top = g_ui_stack.top;

    if (top < 10) {
        g_ui_stack.stack[top] = id;
        g_ui_stack.top ++;

        return SUCCESS;
    }

    if (top == 10) {
        for (i = 0; i < 9; i++) {
            g_ui_stack.stack[i] = g_ui_stack.stack[i + 1];
        }

        g_ui_stack.stack[i] = id;
    }

    return SUCCESS;
}

inline static int pop_ui_statck(void) 
{
    int top;

    top = g_ui_stack.top;
    if (top == 0)
        return FAIL;

    g_ui_stack.top --;
    return g_ui_stack.stack[top - 1];
}

/*
 * fallback function for UI framework 
 */ 
static struct frame * get_target_frame(int id)
{
    int lev_1, lev_2, lev_3;

    struct frame *tmp, *target;

    lev_1 = lev_2 = lev_3 = 0;

    /* 
     * 1, 11, 111
     */
    if (id < 10) {
        lev_1 = id;
        lev_2 = 0;
        lev_3 = 0;
    } else if (id > 10 && id < 100) {
        lev_1 = id / 10;
        lev_2 = id % 10;
        lev_3 = 0; 
    } else if (id > 100) {
        lev_1 = id / 100;
        lev_2 = (id / 10) % 10;
        lev_3 = id % 10; 
    }

    tmp = g_ui_menus[lev_1];
    target = &tmp[lev_2 + lev_3];

    return target;
}

inline static int get_item_num(int menu_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->item_num;
}

inline static char * get_title(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return (char *)(target->items[item_id].title);
}

inline static int get_color(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->items[item_id].color;
}

inline static int get_row(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->items[item_id].pos.row;
}

inline static int get_col(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->items[item_id].pos.col;
}

inline static int get_parent_id(int id)
{
    int parent_id;

    /* 
     * 1, 11, 111
     */
    if (id < 10) {
        parent_id = 0;
    } else if (id > 10 && id < 100) {
        parent_id = id / 10;
    } else if (id > 100) {
        parent_id = id / 10;
    } else 
        parent_id = 0;

    return parent_id;
}

inline static int get_next_id(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->items[item_id].next.next_id;
}


inline static ACTION get_next_action(int menu_id, int item_id)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    return target->items[item_id].next.action;
}


inline static int set_color(int menu_id, int item_id, int color)
{
    struct frame *target;

    target = get_target_frame(menu_id);

    target->items[item_id].color = color;

    return 0;
}

inline static int set_questtion_title(char *str)
{
    struct frame *target;

    target = get_target_frame(7);

    if (str == NULL)
        return -1;
    
    memset(target->items[0].title, 0, sizeof(target->items[0].title));
    strcpy(target->items[0].title, str);

    return 0;
}

/*
 * clear_screen - clear screen 'cause lcm command 
 *                may result in other side effect 
 *                so clear screen manual.
 *  @return : status 
 */
int clear_screen(void)
{
    /*
     * lcm clear is OK 
     */  
    return lcm_clear();
}

/*
 * show_simple_frame - show a simple frame to screen
 *  @frame : content of UI items 
 *
 * ATTENTION: This funtion recommented to be used while 
 * a UI needs to get user input and this UI has no other 
 * interactivity element.
 */
int show_simple_frame(struct simple_frame * frame) 
{
    int row, col;
    int item_num, len;
    char *str, left[24] = {0};

    clear_screen();

    item_num = frame->item_num;
    
    /* item's content may be longer than 12 CN_Bits */
    int i, y;
    for (i = 0; i < item_num; i++) {
        row = frame->items[i].pos.row;
        col = frame->items[i].pos.col;
        str = frame->items[i].title;

        len = strlen(str);
        len = len / 2;

        if (len > (13 - col)) {
            len = (13 - col) * 2;

            y = 0; 
            while(y < len) {
                /* chinese */
                if ((uchar)str[y] > 127) {
                    if ((y + 2) > len)
                        break;

                    left[y] = str[y];
                    y++;
                    left[y] = str[y];
                    y++;

                    continue;
                }

                /* english */
                left[y] = str[y];
                y++;
            }

            lcm_printf(row, col, left);

            memset(left, 0, 24);
            memcpy(left, str + y, strlen(str) - y);
            lcm_printf(row + 1, 1, left);
        } else {
            lcm_printf(row, col, str);
        }
    }

    return SUCCESS;
}

/*
 * show_cur_ui - show id assigned frame to screen 
 *  @id : UI id 
 *  @return : status  
 */
int show_current_ui(int id)
{
    int row = 0;
    int col = 0;

    int item_num, len;
    char *str, left[24 + 1] = {0};

    struct frame *target = get_target_frame(id);
    if (target == NULL)
        return -EUI_BAD_UI_ID;

    clear_screen();

    item_num = get_item_num(id);

    int i, y;
    for (i = 0; i < item_num; i++) {
        row = get_row(id, i);
        col = get_col(id, i);
        str = get_title(id, i);

        len = strlen(str);

        if (len > (13 - col) * 2) {
            len = (13 - col) * 2;

            y = 0; 
            while(y < len) {
                /* chinese */
                if ((uchar)str[y] > 127) {
                    if ((y + 2) > len)
                        break;

                    left[y] = str[y];
                    y++;
                    left[y] = str[y];
                    y++;

                    continue;
                }

                /* english */
                left[y] = str[y];
                y++;
            }

            lcm_printf(row, col, left);

            memset(left, 0, 24 + 1);
            memcpy(left, str + y, strlen(str) - y);

            lcm_printf(row + 1, 1, left);
        } else {
            lcm_printf(row, col, str);
        }
    }

    set_cur_frame_id(id);

    return SUCCESS;
}

/*
 * show_str - show one string after clear screen
 *  @row, col: position to show string 
 *  @str : content to show up on screen 
 *  @return : status 
 */
int show_str(int row, int col, char *str)
{
    lcm_printf(row, col, CHAR_BLANK);
    lcm_printf(row, col, str);

    return SUCCESS;
}

/*
 * --ATTENTION-- 
 *  This API only use for showing something while you're 
 *  geting them from keyboard, this function will add
 *  a dummy cursor. 
 *
 * show_str_dmy_cur - show string with a dummy cursor 
 *  @row, col: position to show string 
 *  @str: content of string 
 *  @return : status 
 */
int show_str_dmy_cur(int row, int col, char *str)
{
    char dmy_str[48] = {0};

    snprintf(dmy_str, 48, "%s__", str);

    return show_str(row, col, dmy_str);
}

int display_str(int row, int col, char *format, ...)
{
    char str[50] = {0};
    va_list arg_ptr;	

    va_start(arg_ptr, format);
    vsprintf(str, format, arg_ptr);	
    va_end(arg_ptr);

    lcm_printf(row, col, str);

    return SUCCESS;
}

int display_info(char *msg)
{
    struct simple_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.item_num = 1;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    strcpy(frame.items[0].title, msg);
    
    show_simple_frame(&frame);
    sleep(1);

    return SUCCESS;
}

int display_warn(char *msg)
{
    display_info(msg);

    clear_cache();
    get_keycode();

    return SUCCESS;
}

int display_err_msg(int err, char *msg)
{
    struct simple_frame frame;

    memset(&frame, 0, sizeof(frame));

    frame.item_num = 2;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 1;
    strcpy(frame.items[0].title, msg);

    frame.items[1].pos.row = 4;
    frame.items[1].pos.col = 1;
    sprintf(frame.items[1].title, "错误代码：%d", err);
    
    /* for debug */
    printf("err_num:%d", err);
    show_simple_frame(&frame);
    sleep(1);

    clear_cache();
    get_keycode();

    return SUCCESS;
}

/*
 * --ATTENTION-- 
 *  we DO NOT use lcm API to set cursor, 'cause cursor 
 *  implement internal LCM. if you don't know how cursor 
 *  work, do NOT use it. 
 */
int set_cursor_on(int row, int col)
{
    int ret; 

    /* set cursor on, blink on*/
    ret = lcm_set_cursor(row, col, ON, ON);
    if (ret < 0)
        display_err_msg(ret, "设置光标出错！");

    return ret;
}

/*
 * --ATTENTION-- 
 *  we DO NOT use lcm API to set cursor, 'cause cursor 
 *  implement internal LCM. if you don't know how cursor 
 *  work, do NOT use it. 
 */
int set_cursor_off(int row, int col)
{
    int ret;

    /* set cursor off, blink off*/
    ret = lcm_set_cursor(row, col, OFF, OFF);
    if (ret < 0)
        display_err_msg(ret, "设置光标出错！");

    return ret;
}

/*
 * highlight_on - reverse line 
 *  @row: 1,2,3,4
 */
int highlight_on(int row)
{
    //return lcm_printf(row, 1, " >");
    return lcm_rev_line(row);
}

/*
 * highlight_off - cancle reverse line 
 *  @row: 1,2,3,4
 */
int highlight_off(int row)
{
    //return lcm_printf(row, 1, "  ");
    return lcm_rev_line(row);
}

/*
 * service for question_user 
 */
int SAY_YES(void) {return POSITIVE;}
int SAY_NO(void) {return NEGATIVE;}

/*
 * question_user - show question and get user option
 *  @return : user option
 */
int question_user(char *title)
{
    int key, ime_state;

    /* 1 : yes, 2: no*/
    int pos = 1, chioce = 1;
    ACTION handle;

    set_questtion_title(title);
    show_current_ui(7);

    lcm_printf(3, 3, "*"); 

    ime_state = get_ime_status();
    set_ime_status(INPUT_FUNC);

    clear_cache();
    while (1) {
        key = get_keycode();
        switch (key) {
            case ENTER:
                handle = get_next_action(7, pos);
                chioce = handle();
                goto handled;
                break;

            case RIGHT:
                if (pos == 1) {
                    lcm_printf(3, 3, " ");
                    lcm_printf(3, 7, "*"); 
                    pos = 2;
                }
                break;

            case LEFT:
                if (pos == 2) {
                    lcm_printf(3, 7, " ");
                    lcm_printf(3, 3, "*");
                    pos = 1;
                }
                break;

            default:
                break;
        }
    } 

handled:
    set_ime_status(ime_state);
    return chioce;
}

/*
 * log_in - show login UI, get username and password 
 *        - initialize login_user
 * return : status 
 */
int log_in(void)
{
    int ret;
    struct simple_frame frame;
    char username[USER_NAME_LEN + 1];
    char password[USER_PASSWD_LEN + 1];
 
    memset(username, 0, USER_NAME_LEN + 1);
    memset(password, 0, USER_PASSWD_LEN + 1);

    memset(&frame, 0, sizeof(frame));
    memset(&login_user, 0, sizeof(login_user));

    frame.item_num = 3;
    frame.items[0].pos.row = 1;
    frame.items[0].pos.col = 5;
    snprintf(frame.items[0].title, MAX_TITLE_LEN, "%s", "登    陆");

    frame.items[1].pos.row = 2;
    frame.items[1].pos.col = 1;
    snprintf(frame.items[1].title, MAX_TITLE_LEN, "%s", "用户名：");

    frame.items[2].pos.row = 4;
    frame.items[2].pos.col = 1;
    snprintf(frame.items[2].title, MAX_TITLE_LEN, "%s", "密  码：");

    while (1) {
        show_simple_frame(&frame);

        while (get_string(2, 5, username) != SUCCESS);
        while (get_passwd(4, 5, password) != SUCCESS);

        tax_file_find_user(username, &login_user);
        ret = strcoll(password, login_user.passwd);
        if (ret != 0) {
            display_warn("用户名或密码错误!");

            memset(username, 0, USER_NAME_LEN + 1);
            memset(password, 0, USER_PASSWD_LEN + 1);
            continue;
        } else if (ret == 0) {
            break;
        }
    }

    show_current_ui(0);
    return SUCCESS;
}

int log_out(void)
{
    display_info("正在注销...");

    usleep(10000);
    sync();
    memset(&login_user, 0, sizeof(login_user));
    memset(&g_ui_stack, 0, sizeof(g_ui_stack));

    g_cur_frame_id = 0;
    g_cur_item_id = 0;
    g_logout_flag = NEGATIVE;
    g_shutdown_flag = NEGATIVE; //clear logout/shutdown flag

    return SUCCESS;
}

int shutdown(void)
{
    int ret;

    display_info("正在关机...");

    usleep(100000);
    sync();

    ret = system("shutdown");
    if (ret < 0)
        return FAIL;

    return SUCCESS;
}

/*
 * ui_get_keycode - get ui keycode 
 *  @key : receive buffer 
 *  @return : status 
 */
int ui_get_keycode(int *key)
{
    int keycode, ime_state;

    ime_state = get_ime_status();
    set_ime_status(INPUT_LOW_CASE);

    clear_cache();
    while (1) {
        keycode = get_keycode();
        if (!(keycode >= '1' && keycode <= '6') && keycode != BACK && keycode != 'z')
            continue;
        else 
            break;
    }

    if (keycode == BACK)
        *key = BACK;
    else if (keycode == 'z')
        *key = HANG;
    else 
        *key = keycode - '0';

    set_ime_status(ime_state);

    return SUCCESS; 
}

/*
 * ui_handle_keycode - handle ui keycode 
 *  @key : key except to be HANG, BACK, 1~6
 *  @return : status
 */
int ui_handle_keycode(int key)
{
    int cur_id, item_num;
    int next_id, parent_id;

    ACTION handler; 

    cur_id = get_cur_frame_id();

    if (key == BACK) {
        parent_id = get_parent_id(cur_id);
        if (cur_id == parent_id)
            return SUCCESS;

        pop_ui_statck();

        set_cur_frame_id(parent_id);
        show_current_ui(parent_id);

        return SUCCESS;
    }

    if (key == HANG) {
        cmd_resume_transact();

        set_cur_frame_id(cur_id);
        show_current_ui(cur_id);

        return SUCCESS;
    } 

    item_num = get_item_num(cur_id);

    if (key > (item_num - 1))
        return FAIL;

    next_id = get_next_id(cur_id, key);
    handler = get_next_action(cur_id, key);

    if (handler == NULL) {
        push_ui_statck(cur_id);

        set_cur_frame_id(next_id);
        show_current_ui(next_id);

        return SUCCESS;
    } else {
        handler();

        push_ui_statck(cur_id);

        if (next_id == CLEAN_UI) {
            /*
             * just for log out and shutdown 
             */
            clear_screen();
        } else {
            set_cur_frame_id(next_id);
            show_current_ui(next_id);
        }

        return SUCCESS;
    }    
}

/* end of ui_api.c */
