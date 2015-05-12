/*
 * input.c - Input function base on keyboard function
 *  - implement multi-functions of input 
 *
 * Author : Leonardo Physh <leonardo.physh@gmail.com>
 * Date   : 2014.9.28
 */

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "config.h"
#include "common.h"
#include "error.h"
#include "keyboard.h"
#include "input.h"
#include "ui_api.h"

/*
 * get_bcd_date - get bcd format date and echo local 
 *  @row, col : local echo position 
 *  @bcd_date : the getting date  
 */
int get_bcd_date(int row, int col, struct bcd_date * enddate)
{
    int ret;
    int key, offset = 0;
    uint y, m, d;
    char year[4 + 1] = {0}, mon[2 + 1] = {0}, day[2 + 1] = {0};
    char ascii_no[9] = {0}; 
    
    bcd_to_greg(enddate, &y, &m, &d);
    if (y != 0 || m != 0 || d != 0) {
        offset = snprintf(ascii_no, 9, "%d%02d%02d", y, m, d);
    } 

    ret = SUCCESS; 
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        show_str(row, col, ascii_no);

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 8) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) {
                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ESC:
                    ret = -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    if (offset > 0) {
                        ret = SUCCESS;
                        goto handled;
                    }
                    break;

                default:
                    break;
            }
        }
    }

handled:
    if (ret == -EUI_ESC) 
        return ret;

    memcpy(year, ascii_no, 4);
    memcpy(mon, ascii_no + 4, 2);
    memcpy(day, ascii_no + 6, 2);
    y = m = d = 0;

    y = atoi(year);
    m = atoi(mon);
    d = atoi(day);

    if (y >= 2014 && y <= 2100 && m > 0 && m <= 12 && d > 0 && d <= 31) {
        greg_to_bcd(enddate, y, m, d);
    } else {
        ret = -EUI_BAD_DATE_FORMAT;
    }

    return ret;
}

int get_greg_time(int row, int col, struct greg_time *time)
{
    int ret;
    int key, offset = 0;
    char hour[2 + 1] = {0}, min[2 + 1] = {0}, sec[2 + 1] = {0};
    char ascii_no[7] = {0}; 

    if (time->hour != 0 || time->min != 0 || time->sec != 0) {
        offset = snprintf(ascii_no, 7, "%02d%02d%02d", 
                time->hour, time->min, time->sec);
    }

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        show_str(row, col, ascii_no);

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 6) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) {
                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ESC:
                    ret = -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    if (offset > 0) {
                        ret = SUCCESS;
                        goto handled;
                    }
                    break;

                default:
                    break;
            }
        }
    }

handled:
    if (ret == -EUI_ESC)
        return ret;

    memcpy(hour, ascii_no, 2);
    memcpy(min, ascii_no + 2, 2);
    memcpy(sec, ascii_no + 4, 2);

    time->hour = time->min = time->sec = 0;

    time->hour = atoi(hour);
    time->min = atoi(min);
    time->sec = atoi(sec);

    if (time->hour >= 0 && time->hour <= 23 
            && time->min >= 0 && time->min <= 59 
            && time->sec >= 0 && time->sec <= 59) {

        ret = SUCCESS;
    } else {
        ret = -EUI_BAD_TIME_FORMAT;
    }

    return ret;
}

/*
 * get_string - get a string(letter&number) with local echo  
 *
 * @return : status 
 */
int get_string(int row, int col, char *str)
{
    int ret;
    int key, offset = 0;
    char ascii_no[MAX_STR_LEN + 1] = {0}; 

    if (strlen(str) > 0) {
        offset = snprintf(ascii_no, MAX_STR_LEN + 1, "%s", str);
    }

    ret = SUCCESS;
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) { 
        show_str(row, col, ascii_no);

        key = get_keycode();
        if (islower(key) || (key >= '0' && key <= '9')) {
            if (offset < MAX_STR_LEN) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) { 
                case ESC:
                    return -EUI_ESC;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ENTER:
                    if (offset > 0) {
                        ret = SUCCESS;
                        goto handled;
                    }
                    break;

                default:
                    break;
            }
        }
    }

handled:
    strcpy(str, ascii_no);
    return ret;
}

