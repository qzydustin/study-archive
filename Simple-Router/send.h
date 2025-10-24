//
// Created by Zhenyu Qi on 10/16/21.
//

#ifndef PROJ1_SEND_H
#define PROJ1_SEND_H

void send_ip_forward(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len,
                     char *interface);

void send_icmp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len,
                     char *interface);

void send_arp(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
              uint8_t *packet, unsigned int len,
              char *interface, int action);

#endif //PROJ1_SEND_H
