/*
 * Tax Card Base Code
 *   -- Defined the ways how CPU and IC card communicate.
 * 
 * Author : Leonardo Physh 
 * Date   : 2014.7.30  Rev 01
 */

#include <unistd.h> 
#include <stdlib.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "error.h"

#include "tax_cmd.h"
#include "tax_system.h"
#include "tax_card_base.h"

#include "serial.h"


int err_num = 0;

/*
 * Global card reader device 
 */
static int card_dev = 0;

/*
 * card_debug - ouput data to terminal 
 *  @buf : data to ouput 
 *  @buf_size : buffer size 
 */
static int card_debug(uchar * buf, int buf_size)
{
    int i;
    int ret;

    debug_msg("\nBuffer : \n");
    for (i = 0; i < buf_size; i ++){
        ret = debug_msg("%x ", buf[i]);
        if (ret < 0)
            return FAIL;
    }
    debug_msg("\nEnd\n");

    return SUCCESS;
}

static int print_res_data(struct card_res_buf * buf)
{
    int i;
    int len;
    uchar *p;

    debug_msg("\nBuffer: \n");
    
    p = (uchar *)buf;

    for (i = 0; i < 3; i++) {
        debug_msg("%x ", *p);
        p++;
    }

    len = buf->cmd_len - 2;
    if (len == 0) {
        p = &buf->chksum;
        debug_msg("%x \n", *p);
        return SUCCESS;
    } else {
        len = len - 2;
        p = (uchar *)(&buf->res_buf);
        for (i = 0; i < len; i++) {
            debug_msg("%x ", *p);
            p++;
        }
        
        debug_msg("%x ", buf->res_buf.sw1);
        debug_msg("%x ", buf->res_buf.sw2);
        debug_msg("\nEND\n");

        return SUCCESS;
    }
}

/* 
 * Global send_buf & res_buf
 *
 * tax base code will share this two structures, so please clear 
 * data in this two structures befor use them.
 */
static struct card_send_buf *send_buf;
static struct card_send_buf *get_send_buf(void)
{
    if (send_buf != NULL)
        return send_buf;

    send_buf = (struct card_send_buf *)malloc(sizeof(struct card_send_buf));
    if (!send_buf) {
        err_num = -ETAX_NOMEM;
        return NULL;
	}
	
    return send_buf;
}

static struct card_res_buf * res_buf;
static struct card_res_buf * get_res_buf(void)
{
    if (res_buf != NULL)
        return res_buf;

    res_buf = (struct card_res_buf *)malloc(sizeof(struct card_res_buf));
    if (!res_buf) {
        err_num = -ETAX_NOMEM;
        return NULL;
	}
	
    return res_buf;	
}


/*
 * fill_chksum - used to calculate checksum before send a command
 *
 *  @send_buf : buffer data need to do check sum
 *  @return : value of check sum
 */
static int fill_chksum(struct card_send_buf *send_buf)
{
    if (!send_buf)
        return -ETAX_NUL_SEND_BUF;

    uchar *ptr;
    ptr = (uchar *)send_buf;

    int buf_size;
    int i, sum = 0;

#ifdef CONFIG_CMDHEAD_4BYTE
    buf_size = send_buf->cmd_len - 1 + 4;
#else
    buf_size = send_buf->cmd_len - 1 + 1;  
#endif 

    for (i = 0; i <= buf_size; i++) {
        sum += *ptr;
        ptr++;
    }

    return (uchar)sum;
}

/*
 * check_res_data - do check sum for card response data, 
 *                  check whether data has transfer error
 *  
 *  @res_buf : responce data need to do check sum 
 *  @return  : bool
 */
static int check_res_data(struct card_res_buf *res_buf)
{
    if (!res_buf)
        return -ETAX_NUL_RES_BUF;

    uchar *ptr;
    ptr = (uchar *)res_buf;

    int buf_size;
    int i, sum = 0;

    /* check head */
    buf_size = 3;
    for (i = 0; i < buf_size; i++) {
        sum += *ptr;
        ptr++;
    }

    /*
     * POWER ON
     * POWER OFF 
     * those commands get no data
     */
    if (res_buf->cmd_len == 0x02)
        goto checkdone;

    buf_size = send_buf->cmd_len - 4;

    for (i = 0; i < buf_size; i++) {
        sum += *ptr;
        ptr++;
    }
    sum += res_buf->res_buf.sw1;
    sum += res_buf->res_buf.sw2;

checkdone:
    return (((uchar)(sum)) == res_buf->chksum);
}

