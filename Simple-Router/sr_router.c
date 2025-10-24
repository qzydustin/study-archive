
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

// my file
#include "arp_cache_table.h"
#include "tool.h"
#include "routing_table.h"
#include "icmp.h"
#include "send.h"
#include "action.h"

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
 * and pass it along to sr_handle_ether_frame for further processing.
 *
 *---------------------------------------------------------------------*/

void sr_handlepacket(struct sr_instance *sr,
                     uint8_t *packet/* lent */,
                     unsigned int len,
                     char *interface/* lent */) {
    /* REQUIRES */
    assert(sr);
    assert(packet);
    assert(interface);

    printf("\n*** -> Received packet of length %d \n", len);

    /*  Begin traversing the packet list */
    struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) packet;
    uint8_t *payload = (uint8_t *) (packet + sizeof(struct sr_ethernet_hdr));
    uint16_t type = ntohs(ethernet_hdr->ether_type);


    // interface
    struct sr_if *if_packet = sr_get_interface(sr, interface);
    struct in_addr my_ip;
    my_ip.s_addr = if_packet->ip;
    unsigned char my_mac[ETHER_ADDR_LEN];
    copy_mac(my_mac, if_packet->addr);

    int action;
    if (type == ETHERTYPE_ARP) {
        struct sr_arphdr *arp_hdr = (struct sr_arphdr *) payload;
        int have_arp_cache = arp_update_cache(arp_hdr->ar_sip, arp_hdr->ar_sha);
        int is_dest_me = arp_hdr->ar_tip == my_ip.s_addr;
        if (is_dest_me) {
            if (!have_arp_cache) {
                uint32_t arp_sip = htonl(arp_hdr->ar_sip);
                unsigned char arp_sha[ETHER_ADDR_LEN];
                copy_mac(arp_sha, arp_hdr->ar_sha);
                arp_add_cache(arp_sip, arp_sha);
            }
            if (ntohs(arp_hdr->ar_op) == ARP_REQUEST) {
                // update type
                arp_hdr->ar_op = htons(ARP_REPLY);
                // update sha and tha
                copy_mac(arp_hdr->ar_tha, arp_hdr->ar_sha);
                copy_mac(arp_hdr->ar_sha, my_mac);
                // update sip and tip
                arp_hdr->ar_tip = arp_hdr->ar_sip;
                arp_hdr->ar_sip = my_ip.s_addr;
                // update ethernet s-mac and d-mac
                copy_mac(ethernet_hdr->ether_dhost, ethernet_hdr->ether_shost);
                copy_mac(ethernet_hdr->ether_shost, sr_get_interface(sr, interface)->addr);
                sr_send_packet(sr, packet, len, interface);
            }
        }
    } else if (type == ETHERTYPE_IP) {
        struct ip *ip_hdr = (struct ip *) payload;
        // 32 bit = 4 bytes, need to * 4 !!!
        int header_length = (int) ip_hdr->ip_hl * 4;

        // checksum
        if (!is_checksum_correct((const char *) ip_hdr, header_length)) {
            return;
        }

        unsigned char *find_mac = arp_get_mac_by_ip(ip_hdr->ip_dst.s_addr);
        if (find_mac) {
            action = ACTION_IP_FORWARD;
            copy_mac(ethernet_hdr->ether_dhost, find_mac);
        } else {
            struct sr_rt *routing_node = routing_search_prefix(sr, ip_hdr->ip_dst);
            if (routing_node->dest.s_addr == 0) {
                find_mac = arp_get_mac_by_ip(routing_node->gw.s_addr);
                if (find_mac) {
                    copy_mac(ethernet_hdr->ether_dhost, find_mac);
                    action = ACTION_IP_FORWARD;
                } else {
                    action = ACTION_ARP_GATEWAY;
                }
            } else {
                action = ACTION_ARP;
            }
        }

        // check address
        int is_address_me = 0;
        struct sr_if *interface_node = sr->if_list;
        while (interface_node) {
            if (interface_node->ip == ip_hdr->ip_dst.s_addr) {
                is_address_me = 1;
                break;
            } else {
                interface_node = interface_node->next;
            }
        }

        if (!is_address_me) {
            // not me, update ttl and forward
            if (ip_hdr->ip_ttl > 1) {
                ip_hdr->ip_ttl--;
            } else {
                // drop
                return;
            }
        } else {
            if (ip_hdr->ip_p == IPPROTO_ICMP) {
                struct icmp *icmp_hdr = (struct icmp *) (payload + sizeof(struct ip));
                if (icmp_hdr->type == 8)
                    action = ACTION_ICMP_REPLY;
            } else {
                // drop
                return;
            }
        }
        ip_hdr->ip_sum = 0;
        ip_hdr->ip_sum = htons(get_checksum((const char *) ip_hdr, header_length));
        if (action == ACTION_ICMP_REPLY) {
            send_icmp_reply(sr, ethernet_hdr, packet, len, interface);
        } else if (action == ACTION_IP_FORWARD) {
            send_ip_forward(sr, ethernet_hdr, packet, len, interface);
        } else {
            send_arp(sr, ethernet_hdr, packet, len, interface, action);
        }
    }

}

