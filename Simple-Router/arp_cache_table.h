//
// Created by Zhenyu Qi on 10/13/21.
//

#ifndef INC_2_ARP_CACHE_TABLE_H
#define INC_2_ARP_CACHE_TABLE_H

struct arp_node *arp_cache_head = NULL;

struct arp_node {
    uint32_t arp_ip;
    unsigned char arp_mac[ETHER_ADDR_LEN];
    struct arp_node *next;
} __attribute__ ((packed));

void arp_add_cache(uint32_t ip, unsigned char *mac);

int arp_update_cache(uint32_t ip, unsigned char *mac);

unsigned char *arp_get_mac_by_ip(uint32_t ip);

#endif //INC_2_ARP_CACHE_TABLE_H