/*
 * prepare_cmd - initialize send buffer
 * 
 *   @CARD : card type <user card or fiscal card>
 *   @CMD  : command type <refer tax_cmd.h>
 *   @size : the data size of cmd_buf 
 *   @cmd_buf : the APDU format data
 *   @return : send buffer initialized
 */
static struct card_send_buf * prepare_cmd(int CARD, int CMD, int size, 
        struct apdu_cmd_send * cmd_buf)
{
    struct card_send_buf *send_buf;

    send_buf = get_send_buf();
    memset(send_buf, 0, sizeof(struct card_send_buf));

    switch(CARD)
    {
        case FISCAL_CARD:
            send_buf->cmd_type = 0x50;
            goto prepare;
            break;

        case USER_CARD:
            send_buf->cmd_type = 0xF0;
            goto prepare;
            break;

        default:
            err_num = -ETAX_BAD_CARD_TYPE;
            return NULL;
            break;
    }

prepare:
    send_buf->cmd_head[0] = 0x02;

#ifdef CONFIG_CMDHEAD_4BYTE
    send_buf->cmd_head[1] = 0x55;
    send_buf->cmd_head[2] = 0xAA;
    send_buf->cmd_head[3] = 0x96;
#endif 

    switch(CMD)
    {
        case CARD_OS_CMD:
            if (!cmd_buf || size == 0) {
                err_num = -ETAX_BADAPDU;
                return NULL;
            }

            send_buf->cmd_len = size + 2;
            send_buf->cmd_type += 0;
            send_buf->cmd_buf = *cmd_buf;
            send_buf->chksum = fill_chksum(send_buf);
            break;

        case CARD_RST_CMD:
            send_buf->cmd_len = 2;
            send_buf->cmd_type += 1;
            fill_chksum(send_buf);
            break;

        case CARD_PWR_ON_CMD:
            send_buf->cmd_len = 2;
            send_buf->cmd_type += 2;
            fill_chksum(send_buf);
            break;

        case  CARD_PWR_OFF_CMD:
            send_buf->cmd_len = 2;
            send_buf->cmd_type += 3;
            fill_chksum(send_buf);
            break;

        default:
            err_num = -ETAX_BAD_CMD_TYPE;
            return NULL;
            break;
    }

    return send_buf;
}


/* 
 * card_send_cmd - method to send a command to IC Card 
 *  
 *  @send_buf: data to be send 
 *  @return : length of response data
 */
static int card_send_cmd(struct card_send_buf *send_buf)
{
    
    if (card_dev == 0)
        return -ETAX_CARD_NOT_INIT;

    if (!send_buf)
        return -ETAX_NUL_SEND_BUF;

    uchar *ptr;
    ptr = (uchar *)send_buf;

    int ret, buf_size;
    uchar buf[MAX_WRBUF_SIZE];

#ifdef CONFIG_CMDHEAD_4BYTE
    buf_size = send_buf->cmd_len - 1 + 4;
#else
    buf_size = send_buf->cmd_len - 1 + 1;
#endif      

    /*
     * we can fill buf like this way beacause struct card_send_buf
     * is continous in memory.
     */
    int i;
    for (i = 0; i <= buf_size; i++) {
        buf[i++] = *ptr;
        ptr++;
    }
    buf[i] = send_buf->chksum;
    
    /*
     * for debug 
     */
    card_debug(buf, buf_size + 1);

    /*
     * buf_size + 1: for adding chksum
     */
    ret = write(card_dev, buf, buf_size + 1);
    if (ret < 0) {
#ifdef CONFIG_CARD_DEBUG
        card_debug(buf, buf_size + 1);
#endif 
        return -ETAX_WRITE_CARD;
    }

    return buf_size;
}


/* 
 * card_get_res - method to get response data from IC Card 
 *  
 *  @res_buf : response data buffer
 *  @return : bool
 */
