/*
 * Linux packet capture implementation using netfilter
 * Replacement for WinDivert functionality on Linux systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <string.h>
#include <errno.h>
#include "linux_packet.h"

static struct nfq_handle *nfq_h = NULL;
static struct nfq_q_handle *qh = NULL;
static int netlink_fd = -1;

// Callback function for handling packets
static int packet_callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                          struct nfq_data *nfa, void *data) {
    struct nfqnl_msg_packet_hdr *ph;
    unsigned char *packet_data;
    int packet_len;
    int packet_id;
    
    ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) {
        packet_id = ntohl(ph->packet_id);
        packet_len = nfq_get_payload(nfa, &packet_data);
        
        if (packet_len >= 0) {
            // Process the packet (this is where DPI bypass logic would go)
            // For now, just accept all packets
            return nfq_set_verdict(qh, packet_id, NF_ACCEPT, 0, NULL);
        }
    }
    
    return 0;
}

int linux_packet_init(uint16_t queue_num) {
    printf("Initializing Linux packet capture (netfilter queue %d)\n", queue_num);
    
    // Open netfilter library handle
    nfq_h = nfq_open();
    if (!nfq_h) {
        fprintf(stderr, "Error: failed to open netfilter queue library handle\n");
        return -1;
    }
    
    // Unbind existing queue handler (if any)
    if (nfq_unbind_pf(nfq_h, AF_INET) < 0) {
        fprintf(stderr, "Warning: failed to unbind netfilter queue handler\n");
    }
    
    // Bind to IPv4
    if (nfq_bind_pf(nfq_h, AF_INET) < 0) {
        fprintf(stderr, "Error: failed to bind netfilter queue handler to AF_INET\n");
        nfq_close(nfq_h);
        return -1;
    }
    
    // Create queue handle
    qh = nfq_create_queue(nfq_h, queue_num, &packet_callback, NULL);
    if (!qh) {
        fprintf(stderr, "Error: failed to create netfilter queue handle\n");
        nfq_close(nfq_h);
        return -1;
    }
    
    // Set copy mode to copy entire packet
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "Error: failed to set netfilter queue copy mode\n");
        linux_packet_cleanup();
        return -1;
    }
    
    // Get file descriptor for the netlink socket
    netlink_fd = nfq_fd(nfq_h);
    if (netlink_fd < 0) {
        fprintf(stderr, "Error: failed to get netlink file descriptor\n");
        linux_packet_cleanup();
        return -1;
    }
    
    printf("Linux packet capture initialized successfully\n");
    printf("Note: You need to set up iptables rules to redirect packets to NFQUEUE\n");
    printf("Example: iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num %d\n", queue_num);
    
    return 0;
}

int linux_packet_loop(int timeout_ms) {
    if (netlink_fd < 0 || !nfq_h) {
        fprintf(stderr, "Error: packet capture not initialized\n");
        return -1;
    }
    
    fd_set fds;
    struct timeval tv;
    char buf[4096];
    int rv;
    
    while (1) {
        FD_ZERO(&fds);
        FD_SET(netlink_fd, &fds);
        
        if (timeout_ms > 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            rv = select(netlink_fd + 1, &fds, NULL, NULL, &tv);
        } else {
            rv = select(netlink_fd + 1, &fds, NULL, NULL, NULL);
        }
        
        if (rv < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        if (rv == 0) {
            // Timeout
            continue;
        }
        
        if (FD_ISSET(netlink_fd, &fds)) {
            rv = recv(netlink_fd, buf, sizeof(buf), 0);
            if (rv >= 0) {
                nfq_handle_packet(nfq_h, buf, rv);
            } else {
                perror("recv");
                break;
            }
        }
    }
    
    return 0;
}

void linux_packet_cleanup(void) {
    if (qh) {
        nfq_destroy_queue(qh);
        qh = NULL;
    }
    
    if (nfq_h) {
        nfq_close(nfq_h);
        nfq_h = NULL;
    }
    
    netlink_fd = -1;
    printf("Linux packet capture cleaned up\n");
}

int linux_packet_send(const void *packet, size_t len) {
    // This would implement packet injection for Linux
    // For now, just log that we would send the packet
    printf("Would send modified packet of %zu bytes\n", len);
    return 0;
}