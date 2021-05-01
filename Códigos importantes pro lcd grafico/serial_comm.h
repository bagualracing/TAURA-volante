/*
 * serial_comm.h
 *
 * Created: Thu, 04, 06, 2015 18:15:03
 *  Author: Carlos R. W. Filho, Bruna Etges, Rodrigo Johann
 */ 


#ifndef SERIAL_COMM_H_
#define SERIAL_COMM_H_

void serial_init();
void USART_send(unsigned char data);
void USART_send_string(char* StringPtr);
void USART_send_int();
void USART_send_float(float variable,int places);

#endif /* SERIAL_COMM_H_ */