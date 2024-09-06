#include <SPI.h>
#include "RF24.h"
#include "NRF24_device.h"

void TaskReadSerial(void *pvParameters);
void TaskSendToRec(void *pvParameters);
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // 等待串口准备就绪
  }

  rf24_init();

  // 创建读取串口数据任务
  // xTaskCreate(
  //     TaskReadSerial, // 任务函数
  //     "ReadSerial",   // 任务名称
  //     2048,           // 堆栈大小（字节）
  //     NULL,           // 任务参数
  //     1,              // 任务优先级
  //     NULL            // 任务句柄
  // );
  // xTaskCreate(
  //     TaskSendToRec, // 任务函数
  //     "SendToRec",   // 任务名称
  //     2048,          // 堆栈大小（字节）
  //     NULL,          // 任务参数
  //     1,             // 任务优先级
  //     NULL           // 任务句柄
  // );
}
  uint8_t dataToSend[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

void loop()
{
  Serial.print(" "); 
  Serial.print(dataToSend[2] );
  Serial.print(" ");  
  PayloadStruct received = sendAndReceive(dataToSend, sizeof(dataToSend));
  vTaskDelay(pdMS_TO_TICKS(50)); // 短暂延迟，避免过度占用CPU
  received = sendAndReceive(dataToSend, sizeof(dataToSend));
  dataToSend[2] = payload.counter++;

  delay(1000); // 为了使串行监视器的输出更易读，每秒传输一次
}

void TaskReadSerial(void *pvParameters)
{
  while (1)
  {
    uint8_t dataToSend[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    PayloadStruct received = sendAndReceive(dataToSend, sizeof(dataToSend));

    if (received.counter > 0)
    {
      dataToSend[2] = payload.counter++;
    }
    // 7ms是极限 低于7ms会出现无法接收ACK的问题
    vTaskDelay(pdMS_TO_TICKS(7)); // 短暂延迟，避免过度占用CPU
  }
}

void TaskSendToRec(void *pvParameters)
{
  uint8_t send_buffer[4] = {0x01, 0x02, 0x03, 0x04};
  while (1)
  {
    bool report = radio.writeFast(send_buffer, sizeof(send_buffer), false);
    if (report)
    {
      // transmission successful; wait for response and print results
      Serial.println("Transmission successful; waiting for response");
      radio.startListening();                 // put in RX mode
      unsigned long start_timeout = millis(); // timer to detect no response
      while (!radio.available())
      {                                     // wait for response or timeout
        if (millis() - start_timeout > 200) // only wait 200 ms
          break;
      }
      radio.stopListening(); // put back in TX mode

      // print summary of transactions
      if (radio.available())
      { // is there a payload received?

        PayloadStruct received;
        radio.read(&received, sizeof(received)); // get payload from RX FIFO
        payload.counter = received.counter;      // save incoming counter for next outgoing counter
      }
    }
    else
    {
      Serial.println("Transmission failed or timed out");
    } // report
    delay(1000); // slow transmissions down by 1 second
  }
}
