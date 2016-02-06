/*
 * UART.h
 *
 * Created: 02.02.2016 01:23:49
 *  Author: Timm
 */ 

#ifndef UART_H_
#define UART_H_

typedef void (*rx_callback_t)(char* buffer, char length);

extern volatile char uart_enabled;
extern volatile char uart_rx_active;
extern volatile char uart_tx_active;

void uart_init(void);
void uart_disable(void);
void uart_enable(void);
void uart_send_data(char* data, char size);
void uart_send_string(char* data);
void uart_set_rx_callback(rx_callback_t callback);

#endif /* UART_H_ */