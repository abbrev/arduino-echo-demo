#ifndef TCP_H
#define TCP_H

#include "ip.h"

struct tcp_header {
	word sport;
	word dport;

	dword seqno;

	dword ackno;

#if 0
	// not in RFC793
	bool  ns:1;
	byte reserved1:3;
#else
	byte reserved1:4;
#endif
	byte data_offset:4;
	bool fin:1;
	bool syn:1;
	bool rst:1;
	bool psh:1;
	bool ack:1;
	bool urg:1;
#if 0
	// not in RFC793
	bool ece:1;
	bool cwr:1;
#else
	byte reserved2:2;
#endif
	word window_size;

	word checksum;
	word urgptr;

	byte options[0];
};

uint16_t tcp_checksum(const ipv4_header *ip, const tcp_header *tcp);

#endif



