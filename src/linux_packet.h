/*
 * Linux packet capture header
 * Replacement for WinDivert functionality on Linux systems
 */

#ifndef LINUX_PACKET_H
#define LINUX_PACKET_H

#include <stdint.h>
#include <stddef.h>

// Initialize Linux packet capture using netfilter queue
// queue_num: NFQUEUE queue number to use (0-65535)
// Returns 0 on success, -1 on error
int linux_packet_init(uint16_t queue_num);

// Start packet capture loop
// timeout_ms: timeout in milliseconds (0 = no timeout)
// Returns 0 on clean exit, -1 on error
int linux_packet_loop(int timeout_ms);

// Clean up packet capture resources
void linux_packet_cleanup(void);

// Send a modified packet back to the network
// packet: packet data to send
// len: length of packet data
// Returns 0 on success, -1 on error
int linux_packet_send(const void *packet, size_t len);

#endif /* LINUX_PACKET_H */