#ifndef P_P_H
#define P_P_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/*
 * Macros that hold the constants values and 
 * measures of the packets.
 * Measured in bytes.
 * */
#define LENGTH 1
#define DATA_LENGTH 8
#define CRC 1
#define PACKET_LENGTH (LENGTH + DATA_LENGTH + CRC)

/*
 * Fixed hex values that indicate an acknowledgement (ACK)
 * or a request to retrasmit a packet (RCK).
 * In particular this ones get inserted in the data section
 * of a packet, with the rest of bits set to 0xFF.
 * */ 
#define ACK 0x12
#define RCK 0x13

/*
 * Struct that holds the
 * fields for the packets of the 
 * p_p packet protocol.
 * For details, you can look at the 
 * arch.png image.
 * */
typedef struct packet_t {
	uint8_t length;
	uint8_t data[PACKET_LENGTH];
	uint8_t crc;
} packet_t;

/*
 * Function that handles the creation of the packet struct.
 * */
packet_t *create_packet(uint8_t length, uint8_t *data);

/*
 * Computes the rcc (CRC-8 implementation, it uses the polynomial '0x07'
 * */
uint8_t compute_crc(uint8_t length, uint8_t *data);

/*
 * Actually sends the packet, and waits for the response.
 * */
void send_packet(packet_t *p);

/*
 * Sends an ACK packet.
 * */
void send_ack();

/*
 * Sends aan RCK packet.
 * */
void send_rck();

/*
 * Handles a received packet.
 * */
void handle_packet();

void write_byte(uint8_t byte);

uint8_t read_byte();

void print_packet(packet_t *p);

#endif // !P_P_H
