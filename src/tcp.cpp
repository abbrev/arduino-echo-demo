#include "ip.h"
#include "tcp.h"

uint16_t tcp_checksum(const ipv4_header *ip, const tcp_header *tcp)
{
	unsigned total_length = ntohs(ip->total_length);
	byte header_length = ip->ihl * 4;
	//byte tcp_header_length = tcp->data_offset * 4;
	//  unsigned data_length = total_length - header_length - tcp_header_length;

	struct {
		byte zeros;
		byte protocol;
		word tcp_length;
	} pseudo;
	pseudo.zeros = 0;
	pseudo.protocol = 6;
	pseudo.tcp_length = htons(total_length - ip->ihl * 4);

	uint16_t sum;
	sum = ip_checksum(&ip->src, 2 * sizeof ip->src);
	sum = ip_checksum(&pseudo, sizeof pseudo, sum);
	sum = ip_checksum(tcp, total_length - header_length, sum);

	return sum;
}

