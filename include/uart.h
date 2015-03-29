/*
 * serial.h = head file of serial port operations
 *  defined APIs of serial ops 
 *
 * Author : Leonardo Physh <leonardo.physh@yahoo.com.hk>
 * Date   : 2014.10.21
 */

#ifndef __UART_H__
#define __UART_H__

extern int open_uart(char *uart, int flag);
extern int read_uart(int fd, unsigned char *buf, int size);
extern int write_uart(int fd, unsigned char *buf, int size);

extern int set_speed(int fd, int speed);
extern int set_parity(int fd, int databits, int stopbits, int parity); 

extern int flush_uart(int fd);
extern int kb_uart_setup(int fd);
extern int card_uart_setup(int fd);

#endif 
