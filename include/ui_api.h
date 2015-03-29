/*
 * UI APIs Head File 
 *  all LCM about funtions are defined here, we suppose that 
 *  only ui_api can handle LCM directly, other modules have 
 *  to show something in LCM by UI API.
 *
 * Author : Leoanrdo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : 2014.12.25
 */

#ifndef  __UI_API_H__
#define  __UI_API_H__

#include "command.h"

#define MAX_TITLE_LEN    	48
#define MAX_FRAME_ITEM  	7
#define INQUIRE_ID      	7

#define CLEAN_UI 	-1

/*
 * row: 1 - 4
 * col: 1 - 12
 */
struct pos {
    int row;
    int col;
};

/*
 * ATTENTION:
 *  next_id : next UI frame id 
 *  action  : responsibily action if this's a end item 
 * 
 * next_id means return_back ui while action is not NULL 
 * CLEAN_UI menas DO NOT show nothing(clean screen) while 
 * action return.
 */
struct next {
    int next_id;
    ACTION  action; 
};

/*
 * id : item id 
 * title : content of item 
 * color : tint or dark
 * pos : position in current screen 
 * next : next UI or to run a handler 
 */
struct item {
    int id;
    char title[MAX_TITLE_LEN];

#define COLOR_TINT  0 
#define COLOR_DARK  1
    int color;

    struct pos pos;
    struct next next;
};

/*
 * used for UI transfer 
 */
struct simple_item {
    char title[MAX_TITLE_LEN];

    int color;
    struct pos pos;
};

/*
 * item_num : 1 - 12
 */
struct frame {
    int id;
    int item_num;

    struct item items[MAX_FRAME_ITEM];
};

/*
 * used for UI transer 
 */
struct simple_frame {
    int item_num;

    struct simple_item items[MAX_FRAME_ITEM];
};


#define CN_SET  0
#define EN_SET  1

#define CURSOR_ON    1 
#define CURSOR_OFF   0

#define CHAR_BLANK   "                        "

#ifndef __UI_MENU__

extern int show_str(int row, int col, char *str);
extern int show_str_dmy_cur(int row, int col, char *str);
extern int display_err_msg(int err, char *msg);
extern int question_user(char *title);
extern int display_info(char *msg);
extern int display_warn(char *msg);
extern int display_str(int row, int col, char *format, ...);

extern int clear_screen(void);

extern int highlight_on(int row);
extern int highlight_off(int row);
extern int set_cursor_on(int row, int col);
extern int set_cursor_off(int row, int col);

extern struct user *get_cur_user(void);
extern int check_userlevel(int level);

extern int check_shutdown(void);
extern int check_logout(void);
extern int set_logout_flag(int flag);
extern int set_shutdown_flag(int flag);
extern int shutdown(void);
extern int log_out(void);
extern int log_in(void);

extern int ui_get_keycode(int *key);
extern int ui_handle_keycode(int key);

extern int show_current_ui(int id);
extern int show_simple_frame(struct simple_frame *frame);

//obsolete api
//extern int set_charset(int charset);
//extern int highlight_pos(int row, int col);

#endif /* ifndef __UI_MENU__ */

#endif /* __UI_API_H__ */