/* end sr_handlepacket */

void send_ip_forward(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len,
                     char *interface) {
    struct ip *ip_hdr = (struct ip *) (packet + sizeof(struct sr_ethernet_hdr));
    // find routing node
    struct sr_rt *rt_entry = routing_search_prefix(sr, ip_hdr->ip_dst);
    // update interface
    interface = &(rt_entry->interface);
    copy_mac(ethernet_hdr->ether_shost, sr_get_interface(sr, interface)->addr);
    sr_send_packet(sr, packet, len, interface);
}

void send_icmp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len,
                     char *interface) {
    struct ip *ip_hdr = (struct ip *) (packet + sizeof(struct sr_ethernet_hdr));
    ip_hdr->ip_ttl = 64;
    struct in_addr tmp = ip_hdr->ip_src;
    ip_hdr->ip_src = ip_hdr->ip_dst;
    ip_hdr->ip_dst = tmp;
    ip_hdr->ip_sum = 0;
    ip_hdr->ip_sum = htons(get_checksum((const char *) ip_hdr, ip_hdr->ip_hl * 4));

    struct icmp *icmp_hdr = (struct icmp *) (packet + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
    icmp_hdr->type = 0;
    icmp_hdr->code = 0;
    icmp_hdr->checksum = 0;
    icmp_hdr->checksum = htons(get_checksum((const char *) icmp_hdr, len -
                                                                     sizeof(struct sr_ethernet_hdr) -
                                                                     sizeof(struct ip)));

    struct sr_rt *routing_node = routing_search_prefix(sr, ip_hdr->ip_dst);
    interface = &(routing_node->interface);
    struct sr_if *if_packet = sr_get_interface(sr, interface);

    if (routing_node->dest.s_addr == 0) {
        unsigned char *find_mac = arp_get_mac_by_ip(routing_node->gw.s_addr);
        if (find_mac) {
            copy_mac(ethernet_hdr->ether_dhost, find_mac);
        } else {
            send_arp(sr, ethernet_hdr, packet, len, interface, ACTION_ARP_GATEWAY);
        }
    }
    copy_mac(ethernet_hdr->ether_shost, if_packet->addr);
    sr_send_packet(sr, packet, len, interface);
}

void send_arp(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
              uint8_t *packet, unsigned int len,
              char *interface, int action) {
    struct ip *ip_hdr = (struct ip *) (packet + sizeof(struct sr_ethernet_hdr));
    struct sr_rt *routing_node = routing_search_prefix(sr, ip_hdr->ip_dst);
    interface = &(routing_node->interface);

    struct sr_if *if_packet = sr_get_interface(sr, interface);
    unsigned char my_mac[ETHER_ADDR_LEN];

    copy_mac(my_mac, if_packet->addr);
    uint32_t my_ip = if_packet->ip;

    uint8_t broadcast_mac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    struct sr_arphdr *arp_hdr;
    unsigned char tmp[ETHER_ADDR_LEN] = {0};
    packet = malloc(sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr));
    ethernet_hdr = (struct sr_ethernet_hdr *) packet;
    arp_hdr = (struct sr_arphdr *) (packet + sizeof(struct sr_ethernet_hdr));
    arp_hdr->ar_hrd = htons(ARPHDR_ETHER);
    arp_hdr->ar_pro = htons(ETHERTYPE_IP);
    arp_hdr->ar_hln = ETHER_ADDR_LEN;
    arp_hdr->ar_pln = 4;
    arp_hdr->ar_op = htons(ARP_REQUEST);
    copy_mac(arp_hdr->ar_sha, my_mac);
    arp_hdr->ar_sip = my_ip;
    copy_mac(arp_hdr->ar_tha, tmp);
    if (action == ACTION_ARP) {
        arp_hdr->ar_tip = ip_hdr->ip_dst.s_addr;
    } else if (action == ACTION_ARP_GATEWAY) {
        arp_hdr->ar_tip = routing_node->gw.s_addr;
    }
    copy_mac(ethernet_hdr->ether_dhost, broadcast_mac);
    copy_mac(ethernet_hdr->ether_shost, my_mac);
    ethernet_hdr->ether_type = htons(ETHERTYPE_ARP);

    len = sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr);
    copy_mac(ethernet_hdr->ether_shost, my_mac);
    sr_send_packet(sr, packet, len, interface);
}

