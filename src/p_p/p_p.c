#include "p_p.h"
#include "../../inc/peripherals.h"

static packet_t created_packet;  // Static instance to hold the created packet

packet_t *create_packet(uint8_t length, uint8_t *data) {
    // Check if the provided length is valid
    if (length > DATA_LENGTH) {
        return NULL;
    }

    // Set the packet fields
    created_packet.length = length;

    // Copy the data into the packet
    for (uint8_t i = 0; i < length; ++i) {
        created_packet.data[i] = data[i];
    }

    // Pad the remaining data with 0xFF
    for (uint8_t i = length; i < DATA_LENGTH; ++i) {
        created_packet.data[i] = 0xFF;
    }

    // Compute and set the CRC
    created_packet.crc = compute_crc(length, data);

    return &created_packet;
}

uint8_t compute_crc(uint8_t length, uint8_t *data) {
    uint8_t crc = 0;

    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void write_byte(uint8_t byte) {
    // We check if there are any data that is being transferred 
    // using the USART_SR register, if the bit 7 is 1, it means that 
    // the data has finished writing.
    // If so, we can use the USART_DR register to write the new data.
    //
    // Section 19.6.1
    while(!((USART2->USART_SR & (1 << 7)))); 

    // Sets the data register to the ASCII code of the 
    // character 'x' by bitwise ANDing it with 0xFF
    // to ensure only the lower 8 bits (byte that we wanna transmit) are considered.
    // To see the actual bytes sent, I used picocom, but any dumb-terminal emulation works
    // fine:
    //
    // picocom -b 9600 /dev/ttyACM0.
    USART2->USART_DR = (byte & 0xFF); 

    // We wait for the transmission to be completed.
    while(!(USART2->USART_SR & (1 << 6)));
}

void send_packet(packet_t *p) {
    // Send packet over UART
    write_byte(p->length);
    for (uint8_t i = 0; i < DATA_LENGTH; ++i) {
        write_byte(p->data[i]);
    }
    write_byte(p->crc);
}

void send_ack() {
    // Create an ACK packet
    uint8_t test_data[] = {ACK};
    packet_t ack_packet = *create_packet(sizeof(test_data), test_data);

    // Send the ACK packet over UART
    write_byte(ack_packet.length);
    read_byte();

    for (uint8_t i = 0; i < DATA_LENGTH; ++i) {
        write_byte(ack_packet.data[i]);
    }

    write_byte(ack_packet.crc);
}

void send_rck() {
    // Create an ACK packet
    uint8_t test_data[] = {RCK};
    packet_t rck_packet = *create_packet(sizeof(test_data), test_data);

    // Send the ACK packet over UART
    write_byte(rck_packet.length);

    for (uint8_t i = 0; i < DATA_LENGTH; ++i) {
        write_byte(rck_packet.data[i]);
    }

    write_byte(rck_packet.crc);
}

uint8_t read_byte() {
    // We check if there are any data that is being transferred 
    // using the USART_SR register, if the bit 5 is 1, it means that 
    // the data has finished writing.
    // If so, we can return the USART_DR register to read new data.
    //
    // Section 19.6.1
    while(!((USART2->USART_SR & (1 << 5))));

    return USART2->USART_DR;
}

void handle_packet() {
    for (uint8_t i = 0; i < PACKET_LENGTH; ++i) {
        uint8_t byte = read_byte();
        write_byte(byte);
    }
}
