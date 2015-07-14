#ifndef SLIP_H
#define SLIP_H

#include <stdint.h>

unsigned slip_recv_packet(uint8_t *p, unsigned len);
void slip_send_packet(const void *vp, unsigned len);

#endif


