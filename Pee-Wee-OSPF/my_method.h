#ifndef SR_FWD_REF_H
#define SR_FWD_REF_H


/* SR methods */
struct sr_rt *rtable_get_routing_by_ip(struct sr_instance *sr, struct in_addr ip);

uint8_t *make_a_new_arp_packet(unsigned char *local_mac, uint32_t sip, uint32_t tip);

int checksum_get(const char *header, unsigned int length);

int checksum_icmp_get(struct icmp *icmp_hdr, unsigned int len);

int checksum_verify(const char *header, unsigned int length);

int checksum_ip_correct(struct ip *ip_hdr);

int handle_arp(struct sr_instance *sr, uint8_t *payload, char *interface);

int handle_ip(struct sr_instance *sr, uint8_t dhost[ETHER_ADDR_LEN], uint8_t *payload,
              char **interface, struct in_addr *next_hop);

void send_arp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                    uint8_t *packet, unsigned int len, char *fromInterface);

void send_arp_request(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                      uint8_t *packet, unsigned int len, char *toInterface,
                      struct in_addr next_hop);

void send_ip_forward(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len, char *toInterface);

void send_icmp_reply(struct sr_instance *sr, struct sr_ethernet_hdr *ethernet_hdr,
                     uint8_t *packet, unsigned int len, char *fromInterface);

unsigned char *arp_get_MAC_by_IP(uint32_t ip);

int arp_miss_MAC(struct sr_instance *sr, uint8_t dhost[ETHER_ADDR_LEN], struct in_addr ip_dst);

void packet_queue_enqueue(struct sr_instance *sr, uint8_t *tmp, unsigned int len, char *interface, struct in_addr ip);

void packet_queue_dequeue(struct packet_queue_node *node);

/*PWOSPF methods*/
int get_interface_name_number(char *);

void handle_pwospf(struct sr_instance *, uint8_t *, unsigned int, char *);

int checksum_ospf_get(struct ospfv2_hdr *ospf_hdr);

int checksum_ip_get(struct ip *ip_hdr);

void arp_fill(struct sr_instance *sr);

void send_hello_packet(struct sr_instance *sr, uint8_t *packet, char *interface);

void send_lsu(void *);

void send_lsu_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface);

_Noreturn void *hello_handler(void *arg);

void *timeout(void *arg);

_Noreturn void *lsu_handler(void *arg);

void mac_copy(uint8_t *dest, const uint8_t *src);

void mac_swap(uint8_t *a, uint8_t *b);

void mac_empty(uint8_t *mac);

#endif /* SR_FWD_REF_H  */
