
/**********************************************************************
 * file:  sr_router.c 
 * date:  Mon Feb 18 12:50:42 PST 2002  
 * Contact: casado@stanford.edu 
 *
 * Description:
 * 
 * This file contains all the functions that interact directly
 * with the routing table, as well as the main entry method
 * for routing. 11
 * 90904102
 **********************************************************************/

#include <stdio.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_pwospf.h"
#include "pwospf_protocol.h"

/* Local includes  */
#include "my_struct.h"
#include "my_thread.h"
#include "my_method.h"

#define ICMP_TYPE_ECHO_REPLY  0
#define ICMP_TYPE_ECHO_REQ    8

#define DROP                 0
#define SEND_ARP_REQUEST     1
#define SEND_ARP_REPLY       2
#define FORWARD_IP           3
#define SEND_ICMP_REPLY      4

struct arp_node *arp_cache_head = NULL;
struct packet_queue_node *queue_head = NULL;
struct packet_queue_node *queue_tail = NULL;
struct pending_packet_counter *pending_queue_head = NULL;
struct pending_packet_counter *pending_queue_tail = NULL;

/*---------------------------------------------------------------------
 * Method: sr_init(void)
 * Scope:  Global
 *
 * Initialize the routing subsystem
 *
 *---------------------------------------------------------------------*/

void sr_init(struct sr_instance *sr) {
    /* REQUIRES */
    assert(sr);

    /* Add initialization code here! */

    /*pwospf_init(sr);*/

} /* -- sr_init -- */


/*---------------------------------------------------------------------
 * Method: sr_handlepacket(uint8_t* p,char* interface)
 * Scope:  Global
 *
 * This method is called each time the router receives a packet on the
 * interface.  The packet buffer, the packet length and the receiving
 * interface are passed in as parameters. The packet is complete with
 * ethernet headers.
 *
 * Note: Both the packet buffer and the character's memory are handled
 * by sr_vns_comm.c that means do NOT delete either.  Make a copy of the
 * packet instead if you intend to keep it around beyond the scope of
 * the method call.
 *
 * @addendum - @author Aditya Kousik
 * Yank the MAC addresses of the destination and source, and type
 * and pass it along to handle_frame for further processing.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance *sr,
                     uint8_t *packet,
                     unsigned int len,
                     char *interface) {
    /* REQUIRES */
    assert(sr);
    assert(packet);
    assert(interface);

    char *source_MAC, *dest_MAC;
    source_MAC = interface;
    dest_MAC = interface;

//    printf("\n*** -> Received packet of length %d \n", len);

    struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) packet;
    uint8_t *payload = (uint8_t *) (packet + sizeof(struct sr_ethernet_hdr));
    uint16_t type = ntohs(ethernet_hdr->ether_type);

    int to_do_action = -1;
    struct ip *ip_hdr = (struct ip *) payload;
    struct in_addr next_hop;
    next_hop.s_addr = 0;
    if (type == ETHERTYPE_ARP) {
        to_do_action = handle_arp(sr, payload, dest_MAC);
    } else if (type == ETHERTYPE_IP && ip_hdr->ip_p != IPPROTO_OSPF) {
        to_do_action = handle_ip(sr, ethernet_hdr->ether_dhost, payload, &dest_MAC, &next_hop);
    } else {
        handle_pwospf(sr, packet, len, dest_MAC);
    }

    pthread_mutex_lock(&count_mutex);
    if (to_do_action == SEND_ARP_REPLY) {
        send_arp_reply(sr, ethernet_hdr, packet, len, source_MAC);
    } else if (to_do_action == SEND_ARP_REQUEST) {
        send_arp_request(sr, ethernet_hdr, packet, len, dest_MAC, next_hop);
    } else if (to_do_action == FORWARD_IP) {
        send_ip_forward(sr, ethernet_hdr, packet, len, dest_MAC);
    } else if (to_do_action == SEND_ICMP_REPLY) {
        send_icmp_reply(sr, ethernet_hdr, packet, len, source_MAC);
    }
    pthread_mutex_unlock(&count_mutex);
}/* end sr_handlepacket */

