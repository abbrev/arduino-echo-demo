#ifndef TCP_H
#define TCP_H

#include "ip.h"

struct tcp_header {
	uint16_t sport;
	uint16_t dport;

	uint32_t seqno;

	uint32_t ackno;

#if 0
	// not in RFC793
	bool     ns:1;
	uint8_t  reserved1:3;
#else
	uint8_t  reserved1:4;
#endif
	uint8_t  data_offset:4;
	bool     fin:1;
	bool     syn:1;
	bool     rst:1;
	bool     psh:1;
	bool     ack:1;
	bool     urg:1;
#if 0
	// not in RFC793
	bool     ece:1;
	bool     cwr:1;
#else
	uint8_t  reserved2:2;
#endif
	uint16_t window_size;

	uint16_t checksum;
	uint16_t urgptr;

	uint8_t  options[0];
};

uint16_t tcp_checksum(const ipv4_header *ip, const tcp_header *tcp);

#endif



