Arduino Echo Server
===================

This is a silly little demo of a networked "server" that runs on an
Arduino. It responds to ICMP Echo Requests and to TCP segments on ports
7 (the echo service) and 9 (the discard service). This means that you
can "ping" your Arduino or connect to it on port 7 or 9 with TCP. Other
TCP ports are refused, and other protocols (eg, UDP) are ignored.

This program uses the UART as a network interface using the plain SLIP
protocol.


ICMP Echo
---------

The "ping" utility found on most operating systems sends ICMP echo
requests to the destination host and waits for ICMP echo replies. This
demo sends a reply for every valid request that it receives. All data
received in the request is sent back.


TCP Echo
--------

The TCP Echo protocol echoes all data received. This demo responds to
all TCP connections on port 7, echoing all data received.


TCP Discard
-----------

The TCP Discard protocol discards all data received. This demo responds
to all TCP connections on port 9, discarding all data received.


Other TCP ports
---------------

This demo responds to connections on all other TCP ports with an RST
segment, which is interpreted as "connection refused".


Configuring SLIP (Linux)
------------------------

Here is the script I use to set up SLIP using the Arduino's serial
connection:

    #! /bin/sh
    
    speed=57600
    tty=/dev/ttyUSB0
    ip=10.0.7.1
    netmask=255.255.255.0
    mtu=576
    
    # configure serial port as a slip network interface
    slattach -Ldvl -p slip -s $speed $tty &
    sleep 0.5
    # XXX is there a way to make slattach go to the background *after*
    # attaching?
    
    # configure ip/netmask on sl0
    ifconfig sl0 $ip netmask $netmask mtu $mtu

Run it as root (sudo is sufficient). Modify it as needed.

The IP address of the echo server is hardcoded to 10.0.7.2. This may be
changed in echo.ino; with slight modifications the IP address may be
removed so that the echo server responds to datagrams sent to any IP
address.


Implementation notes
--------------------

This server supports the minimum required MTU for TCP (576 bytes). An
RFC-compliant host must assume another host supports an MSS of only 536
bytes (= 576 - 20 - 20) unless otherwise indicated.

There are three levels of data buffers for receiving incoming bytes:

* HardwareSerial: 64 bytes
* slip: 1024 bytes
* echo: 576 bytes

The HardwareSerial and slip buffers contain the raw received bytes;
these are expected to be SLIP-encoded IP datagrams. The echo buffer
contains a decoded IP datagram.

The HardwareSerial receive buffer is polled every time the server is
sending a datagram or reading a new datagram. Thus the HardwareSerial
buffer is largely superfluous (I could have used an interrupt to receive
UART data and put all the data directly into the slip buffer).

The IP and TCP processing steps re-use the IP datagram buffer for a
response, if needed. If a response must be sent, the datagram is
SLIP-encoded and sent directly to the HardwareSerial transmit buffer
without any intervening buffer.


Performance
-----------

Not that it really matters for a silly demo like this, but I tested its
TCP and ping performance.

### TCP Performance

Multiple TCP connections were run in parallel (with each connection
streaming data as fast as possible). Tests were performed in Linux.

<table>
<tr><th>Parallel connections</th><th>Transfer rate</th></tr>
<tr><td>1 </td>                  <td>80%</td></tr>
<tr><td>2 </td>                  <td>80%</td></tr>
<tr><td>5 </td>                  <td>85%</td></tr>
<tr><td>10</td>                  <td>83%</td></tr>
<tr><td>20</td>                  <td>82%</td></tr>
<tr><td>30</td>                  <td>72%</td></tr>
<tr><td>50</td>                  <td>65%</td></tr>
</table>

Transfer rate is the application data transfer rate out of the total
available transfer rate (57600 bps). Note that TCP and IP have overhead
of about 40 bytes per data packet plus 40 bytes per ACK-only packet, so
the theoretical maximum application data transfer rate is about 86% of
the total available transfer rate (CSLIP would alleviate this overhead
somewhat).

TCP summary: application data transfer rates are close to the
theoretical maximum data transfer rate with almost any number of
parallel connections. Transfer rates drop above 20 connections, possibly
due to the high RTT on each individual connection.

### Ping Performance

Pings were done with various packet sizes ("-s" option in the Linux
"ping" command). Response times are largely dominated by the transfer
rate.

Default packet size (56+28 bytes; theoretical minimum is 29.2 ms):

    --- 10.0.7.2 ping statistics ---
    15 packets transmitted, 15 received, 0% packet loss, time 14014ms
    rtt min/avg/max/mdev = 31.037/31.361/31.963/0.315 ms

Minimum packet size (16+28 bytes, the smallest to contain ping timing
information; theoretical minimum is 15.2 ms):

    --- 10.0.7.2 ping statistics ---
    15 packets transmitted, 15 received, 0% packet loss, time 14013ms
    rtt min/avg/max/mdev = 17.154/17.701/18.112/0.354 ms

Maximum packet size (548+28 bytes; theoretical minimum is 200 ms):

    --- 10.0.7.2 ping statistics ---
    15 packets transmitted, 15 received, 0% packet loss, time 14013ms
    rtt min/avg/max/mdev = 202.020/202.560/203.006/0.437 ms

Ping summary: latencies are about 2 milliseconds longer than the
theoretical minimum latencies. This may be due to the FTDI USB-to-serial
converter being used.

