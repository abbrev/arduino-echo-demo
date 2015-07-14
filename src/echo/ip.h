#ifndef IP_H
#define IP_H

#include <stdint.h>
//#include <Arduino.h>

// XXX avr-gcc fills the LSb in bitfields first.
// Note: all multi-byte fields are Big Endian.

struct ipv4_header {
	uint8_t  ihl:4;
	uint8_t  version:4;
	uint8_t  ecn:2;
	uint8_t  dscp:6;
	uint16_t total_length;

	uint16_t identification;
	uint8_t  fragment_offset_h:5;
	uint8_t  flags:3;
	uint8_t  fragment_offset_l:8;

	uint8_t  ttl;
	uint8_t  proto;
	uint16_t header_checksum;

	uint32_t src;

	uint32_t dst;

	uint8_t  options[0]; // may be 0 or more bytes
};

struct icmp_header {
	uint8_t  type;
	uint8_t  code;
	uint16_t checksum;

	uint32_t rest_of_header;
};

// IPv4 address. Octets are in network byte order so it can be compared
// directly to an in-memory IP address.
#define IP(a,b,c,d) (((uint32_t)(d) << 24) | ((uint32_t)(c) << 16) | ((uint32_t)(b) << 8) | (a))

static inline uint16_t ntohs(uint16_t x)
{
	return (x >> 8) | (x << 8);
}

static inline uint32_t ntohl(uint32_t x)
{
	return ((x & 0xff000000UL) >> 24) |
	       ((x & 0x00ff0000UL) >>  8) |
	       ((x & 0x0000ff00UL) <<  8) |
	       ((x & 0x000000ffUL) << 24);
}

#define htons ntohs
#define htonl ntohl

uint16_t ip_checksum(const void *p, unsigned len, uint16_t sum = 0);
bool is_valid_ipv4_header(const ipv4_header *h, unsigned len);

#if 0
typedef uint64_t time_t;
typedef int32_t suseconds_t;
struct timeval { time_t tv_sec; suseconds_t tv_usec; };
#endif

#endif

