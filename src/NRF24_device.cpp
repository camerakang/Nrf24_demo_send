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
    radio.stopListening();                // 设置为发送模式
    radio.printDetails();
    radio.setChannel(100);
    payload.counter = 0; // 初始化计数器

    printf_begin();
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
 */
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
        if (!radio.txStandBy(100)) // 等待最多100ms
        {
            Serial.println(F("Transmission failed"));
            return receivedData;
        }
    }

    // 数据发送成功
    Serial.print(F("Sent "));
    Serial.print(dataLength);
    Serial.print(F(" bytes"));

    // 检查是否有返回数据
    uint8_t pipe;
    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getDynamicPayloadSize();
        if (bytes == sizeof(receivedData))
        {
            radio.read(&receivedData, sizeof(receivedData));
            Serial.print(F("Received "));
            Serial.print(bytes);
            Serial.print(F(" bytes  "));
            Serial.print(receivedData.message);
            Serial.println(receivedData.counter);
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