/*
 * get_passwd - get string with "*" local echo  
 *
 * @return : status 
 */
int get_passwd(int row, int col, char *password)
{
    int ret;
    int key, offset = 0;
    char ascii_no[USER_PASSWD_LEN + 1] = {0}; 
    char pass_wd[USER_PASSWD_LEN + 1] = {0};

    if (password[0] != '\0') {
        offset = snprintf(ascii_no, USER_PASSWD_LEN + 1, "%s", password);
    }

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) {
        show_str(row, col, ascii_no);

        key = get_keycode();
        if (key >= '0' && key <= '9') {
            if (offset < USER_PASSWD_LEN) {
                sprintf(ascii_no + offset, "%c", '*');
                pass_wd[offset] = key;
                offset ++;
            }
        } else {
            switch (key) {
                case ESC:
                   return -EUI_ESC;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        pass_wd[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ENTER:
                    if (offset > 0) {
                        ret = SUCCESS;
                        goto handled;
                    }
                    break;

                default:
                    break;
            }
        }
    }

handled:
    strcpy(password, pass_wd);
    return ret;
}


/*
 * --ATTENTION-- 
 *  get_inter_num wouldn't clear the whole line before 
 *  show the content, instead, the heper function will 
 *  just clean few area before show the content.
 */
#define show_inter_num(row, col, cont) \
    do { \
        display_str(row, col, "      "); \
        display_str(row, col, cont); \
    } while(0)

int get_inter_num(int row, int col, int *num)
{
    int ret;
    int key, offset = 0;

    /* max : 0 - 10000 */
    char ascii_no[6] = {0}; 

    if (*num != 0) {
        offset = snprintf(ascii_no, 5, "%d", *num);
    } 

    ret = SUCCESS;    
    set_ime_status(INPUT_LOW_CASE);

    clear_cache();
    while (1) {
        show_inter_num(row, col, ascii_no);

        key = get_keycode();
        if (isdigit(key)) {
            if (offset < 5) {
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) {
                case D_ZERO:
                    if (offset != 0 && offset < 4) {
                        sprintf(ascii_no + offset, "%c%c", '0', '0');
                        offset += 2;
                    } else {
                        if (offset == 4) {
                            sprintf(ascii_no + offset, "%c", '0');
                            offset ++;
                        }
                    }
                    break;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ESC:
                    return -EUI_ESC;
                    goto handled;
                    break;

                case ENTER:
                    ret = SUCCESS;
                    goto handled;
                    break;

                default:
                    break;
            }
        }
    }

handled:
    *num = atoi(ascii_no);

    return ret;
}

/*
 * get_hex_num - get hex number
 *  @row, col : local echo position
 *  @pin : receive buffer 
 *  @len : hex number length  
 */
int get_hex_num(int row, int col, uchar *pin, int len)
{
    int i;
    int key, offset = 0;
    char ascii_no[48] = {0}; 

    memset(pin, 0, len);

    set_ime_status(INPUT_LOW_CASE); 

    clear_cache();
    while (1) { 
        show_str(row, col, ascii_no);

        key = get_keycode();
        if ((key >= 'A' && key <= 'F') || (key >= 'a' && key <= 'f') 
                || (key >= '0' && key <= '9')) {
            if (offset < 2*len){
                sprintf(ascii_no + offset, "%c", (char)key);
                offset ++;
            }
        } else {
            switch (key) { 
                case ESC:
                    return -EUI_ESC;

                case BACK:
                    if (offset > 0) {
                        ascii_no[offset - 1] = '\0';
                        offset --;
                    }
                    break;

                case ENTER:
                    if (offset > 0 && offset == 2*len) {
                        goto handled;
                    }
                    break;

                default:
                    break;
            }
        }
    }

handled:
    for (i = 0; i < 2*len; i++) {
        if (ascii_no[i] >= 'A' && ascii_no[i] <= 'F')
            ascii_no[i] = ascii_no[i] - 'A' + 10;
        else if (ascii_no[i] >= 'a' && ascii_no[i] <= 'f')
            ascii_no[i] = ascii_no[i] - 'a' + 10;
        else if (ascii_no[i] >= '0' && ascii_no[i] <= '9')
            ascii_no[i] = ascii_no[i] - '0';
    }

    for (i = 0; i < len; i++) {
        pin[i] = ascii_no[2 * i] * 16 + ascii_no[2*i + 1];
    }

    return SUCCESS;
}


