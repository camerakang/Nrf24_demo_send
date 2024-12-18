#include <SPI.h>
#include "RF24.h"
#include "NRF24_device.h"
#include "simple_protocol_impl.h"
#include "simple_protocol_tpl.h"
SimpleProtocolImpl *sprotocol_send = dynamic_cast<SimpleProtocolImpl *>(new SimpleProtocolTpl<1, 1, true, 32, 0>({0xA5, 0xA5}));
size_t send_len;
bool send_flag = false;
void TaskReadSerial(void *pvParameters);
void TaskSendToRec(void *pvParameters);
void IRAM_ATTR onReceive();

uint8_t dataToSend[32] = {};
uint8_t dataToReceive[32];
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // 等待串口准备就绪
  }
  Serial.setRxTimeout(1);
  Serial.onReceive(onReceive);
  rf24_init_send();

  // 创建读取串口数据任务
  // xTaskCreate(
  //     TaskReadSerial, // 任务函数
  //     "ReadSerial",   // 任务名称
  //     2048,           // 堆栈大小（字节）
  //     NULL,           // 任务参数
  //     1,              // 任务优先级
  //     NULL            // 任务句柄
  // );
}
void loop()
{
  if (send_flag)
  {
    send_flag = false;
    auto frame{sprotocol_send->make_packer(1, 32)};
    frame.push_back(send_buffer, send_len).end_pack();

    auto recv_len = rf24_send(frame().data(), frame().size(), recv_buffer);
    if (recv_len > 0)
    {
      // Serial.print(F("Received "));
      // for (int i = 0; i < recv_len; i++)
      // {
      //   Serial.print(dataToSend[i], HEX);
      //   Serial.print(" ");
      // }
    }
  }

  delay(1); // 为了使串行监视器的输出更易读，每秒传输一次
}

void IRAM_ATTR onReceive()
{
  send_len = Serial.available();
  memset(send_buffer, 0, sizeof(send_buffer));
  if (send_len > 0)
  {
    size_t bytesRead = Serial.read(send_buffer, send_len);
    send_flag = true;
  }
}