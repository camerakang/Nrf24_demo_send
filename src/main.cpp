#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "printf.h"
#include "NRF24_device.h"
#define MAX_BUFFER_SIZE 256 // 定义一个最大缓冲区大小



// 发送地址
uint8_t address[][6] = {"1Node", "2Node"};
uint8_t buffer[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12};

void TaskReadSerial(void *pvParameters);
void TaskSendToRec(void *pvParameters);
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // 等待串口准备就绪
  }
  // printf_begin();
  // radio_spi->begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  if (!radio.begin())
  {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
    {
    } // 进入无限循环
  }
  radio.setCRCLength(RF24_CRC_8);
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(2, 5);
  radio.setPALevel(RF24_PA_HIGH);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(address[1]);    // 设置发送地址
  radio.openReadingPipe(1, address[3]); // 设置接收地址 (用于ACK)
  radio.stopListening();                // 设置为发送模式
  radio.printDetails();
  radio.setChannel(100);
  payload.counter = 0; // 初始化计数器

  Serial.print(F("CRC Rate: "));
  Serial.println(radio.getCRCLength());
  Serial.print(F("PAL Rate: "));
  Serial.println(radio.getPALevel());
  // getARC
  Serial.print(F("ARC: "));
  Serial.println(radio.getARC());

  // 创建读取串口数据任务
  xTaskCreate(
      TaskReadSerial, // 任务函数
      "ReadSerial",   // 任务名称
      2048,           // 堆栈大小（字节）
      NULL,           // 任务参数
      1,              // 任务优先级
      NULL            // 任务句柄
  );
  // xTaskCreate(
  //     TaskSendToRec, // 任务函数
  //     "SendToRec",   // 任务名称
  //     2048,          // 堆栈大小（字节）
  //     NULL,          // 任务参数
  //     1,             // 任务优先级
  //     NULL           // 任务句柄
  // );
}

void loop()
{

  // unsigned long start_timer = micros(); // 开始计时
  // // bool report = radio.write(&payload, sizeof(payload)); // 发送数据并保存结果
  // bool report = radio.writeFast(buffer, sizeof(buffer), false);
  // unsigned long end_timer = micros(); // 结束计时

  // if (report)
  // {
  //   Serial.print(end_timer - start_timer); // 打印传输时间
  //   Serial.print(F(" us. \t Sent: "));
  //   for (int i = 0; i < sizeof(buffer); i++)
  //   {
  //     Serial.print("0x");
  //     Serial.print((buffer[i]));
  //     Serial.print(" ");
  //   }
  //   buffer[0]++;
  //   uint8_t pipe;
  //   if (radio.available(&pipe))
  //   { // 检查是否有ACK负载
  //     PayloadStruct received;
  //     radio.read(&received, sizeof(received)); // 读取ACK负载
  //     Serial.print(F(" Received "));
  //     Serial.print(radio.getDynamicPayloadSize()); // 打印ACK负载大小
  //     Serial.print(F(" bytes on pipe "));
  //     Serial.print(pipe); // 打印接收的管道编号
  //     Serial.print(F(": "));
  //     Serial.print(received.message);   // 打印收到的消息
  //     Serial.println(received.counter); // 打印收到的计数器
  //   }
  //   else
  //   {
  //     Serial.println(F(" Received: an empty ACK packet")); // 收到空ACK包
  //     // payload.counter++;                                   // 如果没有收到ACK负载，也递增计数器
  //   }
  // }
  // else
  // {
  //   radio.txStandBy(10);
  //   Serial.println(F("Transmission failed or timed out")); // 传输失败
  // }

  delay(1000); // 为了使串行监视器的输出更易读，每秒传输一次
}

void TaskReadSerial(void *pvParameters)
{
  while (1)
  {
    uint8_t dataToSend[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    dataToSend[2] = payload.counter++;
    PayloadStruct received = sendAndReceive(dataToSend, sizeof(dataToSend));

    if (received.counter > 0)
    {

    }
    //7ms是极限 低于7ms会出现无法接收ACK的问题
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

