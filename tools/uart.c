/*
 * serial.c - UART module 
 *  - implement of UART ops 
 *  
 * Author : Leonardo Physh 
 * Update Data : 2010.10.22 
 */

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>      
#include <termios.h>    
#include <errno.h>
#include <string.h>

#include "config.h"
#include "common.h"

static int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
    B38400, B19200, B9600, B4800, B2400, B1200, B300, };

static int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300,
    38400,  19200,  9600, 4800, 2400, 1200,  300, };

/**
 * set_speed - set serial port boardrate
 *  @fd : fd
 *  @speed : boardrate
 *  @return: status 
 */
int set_speed(int fd, int speed)
{
    int i;
    int status;
    
    struct termios opt;
    tcgetattr(fd, &opt);
    
    for (i= 0; i < sizeof(speed_arr) / sizeof(int);  i++) {
        if (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&opt, speed_arr[i]);
            cfsetospeed(&opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &opt);
            if  (status != 0) {
                perror("tcsetattr fd1");

                return FAIL;
            }
        }

        tcflush(fd, TCIOFLUSH);
    }

    return SUCCESS;
}

/**
 * set_parity - setup serail databits and stopbits and parityi
 *  @fd : fd    
 *  @databits : databits, can set as 7 or 8
 *  @stopbits : stopbits, can set as 1 or 2
 *  @parity : parity can set as N,E,O,S
 */
int set_parity(int fd, int databits, int stopbits, int parity)
{
    struct termios option;

    if (tcgetattr(fd, &option) !=  0) {
        perror("SetupSerial 1");
        return -1;
    }

    option.c_cflag &= ~CSIZE;
    switch (databits) {
        case 7:
            option.c_cflag |= CS7;
            break;

        case 8:
            option.c_cflag |= CS8;
            break;

        default:
            fprintf(stderr, "Unsupported data size\n");
            return -1;
    }

    switch (stopbits) {
        case 1:
            option.c_cflag &= ~CSTOPB;
            break;

        case 2:
            option.c_cflag |= CSTOPB;
            break;

        default:
            fprintf(stderr,"Unsupported stop bits\n");
            return -1;
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            option.c_cflag &= ~PARENB;    /* Clear parity enable */
            option.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;

        case 'o':
        case 'O':
            option.c_cflag |= (PARODD | PARENB); 
            option.c_iflag |= INPCK;      /* Disnable parity checking */
            break;

        case 'e':
        case 'E':
            option.c_cflag |= PARENB;     /* Enable parity */
            option.c_cflag &= ~PARODD;   
            option.c_iflag |= INPCK;      /* Disnable parity checking */
            break;

        case 'S':
        case 's':  /* as no parity */
            option.c_cflag &= ~PARENB;
            option.c_cflag &= ~CSTOPB;
            break;

        default:
            fprintf(stderr,"Unsupported parity\n");
            return FAIL;
    }
    
    option.c_cc[VTIME] = MAX_CARD_TIMEOUT * 10;
    option.c_cc[VMIN] = 0;

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd,TCSANOW, &option) != 0) {
        perror("SetupSerial 3");
        return FAIL;
    }

    return SUCCESS;
}

/**
 * open_uart - open serial port 
 *  @dev : device pathname
 *  @return : fd
 */
int open_uart(char *dev, int flag)
{
    int fd;

    fd = open(dev, flag | O_NOCTTY);   //| O_NOCTTY | O_NDELAY
    if (-1 == fd) {
        perror("Can't Open Serial Port");
        return FAIL;
    }

    return fd;

}

/*
 * read_uart - read date to uart
 *  @fd  : fd 
 *  @buf : data buffer
 *  @size: size need to read from uart 
 *  @return : count or status   
 */
int read_uart(int fd, uchar *buf, int size) 
{
    int ret;
    uchar ch = 0;
    int offset = 0, loop = 0;

    if (fd < 0) {
        return FAIL;
    }
    
    loop = size + 10;
    
    while (loop > 0) {
        ret = read(fd, &ch, 1);
        if (ret == 1) {
            buf[offset++] = ch;
            if (offset == size)
                return SUCCESS;
            else 
                continue;
        }
        else { 
            if (ret < 0 && errno != EINTR && errno != EAGAIN) {
                debug_msg("\nRead Uart Error!\n");
                return FAIL;
            }
            else 
                loop--;
        }
    }

    return FAIL;
}

/*
 * write_uart - write date to uart
 *  @fd  : fd 
 *  @buf : data buffer 
 *  @size : data size
 *  @return : count or status   
 */
int write_uart(int fd, uchar *buf, int size) 
{
    int ret;

    if (fd < 0) {
        return FAIL;
    }

    ret = write(fd, buf, size); 
    return ret;    
}

/*
 * flush_uart - flush UART I/O buffer 
 *  @return : status 
 */
int flush_uart(int fd)
{
    return tcflush(fd, TCIOFLUSH);
}

/*
 * kb_uart_setup - specail setup for keybard UART 
 *  @kb : keybard fd 
 *  @return : status 
 */
int kb_uart_setup(int fd) 
{
    struct termios option;

    memset(&option, 0, sizeof(option));
    option.c_cflag = B9600|CS8|CLOCAL|CREAD;
    option.c_iflag = IGNPAR;
    option.c_oflag = 0;
    option.c_lflag = 0;
    option.c_cc[VMIN] = 0;
    option.c_cc[VTIME] = 30;

    tcflush(fd, TCIFLUSH);

    if(tcsetattr(fd, TCSANOW, &option) < 0){
        return FAIL;
    }

    return SUCCESS;
}

/*
 * card_uart_setup - specail setup for card reader UART
 *  @fd : card reader fd 
 *  @return : status 
 */
int card_uart_setup(int fd) 
{
#if 1
    struct termios option;

    memset(&option, 0, sizeof(option));
    option.c_cflag = B9600|CS8|CLOCAL|CREAD;
    option.c_iflag = IGNPAR;
    option.c_oflag = 0;
    option.c_lflag = 0;
    option.c_cc[VMIN] = 0;
    option.c_cc[VTIME] = 30;

    tcflush(fd, TCIFLUSH);

    if(tcsetattr(fd, TCSANOW, &option) < 0){
        return FAIL;
    }

#else
    int ret;
    set_speed(fd, 9600);
    ret = set_parity(fd, 8, 1, 'N');
    if (ret < 0)
        return FAIL;
#endif 

    return SUCCESS; 
}



