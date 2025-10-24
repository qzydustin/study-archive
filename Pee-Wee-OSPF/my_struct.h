#ifndef SR_OSPF_DSTRUCT_H
#define SR_OSPF_DSTRUCT_H

struct sr_ospf_neighbor
{
    char *interface;
    uint32_t rid;
	uint32_t ip;
	uint32_t subnet;
	int is_alive;
	int timeCounter;
	pthread_t *timer;
};

struct stack_node
{
	uint32_t rid;
	struct stack_node *next;
};

struct hello_timeout
{
	int nId;
	int timeout_t;
	struct sr_instance *sr;
};

struct arp_node {
    uint16_t arp_type;
    uint32_t arp_sip;
    unsigned char arp_sha[ETHER_ADDR_LEN];
    struct arp_node *next;
    struct arp_node *prev;
} __attribute__ ((packed));

struct icmp {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
} __attribute__ ((packed));

struct packet_queue_node
{
    struct sr_instance* sr;
    uint8_t *root;
    unsigned int len;
    char* interface;
    struct in_addr dst_ip;
    struct packet_queue_node *next;
    struct packet_queue_node *prev;
} __attribute__ ((packed));


#endif