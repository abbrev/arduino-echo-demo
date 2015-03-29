#ifndef SLIP_H
#define SLIP_H

#include <Arduino.h>

unsigned slip_recv_packet(byte *p, unsigned len);
void slip_send_packet(const void *vp, unsigned len);

#endif