void arp_add_cache(uint32_t ip, unsigned char *mac) {
    struct arp_node *to_add = (struct arp_node *) malloc(sizeof(struct arp_node));
    to_add->arp_ip = ip;
    copy_mac(to_add->arp_mac, mac);
    to_add->next = arp_cache_head;
    arp_cache_head = to_add;
}

int arp_update_cache(uint32_t ip, unsigned char *mac) {
    int have_arp_cache = 0;
    struct arp_node *node = arp_cache_head;
    while (node != NULL) {
        if (node->arp_ip == ip) {
            // exist arp, update mac
            copy_mac(node->arp_mac, mac);
            return ++have_arp_cache;
        }
        node = node->next;
    }
    return have_arp_cache;
}

unsigned char *arp_get_mac_by_ip(uint32_t ip) {
    unsigned char *mac = NULL;
    struct arp_node *node = arp_cache_head;
    while (node != NULL) {
        if (ntohl(node->arp_ip) == ip) {
            mac = malloc(ETHER_ADDR_LEN);
            copy_mac(mac, node->arp_mac);
        }
        node = node->next;
    }
    return mac;
}


struct sr_rt *routing_search_prefix(struct sr_instance *sr, struct in_addr ip) {
    struct sr_rt *routing_node = sr->routing_table;
    struct sr_rt *longest = NULL;
    uint32_t network_id;
    uint32_t routing_network_id;


    while (routing_node) {
        if (!longest) {
            longest = routing_node;
            continue;
        }
        // count the length of Network ID
        int numBits = 0;
        uint32_t mask = ntohl(routing_node->mask.s_addr);
        mask = ~mask;
        while (mask) {
            mask = mask >> 1;
            numBits++;
        }
        routing_network_id = ntohl(routing_node->dest.s_addr) >> numBits;
        network_id = ntohl(ip.s_addr) >> numBits;
        // same network ID, match!
        if (routing_network_id == network_id) {
            longest = routing_node;
        }
        routing_node = routing_node->next;
    }
    return longest;
}

int get_checksum(const char *header, int length) {
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
    checksum = ~checksum;
    return checksum;
}

int is_checksum_correct(const char *header, int length) {
    return ~(get_checksum(header, length) & 0xffff);
}

void copy_mac(uint8_t *dest, const uint8_t *src) {
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        dest[i] = src[i];
    }
}