uint8_t *make_a_new_arp_packet(unsigned char *local_mac, uint32_t sip, uint32_t tip) {
    uint8_t broadcast_mac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct sr_ethernet_hdr *ethernet_hdr = malloc(sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr));
    struct sr_arphdr *arp_hdr = (struct sr_arphdr *) ((uint8_t *) ethernet_hdr + sizeof(struct sr_ethernet_hdr));
    /* Arp header fields */
    arp_hdr->ar_hln = ETHER_ADDR_LEN;
    arp_hdr->ar_pln = IP_ADDR_LEN;
    arp_hdr->ar_hrd = htons(ARPHDR_ETHER);
    arp_hdr->ar_pro = htons(ETHERTYPE_IP);
    arp_hdr->ar_op = htons(ARP_REQUEST);
    mac_copy(arp_hdr->ar_sha, local_mac);
    mac_empty(arp_hdr->ar_tha);
    arp_hdr->ar_tip = tip;
    arp_hdr->ar_sip = sip;
    /* Ethernet header fields */
    mac_copy(ethernet_hdr->ether_dhost, broadcast_mac);
    mac_copy(ethernet_hdr->ether_shost, local_mac);
    ethernet_hdr->ether_type = htons(ETHERTYPE_ARP);
    return (uint8_t *) ethernet_hdr;
}

void *thread_handler(void *arg) {
    struct thread_counter *args = (struct thread_counter *) arg;
    struct pending_packet_counter *cur = pending_queue_head;
    struct packet_queue_node *curPacket = queue_head;
    int len = sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr);
    while (cur && cur->ip != args->ip) {
        cur = cur->next;
    }
    cur->thread_count.id = args->id;
    cur->thread_count.ip = args->ip;
    cur->thread_count.thread = args->thread;

    while (curPacket && curPacket->dst_ip.s_addr != args->ip) {
        curPacket = curPacket->next;
    }
    uint8_t *newPacket = make_a_new_arp_packet(args->mac_vrhost, args->ip_vrhost, args->ip);
    while (cur->received == 0 && cur->numPacketsSent <= MAX_ARP_REQ && curPacket) {
        sr_send_packet(curPacket->sr, newPacket, len, curPacket->interface);
        cur->numPacketsSent++;
        if (cur->received == 0)
            usleep(2000);
    }
    if (cur->numPacketsSent >= MAX_ARP_REQ) {
        if (curPacket) {
            pthread_mutex_lock(&count_mutex);
            packet_queue_dequeue(curPacket);
            pthread_mutex_unlock(&count_mutex);
            cur->numPacketsSent = 0;
            cur->numHostUnreachSent++;
            if (cur->numHostUnreachSent == MAX_ARP_REQ) {
                packet_queue_counter_dequeue(cur);
                pthread_mutex_lock(&queue_mutex);
                curPacket = queue_head;
                while (curPacket) {
                    if (curPacket->dst_ip.s_addr == args->ip)
                        packet_queue_dequeue(curPacket);
                    curPacket = curPacket->next;
                }
                pthread_mutex_unlock(&queue_mutex);
            }
            pthread_join(*(args->thread), NULL);
        }
    }
    pthread_join(*(args->thread), NULL);
    return NULL;
}

void send_arp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                    uint8_t *packet, unsigned int len,
                    char *fromInterface) {
    struct sr_if *if_packet;
    if_packet = sr_get_interface(sr, fromInterface);
    mac_copy(ethernet_hdr->ether_dhost, ethernet_hdr->ether_shost);
    mac_copy(ethernet_hdr->ether_shost, if_packet->addr);
    sr_send_packet(sr, packet, len, fromInterface);
}

void send_arp_request(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                      uint8_t *packet, unsigned int len, char *toInterface,
                      struct in_addr next_hop) {
    struct sr_if *if_packet = sr_get_interface(sr, toInterface);
    uint32_t ip_vrhost = if_packet->ip;

    pthread_mutex_lock(&queue_mutex);
    packet_queue_enqueue(sr, packet, len, toInterface, next_hop);
    pthread_mutex_unlock(&queue_mutex);

    thread_add_arp_request(next_hop, ip_vrhost, if_packet->addr);
    mac_copy(ethernet_hdr->ether_shost, if_packet->addr);
    sr_send_packet(sr, packet, len, toInterface);

}

void send_ip_forward(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len, char *toInterface) {
    struct sr_if *if_packet = sr_get_interface(sr, toInterface);
    mac_copy(ethernet_hdr->ether_shost, if_packet->addr);
    sr_send_packet(sr, packet, len, toInterface);

}

