#ifndef __NRF24_DEVICE__
#define __NRF24_DEVICE__
#include <SPI.h>
#include "RF24.h"
#define MOSI_PIN 5
#define MISO_PIN 6
#define SCK_PIN 4
#define CE_PIN 2
#define CSN_PIN 3

extern RF24 radio;
extern uint8_t recv_buffer[32];
extern uint8_t send_buffer[32];
extern uint8_t send_address[][6];
void rf24_init_send();
void rf24_init_recv();
size_t rf24_send(uint8_t *send_buffer, int send_len, uint8_t *recv_buffer);
void rf24_send_only(uint8_t *send_buffer, int send_len);
size_t rf24_recv(uint8_t *recv_buffer, uint8_t *send_buffer, uint8_t send_len);
size_t change_address_send(uint8_t *address, uint8_t *send_buffer, int send_len, uint8_t *recv_buffer);
#endif // !__NR24_DEVICE__
