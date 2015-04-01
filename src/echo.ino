/*
 * basic algorithm:
 * 1. receive packet from serial port
 * 2. if the packet is an ICMP echo request to us, change the buffer to change
 *    it to an ICMP echo reply
 * 3. send packet to serial port
 *
 * since we don't have anything else to do while we are receiving or sending a
 * packet, those operations can be blocking.
 */

#include "ip.h"
#include "tcp.h"
#include "slip.h"

void setup()
{
	Serial.begin(57600);
}

// 10.32.8.2
// 10.1.72.2
static const dword myip = IP(10,0,7,2);

static const int led = 13;

#define MTU 576

static void swap_ip_addresses(dword *a, dword *b)
{
	dword t = *a;
	*a = *b;
	*b = t;
}

static void swap_ports(word *a, word *b)
{
	word t = *a;
	*a = *b;
	*b = t;
}

void handle_tcp_echo(ipv4_header *ip, tcp_header *tcp)
{
	unsigned total_length = ntohs(ip->total_length);
	//byte header_length = ip->ihl * 4;
	byte tcp_header_length = tcp->data_offset * 4;
	//unsigned segment_length = total_length - header_length - tcp_header_length;

	// Acknowledge a SYN to start a connection.
	// This isn't strictly necessary--it will be a "simultaneous open" and
	// will result in more segments if we don't do this.
	if (tcp->syn) {
		//tcp->mss = ??

		tcp->ack = 1;
		tcp->ackno = htonl(ntohl(tcp->seqno) + 1);
	}

	// keep the pipe filled!
	tcp->window_size = htons(16384);

	// set all options to end-of-option-list
	if (tcp_header_length > 20)
		tcp->options[0] = 0;
	//memset(tcp->options, 0, tcp_header_length - 20);

	swap_ip_addresses(&ip->dst, &ip->src);
	swap_ports(&tcp->dport, &tcp->sport);

	tcp->checksum = 0;
	tcp->checksum = ~tcp_checksum(ip, tcp);

	/*
	ip->header_checksum = 0;
	ip->header_checksum = ~ip_checksum(ip, header_length);
	*/

	slip_send_packet(ip, total_length);
}

void handle_tcp_discard(ipv4_header *ip, tcp_header *tcp)
{
	unsigned total_length = ntohs(ip->total_length);
	byte header_length = ip->ihl * 4;
	byte tcp_header_length = tcp->data_offset * 4;
	unsigned segment_length = total_length - header_length - tcp_header_length;

	// Acknowledge a SYN to start a connection.
	if (tcp->syn || tcp->fin) {
		tcp->ack = 1;
	} else if (segment_length == 0) {
		// nothing to ack
		return;
	}

	uint32_t ackno = ntohl(tcp->seqno) + segment_length + tcp->syn + tcp->fin;

	tcp->seqno = tcp->ackno;

	tcp->ackno = htonl(ackno);

	// keep the pipe filled!
	tcp->window_size = htons(16384);

	// shrink the TCP header to the minimum
	tcp->data_offset = 20 / 4;
	tcp_header_length = 20;
	total_length = header_length + tcp_header_length;
	ip->total_length = htons(total_length);

	swap_ip_addresses(&ip->dst, &ip->src);
	swap_ports(&tcp->dport, &tcp->sport);

	tcp->checksum = 0;
	tcp->checksum = ~tcp_checksum(ip, tcp);

	ip->header_checksum = 0;
	ip->header_checksum = ~ip_checksum(ip, header_length);

	slip_send_packet(ip, total_length);
}

void handle_tcp(ipv4_header *ip, void *data)
{
	unsigned total_length = ntohs(ip->total_length);
	byte header_length = ip->ihl * 4;

	tcp_header *tcp = (tcp_header *)data;
	byte tcp_header_length = tcp->data_offset * 4;

	if (tcp_header_length < 20) return;

	if (tcp->dport == htons(7)) {
		return handle_tcp_echo(ip, tcp);
	} else if (tcp->dport == htons(9)) {
		return handle_tcp_discard(ip, tcp);
	}

	if (tcp->syn) {
		swap_ip_addresses(&ip->dst, &ip->src);
		swap_ports(&tcp->dport, &tcp->sport);

#if 0
		// send a SYN+ACK
		tcp->syn = 1;
		tcp->rst = 0;
#else
		// send a RST+ACK
		tcp->syn = 0;
		tcp->rst = 1;
#endif
		dword seqno = ntohl(tcp->seqno);
		tcp->ackno = htonl(seqno + 1);

		tcp->ack = 1;
		tcp->ackno = htonl(ntohl(tcp->seqno) + 1);
		tcp->seqno = htonl(0);

		// shrink the segment
		tcp_header_length = 20;
		tcp->data_offset = tcp_header_length / 4;
		total_length = (unsigned)header_length + tcp_header_length;
		ip->total_length = htons(total_length);

		tcp->checksum = 0;
		tcp->checksum = ~tcp_checksum(ip, tcp);

		ip->header_checksum = 0;
		ip->header_checksum = ~ip_checksum(ip, header_length);

		slip_send_packet(ip, total_length);
		return;
	}
}

void handle_icmp(ipv4_header *ip, void *data)
{
	unsigned total_length = ntohs(ip->total_length);

	byte header_length = ip->ihl * 4;

	icmp_header *icmp = (icmp_header *)data;

	// type must be 8 (Echo Request)
	if (icmp->type != 8 || icmp->code != 0) return;

	//if (--ip->ttl == 0) return;

	// Packet is a valid ICMP Echo Request. Change it to an ICMP Echo Reply.

	icmp->type = 0;

// XXX debugging
//icmp->rest_of_header = htonl(ip->total_length);

	// XXX we can update the checksum in-place
	// XXX Linux doesn't seem to care that this checksum is wrong
	icmp->checksum = 0;
	icmp->checksum = ~ip_checksum(icmp, total_length - header_length);

	swap_ip_addresses(&ip->dst, &ip->src);

	// XXX we can update the checksum in-place
	ip->header_checksum = 0;
	ip->header_checksum = ~ip_checksum(ip, header_length);

	slip_send_packet(ip, total_length);
}

void loop()
{
	byte packet[MTU];

	unsigned len = slip_recv_packet(packet, sizeof packet);
	digitalWrite(led, HIGH);

	ipv4_header *ip = (ipv4_header *)packet;

	if (!is_valid_ipv4_header(ip, len)) return;

	// is this packet destined to us? TODO or to the broadcast IP?
	if (ip->dst != myip) return;

	byte header_length = ip->ihl * 4;

	void *data = &packet[header_length];

	switch (ip->proto) {
		case 0x01: handle_icmp(ip, data); break;
		case 0x06: handle_tcp(ip, data); break;
	}

	digitalWrite(led, LOW);
}