static int card_get_res(struct card_res_buf * res_buf)
{
    int ret;	
    int try_count;
    int buf_size;

    uchar buf[MAX_WRBUF_SIZE];    

    if (!res_buf)
        return -ETAX_NUL_RES_BUF;

    memset(res_buf, 0, sizeof(*res_buf));

    buf_size = 2;
    memset(buf, 0, MAX_WRBUF_SIZE);

    try_count = 2;

get_head:   
    ret = read(card_dev, buf, buf_size);
    if (ret < 0) {
        if (errno != EAGAIN) {
            return -ETAX_RD_CARD;
        }
        /*
         * while get a EAGAIN errno, that means there is no data
         * for reading, however we don't quit function for this 
         * time, but try twrice more.
         */
        if (try_count == 0)
            return -ETAX_RD_CARD;

        try_count--;  
        DELAY(100);
        goto get_head;
    }

    if (buf[0] != 0x50)
        return -ETAX_BAD_RES_HEAD;

    res_buf->cmd_head = buf[0];
    res_buf->cmd_len = buf[1];

    buf_size = res_buf->cmd_len;
    memset(buf, 0, MAX_WRBUF_SIZE);

    try_count = 2;

get_data:
    ret = read(card_dev, buf, buf_size);
    if (ret < 0) {
        if (errno != EAGAIN)
            return -ETAX_RD_CARD;
        /*
         * while get a EAGAIN errno, that means there is no data
         * for reading, however we don't quit function for this 
         * time, but after try twrice more.
         */
        if (try_count == 0)
            return -ETAX_RD_CARD;

        try_count--;    
        DELAY(100);
        goto get_data;
    }

    if (buf[0] != FC_OS_CMD && buf[0] != UC_OS_CMD) {
#ifdef CONFIG_CARD_DEBUG
        card_debug(buf, buf_size);
#endif 
        return -ETAX_BAD_CMD_TYPE;
    }

    res_buf->cmd_type = buf[0];

    /*
     * POWER ON 
     * POWER OFF
     * thoese commands get no data
     */
    if (res_buf->cmd_len == 0x02) {
        res_buf->chksum = buf[1];   //in this case, cmd_len suposed to be 0x02
        goto checksum;
    }

    /* for so far, we get response data in buf[], parse it */
    buf_size = res_buf->cmd_len - 4;
    memcpy(res_buf->res_buf.data, buf + 1, buf_size);
    res_buf->res_buf.sw1 = buf[buf_size + 1];
    res_buf->res_buf.sw2 = buf[buf_size + 2];
    res_buf->chksum = buf[buf_size + 3];

checksum: 
    /*
     * for debug 
     */     
    print_res_data(res_buf);

    /* check response data with check sum*/
    ret = check_res_data(res_buf);
    if (!ret)
        return -ETAX_BAD_CHKSUM;

    return res_buf->cmd_len + 1;
}


/********************************************************** 
 * Following is the base command functions
 *  -- DO NOT EDIT THIS --
 * 	-- FOR DETAIL, REFER TO GB-1803 --
 *********************************************************/

/* CMD : SELECT FILE */
static int card_select_file_by_df(int CARD, const char *file_name)
{
    int ret;
    int cmd_size;

    if (!file_name)
        return -ETAX_WR_FILENAME;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xA4;
    cmd_buf.P1 = 0x04;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = strlen(file_name);
    memcpy(cmd_buf.cond_body + 1, file_name, cmd_buf.cond_body[0]);
    cmd_size = cmd_buf.cond_body[0] + 5;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 : 90 6A 6A 6A
     * SW2 : 00 81 82 86 
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x81)
            return -ETAX_CARD_PIN_LOCK;

        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;

        return -ETAX_BAD_SW;
    }

    return SUCCESS;
}

/* CMD : SELECT FILE */
static int card_select_file_by_id(int CARD, int file_id)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xA4;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 2;
    cmd_buf.cond_body[1] = (uchar)(file_id >> 8);
    cmd_buf.cond_body[2] = (uchar)(file_id);
    cmd_size = cmd_buf.cond_body[0] + 5;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 : 90 6A 6A 6A
     * SW2 : 00 81 82 86 
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x81)
            return -ETAX_CARD_PIN_LOCK;
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;

        return -ETAX_BAD_SW;
    }

    return SUCCESS;
}

/*
 * CMD : READ_BINARY
 * Notice this:
 *  -- Read Binnary in GB can select file by short file ID with P1 set as 0x8x;
 * but we were not going to support this function.
 *     So, whenever you want to use this function, plz select the file you want 
 * to read before invoke it.
 * 
 */
static int card_read_binary(int CARD, int offset, int size, uchar *res)
{
    int ret;
    int cmd_size;

    assert(res_buf != NULL);

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    /* We were not intend to support select file by short file ID */

    /* reand current file */
    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xB0;
    offset &= 0xFFFF;
    cmd_buf.P1 = (uchar)(offset >> 8);
    cmd_buf.P2 = (uchar)offset;
    cmd_buf.cond_body[0] = (uchar)size;
    cmd_size = 5;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) { 
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82) 
            return -ETAX_NO_CARD_FILE;

        return -ETAX_BAD_SW;
    }


    /* copy data to recive buffer */
    memcpy(res_buf, res_buf->res_buf.data, size);

    return SUCCESS;
}

