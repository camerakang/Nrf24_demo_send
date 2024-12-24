#include "NRF24_device.h"

uint8_t recv_buffer[32]{"recv buffer is empty"};
uint8_t send_buffer[32]{"send buffer is empty"};

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);
SPIClass rf24_spi(HSPI);

// an identifying device destination
// Let these addresses be used for the pair
uint8_t address[][6] = {"1Node", "2Node"};
uint8_t send_address[][6] = {"3Node", "4Node", "5Node", "6Node"};

void rf24_init_send()
{

    rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

    // initialize the transceiver on the SPI bus
    if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // to use ACK payloads, we need to enable dynamic payload lengths (for all nodes)
    radio.enableDynamicPayloads(); // ACK payloads are dynamically sized

    // Acknowledgement packets have no payloads by default. We need to enable
    // this feature for all nodes (TX & RX) to use ACK payloads.
    radio.enableAckPayload();

    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(send_address[1]); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[0]); // using pipe 1

    radio.stopListening(); // this also discards any unused ACK payloads
    // For debugging info
    // printf_begin(); // needed only once for printing details
    radio.printDetails(); // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}
void rf24_init_recv()
{
    rf24_spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

    // initialize the transceiver on the SPI bus
    if (!radio.begin(&rf24_spi, CE_PIN, CSN_PIN))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }

    // Set the PA Level low to try preventing power supply related problems
    // because these examples are likely run with nodes in close proximity to
    // each other.
    radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

    // to use ACK payloads, we need to enable dynamic payload lengths (for all nodes)
    radio.enableDynamicPayloads(); // ACK payloads are dynamically sized

    // Acknowledgement packets have no payloads by default. We need to enable
    // this feature for all nodes (TX & RX) to use ACK payloads.
    radio.enableAckPayload();
    radio.setAutoAck(true);
    // set the TX address of the RX node into the TX pipe
    radio.openWritingPipe(address[1]); // always uses pipe 0

    // set the RX address of the TX node into a RX pipe
    radio.openReadingPipe(1, address[0]); // using pipe 1

    // load the payload for the first received transmission on pipe 0
    radio.writeAckPayload(1, send_buffer, sizeof(send_buffer));
    radio.startListening();
    // For debugging info
    // printf_begin(); // needed only once for printing details
    // radio.printDetails();       // (smaller) function that prints raw register values
    // radio.printPrettyDetails(); // (larger) function that prints human readable data
}
size_t rf24_send(uint8_t *send_buffer, int send_len, uint8_t *recv_buffer)
{
    radio.setAutoAck(true);
    unsigned long start_timer = micros(); // start the timer
    Serial.print(F("Sending data: "));
    for (size_t i = 0; i < send_len; i++)
    {
        Serial.print(send_buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    bool report = radio.writeFast(send_buffer, send_len); // transmit & save the report
    unsigned long end_timer = micros();                   // end the timer

    if (report)
    {
        uint8_t pipe;
        if (radio.available(&pipe))
        { // is there an ACK payload? grab the pipe number that received it
            size_t bytes = radio.getDynamicPayloadSize();
            radio.read(recv_buffer, bytes); // get incoming ACK payload
            return bytes;
        }
        else
        {
            Serial.println(F(" Received: an empty ACK packet")); // empty ACK packet received
            return 0;
        }
    }
    else
    {
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
        return 0;
    }
}
void change_address_send(uint8_t *address, uint8_t *send_buffer, int send_len, uint8_t *recv_buffer)
{
    // 打印address
    radio.openWritingPipe(address); // always uses pipe 0
    auto recv_len = rf24_send(send_buffer, send_len, recv_buffer);
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
void rf24_send_only(uint8_t *send_buffer, int send_len)
{
    radio.setAutoAck(false);
    radio.writeFast(send_buffer, send_len); // transmit & save the report
}

size_t rf24_recv(uint8_t *recv_buffer, uint8_t *send_buffer, uint8_t send_len)
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {                                                  // is there a payload? get the pipe number that received it
        uint8_t bytes = radio.getDynamicPayloadSize(); // get the size of the payload
        radio.read(recv_buffer, bytes);                // get incoming payload
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(recv_buffer[i], HEX);
        // }
        // Serial.println();
        // Serial.print("Sending ACK payload...");
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(send_buffer[i], HEX);
        // }
        // Serial.println();
        radio.writeAckPayload(1, send_buffer, send_len);
        return bytes;
    }
    return 0;
}

size_t rf24_recv_only(uint8_t *recv_buffer)
{
    uint8_t pipe;
    if (radio.available(&pipe))
    {                                                  // is there a payload? get the pipe number that received it
        uint8_t bytes = radio.getDynamicPayloadSize(); // get the size of the payload
        radio.read(recv_buffer, bytes);                // get incoming payload
        // Serial.print(F("Received "));
        // Serial.print(bytes); // print the size of the payload
        // Serial.print(F(" bytes on pipe "));
        // Serial.print(pipe); // print the pipe number
        // // Serial.print(F(": "));
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(recv_buffer[i], HEX);
        // }
        // Serial.println();
        // Serial.print("Sending ACK payload...");
        // for (uint8_t i = 0; i < bytes; i++)
        // {
        //     Serial.print(send_buffer[i], HEX);
        // }
        // Serial.println();
        return bytes;
    }
    return 0;
}