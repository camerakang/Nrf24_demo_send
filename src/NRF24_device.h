#ifndef __NRF24_DEVICE__
#define __NRF24_DEVICE__
#include <SPI.h>
#include "RF24.h"

#define MAX_BUFFER_SIZE 256 // 定义一个最大缓冲区大小

// nRF24L01引脚配置
#define SS_PIN 3
#define MOSI_PIN 5
#define MISO_PIN 6
#define SCK_PIN 4

#define CE_PIN 2
#define CSN_PIN 3
extern RF24 radio;
struct PayloadStruct
{
  char message[12]; // 传输的消息
  uint32_t counter; // 计数器
};

#define MAX_PAYLOAD_SIZE 32

struct ReceivedData
{
  uint8_t pipe;
  uint8_t size;
  uint8_t data[MAX_PAYLOAD_SIZE];
  unsigned long interval;
};
extern PayloadStruct payload;
extern PayloadStruct ackPayload;
void rf24_init();
PayloadStruct sendAndReceive(const uint8_t *dataToSend, int dataLength);
ReceivedData handleRadioReceive(PayloadStruct *ackPayload, uint8_t ackSize);
#endif // !__NR24_DEVICE__