/*
 * CMD : READ_BINARY
 * Notice this:
 *  -- Read Binnary in GB can select file by short file ID with P1 set as 0x8x;
 * but we were not going to support this function.
 *     So, whenever you want to use this function, plz select the file you want 
 * to read before invoke it.
 * 
 */
static int card_update_binary(int CARD, int offset, int size, uchar *res)
{
    int ret;
    int cmd_size;

    assert(res_buf != NULL);

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    /* We were not intend to support select file by short file ID */

    /* update current file */
    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xD6;
    offset &= 0xFFFF;
    cmd_buf.P1 = (uchar)(offset >> 8);
    cmd_buf.P2 = (uchar)offset;
    cmd_buf.cond_body[0] = (uchar)size;
    memcpy(cmd_buf.cond_body + 1, res, size);
    cmd_size = 5 + size;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) { 
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82) 
            return -ETAX_NO_CARD_FILE;

        return -ETAX_BAD_SW;
    }

    return SUCCESS;
}

/*
 * CMD : READ_RECORD
 * Notice this:
 *  -- Read Binnary in GB can select file by short file ID with P1 set as 0x8x;
 * but we were not going to support this function.
 *     So, whenever you want to use this function, plz select the file you want 
 * to read before invoke it.
 */
static int card_read_record(int CARD, uchar record_nb, int size, uchar *res)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xB2;
    cmd_buf.P1 = record_nb;
    cmd_buf.P2 = 0x04;
    cmd_buf.cond_body[0] = (uchar)size;
    cmd_size = 4 + size;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90
     * SW2 00
     */
    if (!(res_buf->res_buf.sw1 == 0x90 && res_buf->res_buf.sw2 == 0x00)) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x83) 
            return -ETAX_NO_CARD_REC;

        return -ETAX_BAD_SW;
    }

    memcpy(res_buf, res_buf->res_buf.data, size);

    return SUCCESS;
}

/*
 * CMD : UPDATE_RECORD
 * Notice this:
 *  -- Read Binnary in GB can select file by short file ID with P1 set as 0x8x;
 * but we were not going to support this function.
 *     So, whenever you want to use this function, plz select the file you want 
 * to read before invoke it.
 */
static int card_update_record(int CARD, uchar record_nb, int size, uchar *res)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0xDC;
    cmd_buf.P1 = record_nb;
    cmd_buf.P2 = 0x04;
    cmd_buf.cond_body[0] = (uchar)size;
    memcpy(cmd_buf.cond_body + 1, res, size);
    cmd_size = 5 + size;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90
     * SW2 00
     */
    if (!(res_buf->res_buf.sw1 == 0x90 && res_buf->res_buf.sw2 == 0x00)) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x83) 
            return -ETAX_NO_CARD_REC;

        return -ETAX_BAD_SW;
    }

    return SUCCESS;
}

/*
 * CMD : EXTERNAL_AUTHENTICATION
 */
static int card_external_authentication(int CARD, uchar key_id, uchar *serect)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0x82;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = key_id;
    cmd_buf.cond_body[0] = 0x08;
    memcpy(cmd_buf.cond_body + 1, serect, 8);
    cmd_size = 5 + 0x08;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 90
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00)
        return -ETAX_BAD_SW;

    return SUCCESS;
}

/*
 * CMD : GET CHALLENGE
 */
static int card_get_challenge(int CARD, uchar size, uchar *randon)
{
    int ret;
    int cmd_size;


    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0x00;
    cmd_buf.INS = 0x84;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = size;

    cmd_size = 5;

    send_buf = prepare_cmd(CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00)
        return -ETAX_BAD_SW;

    int i;
    for (i = 0; i < size; i++)
        *randon++ = res_buf->res_buf.data[i];

    return SUCCESS;
}

/*
 * CMD : GET_REGISTER_NB 
 *  -- This is a specical command for fiscal card
 */
static int card_get_register_nb(struct register_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF0;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x10;
    cmd_size = 5;

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90 6A 93 
     * SW2 00 82 03
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;
        if (res_buf->res_buf.sw1 == 0x93 && res_buf->res_buf.sw2 == 0x03)
            return -ETAX_FC_PIN_LOCK;

        return -ETAX_BAD_SW;
    }

    memcpy(info->random, res_buf->res_buf.data, 4);
    memcpy(info->card_nb, res_buf->res_buf.data + 4, 8);
    memcpy(info->mac1, res_buf->res_buf.data + 12, 4);

    return SUCCESS;
}

/*
 * CMD : TERMINAL REGISTER 
 *  -- This is a specical command for fiscal card
 */