void send_icmp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len, char *fromInterface) {
    unsigned char local_mac[ETHER_ADDR_LEN];
    struct sr_if *if_packet = sr_get_interface(sr, fromInterface);
    mac_copy(local_mac, if_packet->addr);
    uint32_t local_ip = if_packet->ip;
    struct ip *ip_hdr = (struct ip *) (packet + sizeof(struct sr_ethernet_hdr));
    ip_hdr->ip_ttl = 64;
    struct in_addr tmp = ip_hdr->ip_src;
    ip_hdr->ip_src = ip_hdr->ip_dst;
    ip_hdr->ip_dst = tmp;
    ip_hdr->ip_sum = htons(checksum_ip_get(ip_hdr));
    struct icmp *icmp_hdr = (struct icmp *) (packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
    icmp_hdr->type = ICMP_TYPE_ECHO_REPLY;
    icmp_hdr->code = ICMP_TYPE_ECHO_REPLY;
    icmp_hdr->checksum = htons(
            checksum_icmp_get(icmp_hdr, len - sizeof(struct sr_ethernet_hdr) - sizeof(struct ip)));
    struct sr_rt *rt_entry = rtable_get_routing_by_ip(sr, ip_hdr->ip_dst);
    if (rt_entry) {
        fromInterface = rt_entry->interface;
        if_packet = sr_get_interface(sr, fromInterface);
        mac_copy(local_mac, if_packet->addr);
    }
    int to_do_action = arp_miss_MAC(sr, ethernet_hdr->ether_dhost, ip_hdr->ip_dst);
    if (to_do_action == SEND_ARP_REQUEST) {
        pthread_mutex_lock(&queue_mutex);
        packet_queue_enqueue(sr, packet, len, fromInterface, ip_hdr->ip_dst);
        pthread_mutex_unlock(&queue_mutex);
        thread_add_arp_request(ip_hdr->ip_dst, local_ip, local_mac);
    }
    mac_copy(ethernet_hdr->ether_shost, local_mac);
    sr_send_packet(sr, packet, len, fromInterface);
}

void thread_add_arp_request(struct in_addr next_hop, uint32_t ip_vrhost, unsigned char mac_vrhost[ETHER_ADDR_LEN]) {
    struct pending_packet_counter *counter = pending_queue_get_node_by_IP(next_hop.s_addr);
    if (!counter) {
        counter = packet_queue_increment_wait_counter(next_hop, 0);
    } else if (counter->numPacketsSent == 0) {
        counter = packet_queue_increment_wait_counter(next_hop, 1);
    }
    if (counter->sentARPReq == 0) {
        gettimeofday(&timeval, NULL);
        counter->sentARPReq = 1;
        THREAD_COUNT++;
        thread = (pthread_t *) malloc(sizeof(pthread_t));
        p = (struct thread_counter *) malloc(sizeof(struct thread_counter));
        p->id = THREAD_COUNT;
        p->ip = next_hop.s_addr;
        p->type = SEND_ARP_REQUEST;
        p->thread = thread;
        mac_copy(p->mac_vrhost, mac_vrhost);
        p->ip_vrhost = ip_vrhost;
        pthread_create(&thread[0], NULL, thread_handler, (void *) (p));
    }

}

unsigned char *arp_get_MAC_by_IP(uint32_t ip) {
    unsigned char *mac = NULL;
    struct arp_node *node = arp_cache_head;
    while (node) {
        if (ntohl(node->arp_sip) == ip) {
            mac = malloc(ETHER_ADDR_LEN);
            mac_copy(mac, node->arp_sha);
        }
        node = node->next;
    }
    return mac;
}

int arp_miss_MAC(struct sr_instance *sr, uint8_t dhost[ETHER_ADDR_LEN], struct in_addr ip_dst) {
    int to_do_action;
    struct sr_rt *routing_entry = rtable_get_routing_by_ip(sr, ip_dst);
    unsigned char *found_mac = arp_get_MAC_by_IP(ip_dst.s_addr);
    unsigned char *found_mac_in_rtable = arp_get_MAC_by_IP(routing_entry->gw.s_addr);
    if (found_mac) {
        mac_copy(dhost, found_mac);
        to_do_action = FORWARD_IP;
    } else if (found_mac_in_rtable) {
        mac_copy(dhost, found_mac_in_rtable);
        to_do_action = FORWARD_IP;
    } else {
        to_do_action = SEND_ARP_REQUEST;
    }
    return to_do_action;
}


int handle_arp(struct sr_instance *sr, uint8_t *payload, char *interface) {
    int to_do_action = DROP;
    struct sr_arphdr *arp_hdr = (struct sr_arphdr *) payload;
    struct sr_if *if_packet = sr_get_interface(sr, interface);
    struct in_addr local_ip;
    local_ip.s_addr = if_packet->ip;
    unsigned char my_host[ETHER_ADDR_LEN];
    mac_copy(my_host, if_packet->addr);

    // If target is me
    int target_is_me = arp_hdr->ar_tip == local_ip.s_addr;
    if (target_is_me) {
        unsigned char *find_mac = arp_get_MAC_by_IP(htonl(arp_hdr->ar_sip));
        if (!find_mac) {
            struct arp_node *to_add = malloc(sizeof(struct arp_node));
            to_add->arp_type = arp_hdr->ar_pro;
            to_add->arp_sip = htonl(arp_hdr->ar_sip);
            mac_copy(to_add->arp_sha, arp_hdr->ar_sha);
            to_add->next = arp_cache_head;
            arp_cache_head = to_add;
        }
        if (ntohs(arp_hdr->ar_op) == ARP_REQUEST) {
            arp_hdr->ar_op = htons(ARP_REPLY);
            mac_swap(arp_hdr->ar_sha, arp_hdr->ar_tha);
            mac_copy(arp_hdr->ar_sha, my_host);
            uint32_t tmp = arp_hdr->ar_sip;
            arp_hdr->ar_sip = arp_hdr->ar_tip;
            arp_hdr->ar_tip = tmp;
            to_do_action = SEND_ARP_REPLY;
        } else if (ntohs(arp_hdr->ar_op) == ARP_REPLY) {
            struct pending_packet_counter *counter = pending_queue_get_node_by_IP(arp_hdr->ar_sip);
            if (counter != NULL) {
                counter->numPacketsSent = 0;
                counter->received = 1;
                packet_queue_counter_dequeue(counter);
                pthread_join(*(counter->thread_count.thread), NULL);
            }
            pthread_mutex_lock(&queue_mutex);
            struct packet_queue_node *node = queue_head;
            while (node) {
                struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) node->root;
                struct in_addr ip;
                ip.s_addr = arp_hdr->ar_sip;
                int flag = arp_miss_MAC(node->sr, ethernet_hdr->ether_dhost, ip);
                if (flag == FORWARD_IP) {
                    send_ip_forward(node->sr, ethernet_hdr, node->root, node->len, node->interface);
                    packet_queue_dequeue(node);
                }
                node = node->next;
            }
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    return to_do_action;
}

int handle_ip(struct sr_instance *sr,
              uint8_t dhost[ETHER_ADDR_LEN],
              uint8_t *payload,
              char **interface,
              struct in_addr *next_hop) {
    int to_do_action = DROP;
    struct ip *ip_hdr = (struct ip *) payload;
    struct icmp *icmp_hdr = (struct icmp *) (payload + sizeof(struct ip));
    struct sr_rt *rt_entry;

    if (!checksum_ip_correct(ip_hdr)) {
        return to_do_action;
    }
    struct sr_if *target_interface = NULL;
    struct sr_if *local_interface = sr->if_list;
    while (local_interface) {
        if (local_interface->ip == ip_hdr->ip_dst.s_addr) {
            target_interface = local_interface;
            break;
        } else {
            local_interface = local_interface->next;
        }
    }
    if (!target_interface) {
        // target is not me
        if (ip_hdr->ip_ttl <= 1) {
            return to_do_action;
        }
        ip_hdr->ip_ttl--;
        rt_entry = rtable_get_routing_by_ip(sr, ip_hdr->ip_dst);
        if (rt_entry) {
            // set the new interface
            *interface = rt_entry->interface;
            if (rt_entry->gw.s_addr) {
                next_hop->s_addr = rt_entry->gw.s_addr;
            } else {
                next_hop->s_addr = ip_hdr->ip_dst.s_addr;
            }
            // check arp cache
            to_do_action = arp_miss_MAC(sr, dhost, *next_hop);
        } else {
            return to_do_action;
        }
    } else {
        // target is me
        if (ip_hdr->ip_p == IPPROTO_ICMP && icmp_hdr->type == ICMP_TYPE_ECHO_REQ) {
            to_do_action = SEND_ICMP_REPLY;
        }
    }
    // update checksum
    ip_hdr->ip_sum = htons(checksum_ip_get(ip_hdr));
    return to_do_action;
}

int checksum_get(const char *header, unsigned int length) {
    u_long sum = 0;

    for (int i = 0; i < length; i += 2) {
        u_long tmp = 0;
        tmp += (u_char) header[i] << 8;
        tmp += (u_char) header[i + 1];
        sum += tmp;
    }
    u_short lWord = sum & 0x0000FFFF;
    u_short hWord = sum >> 16;
    u_short checksum = lWord + hWord;
    return ~checksum;
}

int checksum_ip_get(struct ip *ip_hdr) {
    ip_hdr->ip_sum = 0;
    return checksum_get((const char *) ip_hdr, (unsigned int) ip_hdr->ip_hl << 2);
}

int checksum_icmp_get(struct icmp *icmp_hdr, unsigned int len) {
    icmp_hdr->checksum = 0;
    return checksum_get((const char *) icmp_hdr, len);
}

int checksum_verify(const char *header, unsigned int length) {
    return ~(checksum_get(header, length) & 0xffff);
}

int checksum_ip_correct(struct ip *ip_hdr) {
    return checksum_verify((const char *) ip_hdr, (unsigned int) ip_hdr->ip_hl << 2);
}

void mac_copy(uint8_t *dest, const uint8_t *src) {
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        dest[i] = src[i];
    }
}

void mac_swap(uint8_t *a, uint8_t *b) {
    unsigned char buf[ETHER_ADDR_LEN];
    mac_copy(buf, a);
    mac_copy(a, b);
    mac_copy(b, buf);
}

void mac_empty(uint8_t *mac) {
    unsigned char buf[ETHER_ADDR_LEN] = {0};
    mac_copy(mac, buf);
}

void packet_queue_enqueue(struct sr_instance *sr, uint8_t *tmp, unsigned int len, char *interface, struct in_addr ip) {
    struct packet_queue_node *new = (struct packet_queue_node *) malloc(sizeof(struct packet_queue_node));
    new->sr = sr;
    new->len = len;
    new->interface = interface;
    new->dst_ip = ip;
    new->root = (uint8_t *) malloc(sizeof(uint8_t) * len);
    memcpy(new->root, tmp, sizeof(uint8_t) * len);
    new->prev = queue_tail;
    if (queue_tail) {
        queue_tail->next = new;
    } else {
        queue_head = new;
    }
    queue_tail = new;
    new->next = NULL;
}

void packet_queue_dequeue(struct packet_queue_node *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        queue_head = node->next;
        if (queue_head == NULL) {
            queue_tail = NULL;
        }
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    free(node);
}

struct sr_rt *rtable_get_routing_by_ip(struct sr_instance *sr, struct in_addr ip) {
    pwospf_lock(sr->ospf_subsys);
    struct sr_rt *routing_node = sr->routing_table;

    while (routing_node) {
        struct in_addr dest = routing_node->dest;
        if ((ip.s_addr & routing_node->mask.s_addr) == (dest.s_addr & routing_node->mask.s_addr)) {
            if (dest.s_addr != 0) {
                pwospf_unlock(sr->ospf_subsys);
                return routing_node;
            }
        }
        routing_node = routing_node->next;
    }
    routing_node = sr->routing_table;
    while (routing_node) {
        if (routing_node->dest.s_addr == 0) {
            pwospf_unlock(sr->ospf_subsys);
            return routing_node;
        }
        routing_node = routing_node->next;
    }
    pwospf_unlock(sr->ospf_subsys);
    return NULL;

}

struct pending_packet_counter *pending_queue_get_node_by_IP(uint32_t ip) {
    struct pending_packet_counter *node = pending_queue_head;
    while (node && node->ip != ip) {
        node = node->next;
    }
    return node;
}

void packet_queue_counter_dequeue(struct pending_packet_counter *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        pending_queue_head = node->next;
        if (!pending_queue_head)
            pending_queue_tail = NULL;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    free(node);
}

struct pending_packet_counter *packet_queue_increment_wait_counter(struct in_addr ip, int do_reset) {
    struct pending_packet_counter *point = pending_queue_head;
    struct pending_packet_counter *new;
    while (point && point->ip != ip.s_addr)
        point = point->next;
    if (!point) {
        new = (struct pending_packet_counter *) malloc(sizeof(struct pending_packet_counter));
        new->ip = ip.s_addr;
        new->received = 0;
        new->numPacketsSent = 1;
        new->sentARPReq = 0;
        new->numHostUnreachSent = 0;
        new->prev = pending_queue_tail;
        if (pending_queue_tail) {
            pending_queue_tail->next = new;
        } else {
            pending_queue_head = new;
        }
        pending_queue_tail = new;
        new->next = NULL;
        return pending_queue_head;
    } else {
        if (do_reset) {
            point->numPacketsSent = 1;
            point->received = 0;
            point->sentARPReq = 0;
        }
        return point;
    }
}

