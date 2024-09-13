#include "NRF24_device.h"
#include "printf.h"

PayloadStruct payload;
PayloadStruct ackPayload;

RF24 radio(CE_PIN, CSN_PIN);
#define MAX_BUFFER_SIZE 256 // 定义一个最大缓冲区大小

// 发送地址
uint8_t address[][6] = {"1Node", "2Node"};
uint8_t buffer[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12};

void rf24_init()
{
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
    radio.flush_tx();                     // 清空TX FIFO

    radio.stopListening(); // 设置为发送模式
    radio.setChannel(100);
    payload.counter = 0; // 初始化计数器
    radio.printDetails();

    // Serial.print(F("CRC Rate: "));
    // Serial.println(radio.getCRCLength());
    // Serial.print(F("PAL Rate: "));
    // Serial.println(radio.getPALevel());
    // // getARC
    // Serial.print(F("ARC: "));
    // Serial.println(radio.getARC());
}
/**
 * 发送数据并接收PAYLOAD ACK响应
 * 该函数负责通过无线通信模块发送数据，并接收返回的数据
 *
 * @param dataToSend 要发送的数据指针
 * @param dataLength 要发送的数据长度
 * @return 返回接收到的数据结构
 *
 * 注意: 该函数假设有适当的初始化和错误处理机制存在于调用该函数的上下文中
 * 调用示例
 *  uint8_t dataToSend[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    dataToSend[2] = payload.counter++;
    PayloadStruct received = sendAndReceive(dataToSend, sizeof(dataToSend));

 */
     uint8_t send_times=0;

PayloadStruct sendAndReceive(const uint8_t *dataToSend, int dataLength)
{
    PayloadStruct receivedData = {"", 0};

    if (dataLength <= 0 || dataLength > MAX_BUFFER_SIZE)
    {
        Serial.println(F("Invalid data length"));
        return receivedData;
    }
    if (!radio.writeFast(dataToSend, dataLength, true)) // 使用 writeFast 并启用自动重试
    {
        if (!radio.txStandBy(10)) // 等待最多100ms
        {
            Serial.println(F("Transmission failed"));
            return receivedData;
        }
    }
    // 数据发送成功
    Serial.print(F("Sent :"));
    // Serial.print(dataLength);
    // Serial.print(F(" bytes"));
    // for (size_t i = 0; i < dataLength; i++)
    // {
    Serial.print(dataToSend[2]);
    //     Serial.print(F(" "));
    // }
    send_times++;
    // 检查是否有返回数据
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getDynamicPayloadSize();
        if (bytes == sizeof(receivedData))
        {

            radio.read(&receivedData, sizeof(receivedData));
            Serial.print(F("  Received "));
            Serial.print(receivedData.message);
            Serial.println(receivedData.counter);
            radio.flush_rx();
        }
        else
        {
            Serial.println(F("Received data size mismatch"));
            radio.flush_rx(); // 清空接收缓冲区
        }
    }
    else
    {
        Serial.println(F("No response received"));
    }

    return receivedData;
}

PayloadStruct sendAndReceive_twice(const uint8_t *dataToSend, int dataLength)
{
    PayloadStruct receivedData = {"", 0};

    if (dataLength <= 0 || dataLength > MAX_BUFFER_SIZE)
    {
        Serial.println(F("Invalid data length"));
        return receivedData;
    }

    if (!radio.writeFast(dataToSend, dataLength, true)) // 使用 writeFast 并启用自动重试
    {
        if (!radio.txStandBy(100)) // 等待最多100ms
        {
            Serial.println(F("Transmission failed"));
            return receivedData;
        }
    }

    // 数据发送成功
    // Serial.print(F("Sent "));
    // Serial.print(dataLength);
    // Serial.print(F(" bytes"));
    // for (size_t i = 0; i < dataLength; i++)
    // {
    //     Serial.print(dataToSend[i], HEX);
    //     Serial.print(F(" "));
    // }

    // 检查是否有返回数据
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getDynamicPayloadSize();
        if (bytes == sizeof(receivedData))
        {

            radio.read(&receivedData, sizeof(receivedData));
            // Serial.print(F("Received "));
            // Serial.print(bytes);
            // Serial.print(F(" bytes  "));
            // Serial.print(receivedData.message);
            // Serial.println(receivedData.counter);
        }
        else
        {
            Serial.println(F("Received data size mismatch"));
            radio.flush_rx(); // 清空 接收缓冲区
        }
    }
    else
    {
        Serial.println(F("No response received"));
    }

    return receivedData;
}
unsigned long start_timer = 0;
/**
 * 处理无线电接收功能的函数
 *
 * 该函数负责接收无线电数据，计算接收间隔，并且可发送确认payload
 *
 * @param void *ackPayload 指向需要作为确认回复发送的数据缓冲区
 * @param uint8_t ackSize 确认回复数据的大小
 *
 * @return 返回一个ReceivedData结构体，其中包含接收到的数据、接收管道、数据大小和接收间隔
 * 调用示例
 * memcpy(&ackPayload, "payload ack", sizeof("payload ack"));
  // 调用函数
  ReceivedData receivedData = handleRadioReceive(&ackPayload, sizeof(ackPayload));
 */