static int card_terminal_register(struct register_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF1;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x04;
    memcpy(cmd_buf.cond_body + 1, info->mac2, 4); 
    cmd_buf.cond_body[5] = 0x08;
    cmd_size = 10;

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90 6A 93 
     * SW2 00 82 03
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;
        if (res_buf->res_buf.sw1 == 0x93 && res_buf->res_buf.sw2 == 0x03)
            return -ETAX_FC_PIN_LOCK;

        return -ETAX_BAD_SW;
    }

    /* Save user password*/
    int i;
    for (i = 0; i < 8; i++) {
        info->pin[i] = res_buf->res_buf.data[i];
    }        

    return SUCCESS;
}

/*
 * CMD : ISSUE INVOICE 
 *  -- This is a specical command for fiscal card
 */
static int card_issue_invoice(struct issue_invoice *info, struct issue_invoice_res * res)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF2;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x2C;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info));
    cmd_buf.cond_body[0x2C + 1] = 0x08;
    cmd_size = 6 + sizeof(*info);

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 61 6A 93 
     * SW2 08 82 03
     */
    if (res_buf->res_buf.sw1 != 0x61 && res_buf->res_buf.sw2 != 0x08) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;
        if (res_buf->res_buf.sw1 == 0x93 && res_buf->res_buf.sw2 == 0x03)
            return -ETAX_FC_PIN_LOCK;
        if (res_buf->res_buf.sw1 == 0x94 && res_buf->res_buf.sw2 == 0x01)
            return -ETAX_MON_OUT_RANG;

        return -ETAX_BAD_SW;
    }

    /* save respon data */
    int i;
    for (i = 0; i < 8; i++) {
        res->half_top[i] = res_buf->res_buf.data[i]; // Be careful with this code
    }

    return SUCCESS;
}

/*
 * CMD : DECLARE_DUTY
 * -- This is special command fur fiscal card
 */
static int card_declare_duty(struct declare_duty * info, struct declare_res * res)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF4;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x47;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info));
    cmd_buf.cond_body[0x47 + 1] = 0x8E;
    cmd_size = 6 + sizeof(*info);

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 61 
     * SW2 8E
     */
    if (res_buf->res_buf.sw1 != 0x61 && res_buf->res_buf.sw2 != 0x8E) {
        if (res_buf->res_buf.sw1 == 0x6A && res_buf->res_buf.sw2 == 0x82)
            return -ETAX_NO_CARD_FILE;
        if (res_buf->res_buf.sw1 == 0x93 && res_buf->res_buf.sw2 == 0x03)
            return -ETAX_FC_PIN_LOCK;

        return -ETAX_BAD_SW;
    }

    /*check xor */
    uchar xor_value = 0;
    int i;
    for (i = 0; i < 0x8E - 1; i++) {
        xor_value ^= res_buf->res_buf.data[i];
    }

    if (xor_value != res_buf->res_buf.data[i])
        return -ETAX_BAD_XOR;

    /* save respon data */
    for (i = 0; i < 0x8E; i++) {
        res->total_valid[i] = res_buf->res_buf.data[i]; // Be careful with this code
    }

    return SUCCESS; 
}


/*
 * CMD : UPDATE_CONTROLS
 * -- This is special command fur fiscal card
 */ 
static int card_update_controls(struct update_ctl_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF6;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x1D;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info));
    cmd_size = 5 + sizeof(*info);

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90 
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00)
        return -ETAX_BAD_SW;

    return SUCCESS; 
}

/*
 * CMD : INPUT_INVOICE_NB
 * -- This is special command fur fiscal card
 */
static int card_input_invoice_nb(struct invoice_roll_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF7;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x16;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info));
    cmd_size = 5 + sizeof(*info);

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /*
     * SW1 90 
     * SW2 00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00)
        return -ETAX_BAD_SW;

    return SUCCESS; 
}


/*
 * CMD : VERIFY_FISCAL_PIN
 * -- This is special command fur fiscal card
 */
static int card_verify_fiscal_pin(uchar *old_pin, uchar * new_pin)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xF9;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x08;
    memcpy(cmd_buf.cond_body + 1, old_pin, 8);
    cmd_buf.cond_body[0x08 + 1] = 0x8E;
    cmd_size = 6 + 0x08;

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 90 69
     * SW2 00 83
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x69 && res_buf->res_buf.sw2 == 0x83)
            return -ETAX_FC_PIN_LOCK;
        else 
            return -ETAX_BAD_SW;
    }

    /* save new pin*/
    int i;
    for (i = 0; i < 8; i++)
        new_pin[i] = res_buf->res_buf.data[i];

    return SUCCESS;
}

