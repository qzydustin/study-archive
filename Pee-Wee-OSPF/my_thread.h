//
// Created by Zhenyu Qi on 11/28/21.
//

#ifndef PEEWEEOSPF_MY_THREAD_H
#define PEEWEEOSPF_MY_THREAD_H

#define MAX_ARP_REQ 5
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global thread counter */
int THREAD_COUNT = 0;
struct thread_counter *p;
pthread_t *thread;
struct timeval timeval;

struct thread_counter
{
    int id;
    uint32_t ip;
    unsigned char mac_vrhost[ETHER_ADDR_LEN];
    uint32_t ip_vrhost;
    pthread_t *thread;
    int type;
};
struct pending_packet_counter
{
    uint32_t ip;
    int      numPacketsSent;
    int received;
    int sentARPReq;
    int numHostUnreachSent;
    struct thread_counter thread_count;
    struct pending_packet_counter *next;
    struct pending_packet_counter *prev;
};

struct pending_packet_counter* pending_queue_get_node_by_IP(uint32_t ip);

void packet_queue_counter_dequeue(struct pending_packet_counter *node) ;

struct pending_packet_counter *packet_queue_increment_wait_counter(struct in_addr ip, int do_reset);

void *thread_handler(void *arg);

void thread_add_arp_request(struct in_addr next_hop, uint32_t ip_vrhost, unsigned char mac_vrhost[ETHER_ADDR_LEN]);

#endif //PEEWEEOSPF_MY_THREAD_H
