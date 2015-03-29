#ifndef IP_H
#define IP_H

//#include <stdint.h>
#include <Arduino.h>

// Arduino doesn't define these types:
typedef uint16_t word;
typedef uint32_t dword;

// XXX avr-gcc fills the LSb in bitfields first.
// Note: all multi-byte fields are Big Endian.

struct ipv4_header {
	byte ihl:4;
	byte version:4;
	byte ecn:2;
	byte dscp:6;
	word total_length;

	word identification;
	byte fragment_offset_h:5;
	byte flags:3;
	byte fragment_offset_l:8;

	byte ttl;
	byte proto;
	word header_checksum;

	dword src;

	dword dst;

	byte options[0]; // may be 0 or more bytes
};

struct icmp_header {
	byte type;
	byte code;
	word checksum;

	dword rest_of_header;
};

// IPv4 address. Octets are in network byte order so it can be compared
// directly to an in-memory IP address.
#define IP(a,b,c,d) (((dword)(d) << 24) | ((dword)(c) << 16) | ((dword)(b) << 8) | (a))

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