/*
 * CMD : DAILY_COLLECT_SIGN
 * -- This is special command for fiscal card
 */
static int card_daily_collect_sign(struct daily_collect * info, struct daily_collect_res * res)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf; 

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xFB;
    cmd_buf.P1 = 0x00;

#ifdef CONFIG_CARD_DEBUG 
    assert(info != NULL);
    assert(res != NULL);
#else 
    if (!info)  
        return -ETAX_NUL_SEND_BUF;

    if (!res)
        return -ETAX_NUL_RES_BUF;
#endif 

    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x41;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info));
    cmd_buf.cond_body[0x41 + 1] = 0x81;
    cmd_size = 6 + sizeof(*info);

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 61 
     * SW2 81
     */
    if (res_buf->res_buf.sw1 != 0x61 && res_buf->res_buf.sw2 != 0x81)
        return -ETAX_BAD_SW;

    /*check xor */
    uchar xor_value = 0;
    int i;
    for (i = 0; i < 0x81 - 1; i++) {
        xor_value ^= res_buf->res_buf.data[i];
    }

    if (xor_value != res_buf->res_buf.data[i])
        return -ETAX_BAD_XOR;

    /* save respon data */
    for (i = 0; i < 0x81; i++) {
        res->sign[i] = res_buf->res_buf.data[i]; // Be careful with this code
    }

    return SUCCESS; 
}


/*
 * CMD : DATA_COLLECT_SIGN
 *   fiscal card commond 
 */