#define CONTENT_SIZE    18

static int get_chn_pad(char *chn, char *chn_content)
{
    int len, pos; 
    memset(chn_content, 0, CONTENT_SIZE);

    len = strlen(chn);
    if (len > 8)
        len = 8;

    len = len / 2;

    for (pos = 0; pos < len; pos ++) {
        sprintf(chn_content + pos * 4, " %d", pos + 1);
        memcpy(chn_content + pos * 4 + 2, chn + pos * 2, 2);
    }

    chn_content[16] = ' ';
    chn_content[17] = ' ';

    return SUCCESS;
}

#define IME_PAD_BLANK   "      "
#define CHN_PAD_BLANK   "                    "

#define show_ime_pad(row, col, cont)    \
    do { \
        display_str(row, col, IME_PAD_BLANK); \
        display_str(row, col, cont); \
    } while(0)

#define show_chn_pad(row, col, cont)    \
    do { \
        display_str(row, col, CHN_PAD_BLANK);\
        display_str(row, col, cont); \
    } while(0)

/*
 * ui_pinyin - show pinyin spell and corresponding chinese 
 *  @cont : buffer 
 *  @return : one choose chinese 
 */
static int ui_pinyin(char *cont)
{
    int keycode, ime_state, count;
    int pos, page;
    char *chn = NULL;
    char spell[6] = {0};
    char chn_content[CONTENT_SIZE];
    char *ime_pad[] = {
        "        ",  
        "大写数字",
        "小写数字",
        "拼音输入",
        "符号输入"
    };

    ime_state = get_ime_status();
    if (ime_state == INPUT_FUNC) {
        set_ime_status(INPUT_PINYIN);
        ime_state = INPUT_PINYIN;
    }

    show_ime_pad(4, 1, ime_pad[ime_state]);
    clear_cache();

    pos = page = count = 0;
    while (1) {
        keycode = get_keycode();
        switch (keycode) {
            case SHIFT:
                if (ime_state == INPUT_SIGN) 
                    ime_state = INPUT_UP_CASE;
                else 
                    ime_state ++;

                count = 0;
                chn = NULL;    
                memset(spell, 0, 6); 
                memset(chn_content, 0, CONTENT_SIZE);

                /* somewhere blink cursor, concel it */ 
                if (pos > 0) {
                    set_cursor_off(4, 5 + (pos - 1) * 2);
                    pos = page = 0;
                }
                set_ime_status(ime_state);
                show_str(4, 1, ime_pad[ime_state]);

                break;

            case ESC:
                if (pos > 0) {
                    set_cursor_off(4, 5 + (pos - 1) * 2);
                    pos = page = 0;
                }

                show_str(4, 1, CHAR_BLANK); 
                return -EUI_ESC;

                break;

            case BACK:
                if (ime_state != INPUT_PINYIN)
                    return -EUI_BACK;

                if (count > 0) {
                    if (pos > 0) {
                        set_cursor_off(4, 5 + (pos - 1) * 2);
                        pos = page = 0;
                    }

                    spell[--count] = '\0';
                    if (count == 0) {
                        show_str(4, 1, ime_pad[ime_state]);
                        chn = NULL;
                        memset(chn_content, 0, CONTENT_SIZE);
                    } else {
                        show_ime_pad(4, 1, spell);
                        chn = py_ime(spell);
                        if (chn != NULL) {
                            get_chn_pad(chn, chn_content);
                            show_chn_pad(4, 4, chn_content);
                        } else 
                            show_chn_pad(4, 4, NULL);
                    }
                } else {
                    return -EUI_BACK;
                }

                break;

            case ENTER:
                if (count > 0) {
                    if (chn != NULL && pos > 0) {
                        set_cursor_off(4, 5 + (pos - 1) * 2);
                        pos = page = 0;
                        show_str(4, 1, ime_pad[ime_state]);

                        memcpy(cont, chn + (pos - 1) * 2, 2);
                        goto handled;
                    }
                } else 
                    return ENTER;

                break;

            case LEFT:
                if (chn != NULL) {
                    if (pos > 1) {
                        set_cursor_off(4, 5 + (pos - 1) * 2);
                        pos --;
                        set_cursor_on(4, 5 + (pos - 1) * 2);
                    } 
                }

                break;

            case RIGHT:
                if (chn != NULL) {
                    if (pos < 4 && strlen(chn) / 2 > pos){
                        if (pos != 0)
                            set_cursor_off(4, 5 + (pos - 1) * 2);

                        pos ++;  
                        set_cursor_off(4, 5 + (pos - 1) * 2);
                    }
                }
                break;

            case UP:
                if (chn != NULL) {
                    if (page > 0) {
                        if (pos != 0)
                            set_cursor_off(4, 5 + (pos - 1) * 2);

                        chn -= 8;
                        get_chn_pad(chn, chn_content);
                        show_chn_pad(4, 4, chn_content);

                        pos = 1;
                        set_cursor_on(4, 5 + (pos - 1) * 2);
                        page --;
                    }
                }
                break;

            case DOWN:
                if (chn != NULL) {
                    if (strlen(chn) > 8) {
                        if (pos != 0)
                            set_cursor_off(4, 5 + (pos - 1) * 2);

                        chn += 8;
                        get_chn_pad(chn, chn_content);
                        show_chn_pad(4, 4, chn_content);

                        pos = 1;
                        set_cursor_on(4, 5 + (pos - 1) * 2);

                        page ++;
                    }
                }
                break;

            default:
                if (ime_state != INPUT_PINYIN) {
                    if (keycode != '\0') {
                        cont[0] = keycode;
                        goto handled;
                    }
                } else {
                    if (islower(keycode)) {
                        if (count < 5) {
                            if (pos > 0) {
                                set_cursor_off(4, 5 + (pos - 1) * 2);
                                pos = page = 0;
                            }

                            spell[count++] = (char)keycode;
                            show_ime_pad(4, 1, spell);
                            chn = py_ime(spell);
                            if (chn != NULL) {
                                get_chn_pad(chn, chn_content);
                                show_chn_pad(4, 4, chn_content);
                            } else {
                                show_chn_pad(4, 4, NULL);
                            }
                        }
                    }

                    if (isdigit(keycode)) {
                        if (chn != NULL) {
                            if ((keycode - '0') <= (strlen(chn) / 2)) {
                                if (pos > 0) 
                                    set_cursor_off(4, 5 + (pos - 1) * 2);

                                count = pos = page = 0;
                                memcpy(cont, chn + (keycode - '1') * 2, 2);
                                show_str(4, 1, ime_pad[ime_state]);
                                goto handled;
                            }
                        }
                    }
                }

                break;
        }
    }

handled:

    return SUCCESS;
}


