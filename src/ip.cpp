#include "ip.h"

// XXX assumes len is even
uint16_t ip_checksum(const void *p, unsigned len, uint16_t sum)
{
	const word *w = (const word *)p;
	dword dsum = sum;
	for (; len >= 2; len -= 2) {
		dsum += *w++;
	}
	if (len) {
		// one odd byte
		union {
			byte b[2];
			word w;
		} x = { { *(const uint8_t *)w, 0 } };
		dsum += x.w;
	}
	// handle carry
	while (dsum > 0xffffU) {
		word n = dsum >> 16;
		dsum &= 0xffffU;
		dsum += n;
	}
	return dsum;
}

bool is_valid_ipv4_header(const ipv4_header *ip, unsigned len)
{
	// minimum length is 20 octets
	if (len < 20) return false;

	// we support only version 4
	if (ip->version != 4) return false;

	unsigned total_length = ntohs(ip->total_length);

	// total length must be >= 20
	if (total_length < 20) return false;

	// total length cannot be longer than packet
	if (total_length > len) return false;

	byte header_length = ip->ihl * 4;

	// header length cannot be larger than total length
	if (header_length > total_length) return false;

	//// verify checksum
	if (ip_checksum(ip, header_length) != 0xffffU) return false;

	return true;
}