static int card_data_collect_sign(struct data_collect * info, struct daily_collect_res * res)
{
#if 0
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf; 

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xFB;
    cmd_buf.P1 = 0x00;

#ifdef CONFIG_CARD_DEBUG
    assert(info != NULL);
    assert(res != NULL);
#else  
    if (!sign)
        return -ETAX_NUL_SEND_BUF;

    if (!res)
        return -ETAX_NUL_RES_BUF;
#endif  /* CONFIG_CARD_DEBUG */ 

    cmd_buf.P2 = 0x01;
    cmd_buf.cond_body[0] = 0x81;
    memcpy(cmd_buf.cond_body + 1, sign, 128 + 1);
    cmd_buf.cond_body[0x81 + 1] = 0x81;
    cmd_size = 6 + 128 + 1;

    send_buf = prepare_cmd(FISCAL_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 61 
     * SW2 81
     */
    if (res_buf->res_buf.sw1 != 0x61 && res_buf->res_buf.sw2 != 0x81)
        return -ETAX_BAD_SW;

    /*check xor */
    uchar xor_value = 0;
    int i;
    for (i = 0; i < 0x81 - 1; i++) {
        xor_value ^= res_buf->res_buf.data[i];
    }

    if (xor_value != res_buf->res_buf.data[i])
        return -ETAX_BAD_XOR;

    /* save respon data */
    for (i = 0; i < 0x81; i++) {
        res->sign[i] = res_buf->res_buf.data[i]; // Be careful with this code
    }

#endif 

    return SUCCESS; 
}


/*
 * CMD : REGISTER_SIGN 
 *  -- This is a special command for user card
 */
static int card_register_sign(struct register_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xE4;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x10;
    memcpy(cmd_buf.cond_body + 1, info->random, 4); 
    memcpy(cmd_buf.cond_body + 5, info->card_nb, 8);
    memcpy(cmd_buf.cond_body + 13,info->mac1, 4);
    cmd_buf.cond_body[17] = 0x04;
    cmd_size = 18;

    send_buf = prepare_cmd(USER_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 0x61
     * SW2 0x04 
     */
    if (res_buf->res_buf.sw1 != 0x61 && res_buf->res_buf.sw2 != 0x04)
        return -ETAX_BAD_SW;

    /* save mac2 */
    info->mac2[0] = res_buf->res_buf.data[0];
    info->mac2[1] = res_buf->res_buf.data[1];
    info->mac2[2] = res_buf->res_buf.data[2];
    info->mac2[3] = res_buf->res_buf.data[3];

    return SUCCESS;
}

/*
 * CMD : DATA_COLLECT
 *  -- This is a special command for user card
 */
static int card_data_collect(struct data_collect * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xE6;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0xDC;
    memcpy(cmd_buf.cond_body + 1, (uchar *)info, sizeof(*info)); 

    cmd_size = 5 + sizeof(*info);

    send_buf = prepare_cmd(USER_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 0x90
     * SW2 0x00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00)
        return -ETAX_BAD_SW;

    return SUCCESS;
}

/*
 * CMD : ECHOPLEX_CONTROLS
 *  -- This is a special command for user card
 */
static int card_echoplex_controls()
{
    /* THIS COMMAND IS TAX MANAGER SPECICAL COMMOND, WE DO NOT NEED TO ACHIVE IT */
    return SUCCESS;
}


/*
 * CMD : DISTRIBUTE_INVOICE_NB
 *  -- This is a special command for user card
 */
static int card_distribute_invoice_nb(struct invoice_roll_info * info)
{
    int ret;
    int cmd_size;

    struct apdu_cmd_send cmd_buf;
    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    cmd_buf.CLA = 0xC0;
    cmd_buf.INS = 0xE9;
    cmd_buf.P1 = 0x00;
    cmd_buf.P2 = 0x00;
    cmd_buf.cond_body[0] = 0x00;
    cmd_buf.cond_body[1] = 0x16;

    cmd_size = 6;

    send_buf = prepare_cmd(USER_CARD, CARD_OS_CMD, cmd_size, &cmd_buf);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;
    /*
     * SW1 0x90
     * SW2 0x00
     */
    if (res_buf->res_buf.sw1 != 0x90 && res_buf->res_buf.sw2 != 0x00) {
        if (res_buf->res_buf.sw1 == 0x94 && res_buf->res_buf.sw2 != 0x01) 
            return -ETAX_UC_EMPTY;

        return -ETAX_BAD_SW;
    }
    /* save invoice information */
    int i;
    for (i = 0; i < sizeof(*info); i++)
        info->invoice_code[i] = res_buf->res_buf.data[i];

    return SUCCESS;
}


/* CMD : POWER ON */
static int card_power_on(int CARD)
{
    int ret;

    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    struct fiscal_card * card;


    if (CARD == FISCAL_CARD) {
        card = get_fiscal_card();
    } else if (CARD == USER_CARD) {
        card = (struct fiscal_card *)get_user_card();
    } else {
        return -ETAX_BAD_CARD_TYPE;
    }

    if (card->power_mark == POSITIVE)
        return SUCCESS;

    send_buf = prepare_cmd(CARD, CARD_PWR_ON_CMD, 0, NULL);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /* set card power mark */
    card->power_mark = POSITIVE;    

    return SUCCESS;
}

/* CMD : POWER OFF */
static int card_power_off(int CARD)
{
    int ret;

    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    struct fiscal_card * card;

    if (CARD == FISCAL_CARD) {
        card = get_fiscal_card();
    } else if (CARD == USER_CARD) {
        card = (struct fiscal_card *)get_user_card();
    } else {
        return -ETAX_BAD_CARD_TYPE;
    }

    if (card->power_mark == NEGATIVE)
        return SUCCESS;

    send_buf = prepare_cmd(CARD, CARD_PWR_OFF_CMD, 0, NULL);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    /* set card power mark */
    card->power_mark = NEGATIVE;

    return SUCCESS;
}

/* CMD : POWER RESET */
static int card_power_rst(int CARD)
{
    int ret;

    struct card_send_buf *send_buf;
    struct card_res_buf *res_buf;

    send_buf = prepare_cmd(CARD, CARD_RST_CMD, 0, NULL);
    if (send_buf == NULL)
        return -ETAX_NUL_SEND_BUF;

    ret = card_send_cmd(send_buf);
    if (ret < 0)
        return ret;

    res_buf = get_res_buf();
    ret = card_get_res(res_buf);
    if (ret < 0)
        return ret;

    if (res_buf->res_buf.data[0] != 0x3B && res_buf->res_buf.data[1] != 0x7A)
        return -ETAX_BAD_SW;

    return SUCCESS;
}

/*
 * card_device_init - open card read used uart
 *  @return : status 
 */
int card_device_init(void)
{
    int ret;
    int fd;

    fd = open_dev(CARD_DEV, O_RDWR);
    if (fd < 0) {
        debug_msg("Card Device Init Failed\n");
        return -ETAX_OPEN_CARD;
    }

    card_dev = fd;

    return SUCCESS;
}

/*
 * card_device_stop - close card reader used uart 
 *  @return : status 
 */
int card_device_stop(void)
{
    if (card_dev == 0)
        return SUCCESS;

    close(card_dev);

    return SUCCESS;
}



/* INTERFACE TO OTHER MODULE */
static struct fiscal_card fiscal_card_base = {
    /* staus mark*/
    .power_mark = NEGATIVE,

#ifdef CONFIG_CARD_DEBUG
    .card_send_buf = get_send_buf,
    .card_res_buf = get_res_buf,
#endif 

    .device_init = card_device_init,
    .device_stop = card_device_stop,

    /* COMMON FUNCTION */
    .power_on = card_power_on,
    .power_off = card_power_off,
    .power_rst = card_power_rst,
    .select_file_by_df = card_select_file_by_df,
    .select_file_by_id = card_select_file_by_id,
    .read_binary = card_read_binary,
    .update_binary = card_update_binary,
    .read_record = card_read_record,
    .update_record = card_update_record,
    .external_auth = card_external_authentication,
    .get_challenge = card_get_challenge,


    /* FISCAL CARD FUNCTION */
    .get_register_nb =  card_get_register_nb,
    .terminal_register =  card_terminal_register,
    .issue_invoice =  card_issue_invoice,
    .declare_duty = card_declare_duty,
    .update_controls =  card_update_controls,
    .input_invoice_nb =  card_input_invoice_nb,
    .verify_fiscal_pin = card_verify_fiscal_pin,
    .daily_collect_sign =  card_daily_collect_sign,
    .data_collect_sign = card_data_collect_sign,
};

static struct user_card user_card_base = {
    /* staus mark*/
    .power_mark = NEGATIVE,

#ifdef CONFIG_CARD_DEBUG
    .card_send_buf = get_send_buf,
    .card_res_buf = get_res_buf,
#endif 

    .device_init = card_device_init,
    .device_stop = card_device_stop,

    /* COMMON FUNCTION */
    .power_on = card_power_on,
    .power_off = card_power_off,
    .power_rst = card_power_rst,
    .select_file_by_df = card_select_file_by_df,
    .select_file_by_id = card_select_file_by_id,
    .read_binary = card_read_binary,
    .update_binary = card_update_binary,
    .read_record = card_read_record,
    .update_record = card_update_record,
    .external_auth = card_external_authentication,
    .get_challenge = card_get_challenge,

    /* USER CARD FUNCTION */
    .register_sign = card_register_sign,
    .data_collect = card_data_collect,
    .distribute_invoice_nb = card_distribute_invoice_nb,
};

/*API for other modules */
struct fiscal_card * get_fiscal_card(void)
{
    return &fiscal_card_base;
}

struct user_card * get_user_card(void)
{
    return &user_card_base;
}

/* TAX BASE OVER REV_01*/

/*
 * Following is test code 
 */
#if 1

#define PORT_0      "/dev/ttySAC0"
#define PORT_1      "/dev/ttySAC1"
#define PORT_2      "/dev/ttySAC2"
#define PORT_3      "/dev/ttySAC3"

int main(void)
{
    int fd;

    int option, g_port;

    uchar ch;

    printf("CARD TEST TOOL(2014-10-23)\n");
    printf("------------------\n");
    printf("1- %s\n", PORT_0);
    printf("2- %s\n", PORT_1);
    printf("3- %s\n", PORT_2);
    printf("4- %s\n", PORT_3);
    printf("------------------\n");
    printf("CARD Used which serial port?(1-4):");

    scanf("%d", &g_port);
    while ((ch = getchar()) != '\n' && ch != EOF);

    switch (g_port) {
        case 1:
            fd = open_dev(PORT_0, O_RDWR);
            if (fd < 0)
                return FAIL;

            break;

        case 2:
            fd = open_dev(PORT_1, O_RDWR);
            if (fd < 0)
                return FAIL;

            break;

        case 3:
            fd = open_dev(PORT_2, O_RDWR);
            if (fd < 0)
                return FAIL;

            break;

        case 4:
            fd = open_dev(PORT_3, O_RDWR);
            if (fd < 0)
                return FAIL;

            break;

        default:
            return FAIL;
            break;
    }

    while (1)  {
        printf("--------------------\n");
        printf("Card Test Items\n");
        printf("--------------------\n");
        printf("1- UC Power ON\n");
        printf("2- FC Power ON\n");
        printf("3- UC Power OFF\n");
        printf("4- FC Power OFF\n");
        printf("5- UC Power RST\n");
        printf("6- FC Power RST\n");
        printf("--------------------\n");
        printf("Input item num(1-6):");

        scanf("%d", &option);
        while ((ch = getchar()) != '\n' && ch != EOF);

        if (option == 0)
            break;

        switch (option) {
            case 1:
                card_power_on(USER_CARD);
                break;

            case 2:
                card_power_off(FISCAL_CARD);
                break;

            case 3:
                card_power_off(USER_CARD);
                break;

            case 4:
                card_power_off(FISCAL_CARD);
                break;

            case 5:
                card_power_rst(USER_CARD);
                break;

            case 6:
                card_power_rst(FISCAL_CARD);
                break;

            default:
                break;
        }
    }
}

#endif 