int get_chn_str(int row, int col, char *name)
{
    int ret, len = 0;
    char chn[2];

    set_ime_status(INPUT_PINYIN);

    clear_cache();
    while (1) {
        show_str(row, col, name);
        len = strlen(name);

        chn[0] = chn[1] = '\0';

        ret = ui_pinyin(chn);
        switch (ret) {
            case ENTER:
                /*
                   if (len != 0)
                   return SUCCESS;
                   */
                /* we don't check string is null or not, 
                 * let up-layer do that 
                 */
                return SUCCESS;
                break;

            case -EUI_BACK:
                if (len > 0) {
                    if ((uchar)name[len - 1] < 127) {
                        name[len - 1] = '\0';
                    } else {
                        name[len - 1] = '\0';
                        name[len - 2] = '\0';
                    }
                } else 
                    return -EUI_BACK;
                break;

            case -EUI_ESC:
                return -EUI_ESC;
                break;

            case SUCCESS:
                if ((uchar)chn[0] > 127) {
                    name[len] = chn[0];
                    name[len + 1] = chn[1];
                } else {
                    name[len] = chn[0];
                }
                break;

            default:
                break;
        } 
    }
}

int get_barcode(int row, int col, char * barcode)
{
    int ret;

#ifndef CONFIG_HPP
    return FAIL;
#endif 
    //read uart to get barcode
    ret = SUCCESS;

    return ret;
}

/* end of input.c */