ReceivedData handleRadioReceive(PayloadStruct *ackPayload, uint8_t ackSize)
{
    ReceivedData result = {0}; // 初始化接收结果结构体

    // 检查是否有数据可用
    if (radio.available(&result.pipe))
    {
        unsigned long current_time = micros();        // 获取当前时间，用于计算接收间隔
        result.interval = current_time - start_timer; // 计算接收间隔
        start_timer = current_time;                   // 更新计时器起点

        result.size = radio.getDynamicPayloadSize(); // 获取动态payload大小
        // 确保payload大小不会超过最大值
        if (result.size > MAX_PAYLOAD_SIZE)
        {
            result.size = MAX_PAYLOAD_SIZE;
        }

        radio.read(result.data, result.size); // 读取接收到的数据

        // 在串口监视器上输出接收到的数据信息
        Serial.print(F("Received "));
        Serial.print(result.size);
        Serial.print(F(" bytes on pipe "));
        Serial.print(result.pipe);
        Serial.print(F(": "));

        // 遍历并输出接收到的数据
        for (int i = 0; i < result.size; i++)
        {
            Serial.print("0x");
            Serial.print(result.data[i], HEX);
            Serial.print(" ");
        }

        Serial.print(F(" Interval: "));
        Serial.print(result.interval);
        Serial.print(F(" us "));

        // 如果提供了确认payload，则发送它
        if (ackPayload != NULL && ackSize > 0)
        {
            Serial.println(ackPayload->counter);
            ackPayload->counter++; // 假设你想在每次ACK时增加计数器

            radio.writeAckPayload(result.pipe, ackPayload, ackSize);
        }
        start_timer = micros(); // 使用micros()开始计时，用于计算接收间隔
    }

    return result; // 返回接收结果结构体
}

/**
 * 发送数据并等待接收响应
 *
 * @param dataToSend 要发送的数据指针
 * @param sendSize 要发送的数据大小
 * @param received 用于存储接收到的数据的缓冲区
 * @param maxReceiveSize 接收缓冲区的最大大小
 * @return 实际接收到的数据大小，如果发送失败或没有接收到数据则返回0
 */
int sendAndReceive(const uint8_t *dataToSend, uint8_t sendSize, uint8_t *received, uint8_t maxReceiveSize)
{
    radio.stopListening();

    bool report = radio.write(dataToSend, sendSize);
    if (report)
    {
        radio.startListening();
        unsigned long start_timeout = millis();
        while (!radio.available())
        {
            if (millis() - start_timeout > 200)
            { // 200ms超时
                radio.stopListening();
                return 0; // 超时，没有接收到数据
            }
        }

        radio.stopListening();
        if (radio.available())
        {
            uint8_t bytesReceived = radio.getDynamicPayloadSize();
            if (bytesReceived > maxReceiveSize)
            {
                bytesReceived = maxReceiveSize;
            }
            radio.read(received, bytesReceived);

            Serial.print("Received after send: ");
            for (int i = 0; i < bytesReceived; i++)
            {
                Serial.print(received[i], HEX);
                Serial.print(" ");
            }
            Serial.println();

            return bytesReceived;
        }
    }

    return 0; // 发送失败或没有接收到数据
}

#define MAX_BUFFER_SIZE 32 // 假设最大缓冲区大小为32字节，您可以根据需要调整

bool receiveAndRespondData(uint8_t *buffer, uint8_t &length, uint32_t timeout = 150)
{
    radio.startListening();

    unsigned long startTime = millis();
    bool dataReceived = false;

    while (millis() - startTime < timeout)
    {
        if (radio.available())
        {
            length = radio.getDynamicPayloadSize();
            if (length > MAX_BUFFER_SIZE)
            {
                length = MAX_BUFFER_SIZE;
            }
            radio.read(buffer, length);
            dataReceived = true;
            break;
        }
    }

    if (dataReceived)
    {
        Serial.print("TX Received from RX ");
        Serial.print(length);
        Serial.print(" bytes: ");
        for (int i = 0; i < length; i++)
        {
            Serial.print("0x");
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        radio.stopListening();
        bool sent = radio.writeFast(buffer, length);
        bool success = radio.txStandBy(timeout);
        radio.startListening();

        return sent && success;
    }

    return false;
}
