/*-----------------------------------------------------------------------------
 * file: sr_pwospf.c
 * date: Tue Nov 23 23:24:18 PST 2004
 * Author: Martin Casado
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#include "sr_pwospf.h"
#include "sr_protocol.h"
#include "sr_router.h"
#include "sr_if.h"
#include "sr_rt.h"
#include "pwospf_protocol.h"

/* Custom headers */
#include "my_method.h"
#include "my_struct.h"
#include "my_graph.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>

#define DROP                 0
#define ROUTER                0
#define SUBNET               1
#define INF                 5
pthread_t *hello_thread;
pthread_t *lsu_thread;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t all_lock = PTHREAD_MUTEX_INITIALIZER;


long int global_rid, aid = 0;
int interface_number = 0;
int lsu_seq = 1, vid = 0;
struct sr_ospf_neighbor *neighbors = NULL;
struct graph *subnet_graph = NULL;
struct stack_node *stack_head = NULL;

/* -- declaration of main thread function for pwospf subsystem --- */
_Noreturn static void *pwospf_run_thread(void *arg);

/*---------------------------------------------------------------------
 * Method: pwospf_lock
 *
 * Lock lock associated with pwospf_subsys
 *
 *---------------------------------------------------------------------*/

void pwospf_lock(struct pwospf_subsys *subsys) {
    if (pthread_mutex_lock(&subsys->lock)) { assert(0); }
} /* -- pwospf_subsys -- */

/*---------------------------------------------------------------------
 * Method: pwospf_unlock
 *
 * Unlock lock associated with pwospf subsystem
 *
 *---------------------------------------------------------------------*/

void pwospf_unlock(struct pwospf_subsys *subsys) {
    if (pthread_mutex_unlock(&subsys->lock)) { assert(0); }
} /* -- pwospf_subsys -- */

/*---------------------------------------------------------------------
 * Method: pwospf_init(..)
 *
 * Sets up the internal data structures for the pwospf subsystem
 *
 * You may assume that the interfaces have been created and initialized
 * by this point.
 *---------------------------------------------------------------------*/

int pwospf_init(struct sr_instance *sr) {
    struct sr_if *walker = sr->if_list;
    while (walker) {
        if (interface_number == 0)
            global_rid = walker->ip;
        interface_number++;
        walker = walker->next;
    }
    sr->ospf_subsys = (struct pwospf_subsys *) malloc(sizeof(struct pwospf_subsys));
    pthread_mutex_init(&(sr->ospf_subsys->lock), 0);

    hello_thread = (pthread_t *) malloc(sizeof(pthread_t));
    lsu_thread = (pthread_t *) malloc(sizeof(pthread_t));
    neighbors = (struct sr_ospf_neighbor *) malloc(interface_number * sizeof(struct sr_ospf_neighbor));

    walker = sr->if_list;
    // init neighbor
    for(int j =0;; j++){
        char iface[10] = "eth", buf[2];
        neighbors[j].rid = 0;
        neighbors[j].ip = 0;
        neighbors[j].subnet = walker->ip & walker->mask;
        sprintf(buf, "%d", j);
        strcat(iface, buf);
        neighbors[j].interface = (char *) malloc(strlen(iface) + 1);
        strcpy(neighbors[j].interface, iface);
        neighbors[j].is_alive = 1;
        walker = walker->next;
        if(!walker){
            break;
        }
    }

    arp_fill(sr);
    subnet_graph = (struct graph *) malloc(sizeof(struct graph));
    subnet_graph->V = 0;
    subnet_graph->vList = NULL;
    add_vertex(global_rid, 0, 0, ROUTER);

    //add Iface to vertices
    walker = sr->if_list;
    struct vertexList *r = get_node_by_rid(global_rid);

    struct in_addr dest_addr;
    struct in_addr gw_addr;
    struct in_addr mask_addr;

    while (walker) {
        add_vertex(global_rid, walker->ip & walker->mask, walker->mask, SUBNET);
        add_edge(r, subnet_graph->vList, 1);

        dest_addr.s_addr = walker->ip & walker->mask;
        gw_addr.s_addr = 0;
        mask_addr.s_addr = walker->mask;

        sr_add_rt_entry(sr, dest_addr, gw_addr,
                        mask_addr, walker->name);
        walker = walker->next;
    }

    pthread_create(&sr->ospf_subsys->thread, 0, pwospf_run_thread, sr);
    pthread_create(&hello_thread[0], NULL, hello_handler, sr);
    pthread_create(&lsu_thread[0], NULL, lsu_handler, sr);

    return 0; /* success */
} /* -- pwospf_init -- */

/*---------------------------------------------------------------------
 * Method: pwospf_run_thread
 *
 * Main thread of pwospf subsystem.
 *
 *---------------------------------------------------------------------*/

_Noreturn static
void* pwospf_run_thread(void* arg)
{
    struct sr_instance* sr = (struct sr_instance*)arg;

    while(1) {
        /* -- PWOSPF subsystem functionality should start  here! -- */
        printf(" pwospf subsystem awake \n");
        pwospf_lock(sr->ospf_subsys);
        pthread_mutex_lock(&lock);
        pthread_mutex_unlock(&lock);
        pwospf_unlock(sr->ospf_subsys);
        printf(" pwospf subsystem sleeping \n");
        sleep(2);
    }
} /* -- run_ospf_thread -- */

//*************************


void handle_pwospf(struct sr_instance *sr, uint8_t *packet,
                   unsigned int len,
                   char *interface) {
    uint8_t *payload = (uint8_t *) (packet + sizeof(struct sr_ethernet_hdr));
    struct ospfv2_hdr *ospf_hdr = (struct ospfv2_hdr *) (payload + sizeof(struct ip));
    struct ip *ip_hdr = (struct ip *) payload;

    if (ospf_hdr->version != OSPF_V2) {
        return;
    }

    if (!checksum_verify((const char *) ospf_hdr, ntohs(ospf_hdr->len))) {
        return;
    }

    if (ntohl(ospf_hdr->aid) != aid) {
        return;
    }

    if (ospf_hdr->autype != 0) {
        return;
    }

    if (ntohl(ip_hdr->ip_dst.s_addr) != OSPF_AllSPFRouters) {
        return;
    }

    if (ospf_hdr->type == OSPF_TYPE_HELLO) {
        send_hello_packet(sr, packet, interface);
    } else if (ospf_hdr->type == OSPF_TYPE_LSU) {
        send_lsu_packet(sr, packet, len, interface);
    }


}

void send_hello_packet(struct sr_instance *sr, uint8_t *packet, char *interface) {
    uint8_t *payload = (uint8_t *) (packet + sizeof(struct sr_ethernet_hdr));
    struct ip *ip_hdr = (struct ip *) payload;
    struct ospfv2_hdr *ospf_hdr = (struct ospfv2_hdr *) (payload + sizeof(struct ip));
    struct ospfv2_hello_hdr *hello_packet = (struct ospfv2_hello_hdr *)
            (payload + sizeof(struct ip) + sizeof(struct ospfv2_hdr));
    int index = get_interface_name_number(interface);
    struct hello_timeout *p = (struct hello_timeout *) malloc(sizeof(struct hello_timeout));
    struct sr_if *walker = sr_get_interface(sr, interface);

    if ((hello_packet->nmask != walker->mask) || (ntohs(hello_packet->helloint) != OSPF_DEFAULT_HELLOINT)) {
        return;
    }

    if (neighbors[index].rid == 0) {
        unsigned char local_mac[ETHER_ADDR_LEN];
        mac_copy(local_mac, walker->addr);
        pthread_mutex_lock(&all_lock);
        neighbors[index].rid = ospf_hdr->rid;
        neighbors[index].ip = ip_hdr->ip_src.s_addr;
        neighbors[index].is_alive = 1;
        neighbors[index].timeCounter = 0;
        neighbors[index].timer = (pthread_t *) malloc(sizeof(pthread_t));
        pthread_mutex_unlock(&all_lock);

        p->nId = index;
        p->timeout_t = OSPF_NEIGHBOR_TIMEOUT;
        p->sr = sr;
        pthread_create(&neighbors[index].timer[0], NULL, timeout, (void *) (p));
        uint8_t *new_package = make_a_new_arp_packet(local_mac, walker->ip, neighbors[index].ip);
        sr_send_packet(sr, new_package, sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr), walker->name);
    } else {
        pthread_mutex_lock(&all_lock);
        neighbors[index].timeCounter = 0;
        pthread_mutex_unlock(&all_lock);
        if (!neighbors[index].is_alive) {
            pthread_mutex_lock(&all_lock);
            neighbors[index].is_alive = 1;
            neighbors[index].timer = (pthread_t *) malloc(sizeof(pthread_t));
            pthread_mutex_unlock(&all_lock);

            p->nId = index;
            p->timeout_t = OSPF_NEIGHBOR_TIMEOUT;
            p->sr = sr;
            pthread_create(&neighbors[index].timer[0], NULL, timeout, (void *) (p));
            send_lsu((void *) sr);
        }
    }
}

void send_lsu_packet(struct sr_instance *sr, uint8_t *packet, unsigned int len, char *interface) {
    uint8_t *payload = (uint8_t *) (packet + sizeof(struct sr_ethernet_hdr));
    struct ospfv2_hdr *ospf_hdr = (struct ospfv2_hdr *) (payload + sizeof(struct ip));

    struct ospfv2_lsu_hdr *lsu_hdr = (struct ospfv2_lsu_hdr *)
            (payload + sizeof(struct ip) + sizeof(struct ospfv2_hdr));

    struct ospfv2_lsu *lsa = (struct ospfv2_lsu *) malloc(ntohl(lsu_hdr->num_adv) * sizeof(struct ospfv2_lsu));

    long int advertisement = ntohl(lsu_hdr->num_adv);
    memcpy(lsa, payload + sizeof(struct ip) + sizeof(struct ospfv2_hdr) + sizeof(struct ospfv2_lsu_hdr),
           advertisement * sizeof(struct ospfv2_lsu));

    long int numAdv = ntohl(lsu_hdr->num_adv);
    memcpy(lsa, payload + sizeof(struct ip) + sizeof(struct ospfv2_hdr) + sizeof(struct ospfv2_lsu_hdr),
           numAdv * sizeof(struct ospfv2_lsu));

    struct vertexList *router;
    if (ospf_hdr->rid == global_rid) {
        return;
    } else {
        router = get_node_by_rid(ospf_hdr->rid);
        if (router) {
            if (router->v.latestSeqNum > ntohs(lsu_hdr->seq)) {
                return;
            } else {
                router->v.latestSeqNum = ntohs(lsu_hdr->seq);
            }
        }
    }


    if (get_node_by_rid(ospf_hdr->rid) == NULL) {
        add_vertex(ospf_hdr->rid, 0, 0, ROUTER);
        router = get_node_by_rid(ospf_hdr->rid);
        router->v.latestSeqNum = ntohs(lsu_hdr->seq);
    }
    for (int i = 0; i < advertisement; ++i) {
        struct vertexList *from, *to, *temp;

        to = get_node_by_subnet(lsa[i].subnet);

        if (to == NULL) {
            add_vertex(ospf_hdr->rid, lsa[i].subnet, lsa[i].mask, SUBNET);
        }
        from = get_node_by_rid(ospf_hdr->rid);
        to = get_node_by_subnet(lsa[i].subnet);

        if (!is_edge_present(from->v.id, to->v.id)) { add_edge(from, to, 1); }

        if (lsa[i].rid != 0) {
            if (get_node_by_rid(lsa[i].rid) == NULL) {
                add_vertex(lsa[i].rid, 0, 0, ROUTER);
            }
        }

        from = get_node_by_rid(ospf_hdr->rid);
        to = get_node_by_rid(lsa[i].rid);

        if (from && to) {
            if (!is_edge_present(from->v.id, to->v.id)) { add_edge(from, to, 1); }
            if (!is_edge_present(to->v.id, from->v.id)) {
                add_edge(to, from, 1);
            }
        }

            // OSPF RID that this router is connected to is 0. Check what to invalidate
        else {
            struct vertexList *new_router = get_node_by_rid(ospf_hdr->rid);
            struct adjList *e = new_router->v.head;
            if (lsa[i].subnet != 0) {
                while (e) {
                    temp = get_vertex(e->e.to);
                    if (temp && temp->v.type == ROUTER) {
                        // stack_push_node
                        struct stack_node *to_add = (struct stack_node *) malloc(sizeof(struct stack_node));
                        to_add->rid = temp->v.rid;
                        to_add->next = stack_head;
                        stack_head = to_add;
                    }
                    e = e->next;
                }
                int cannot_find = 0;
                while (stack_head) {
                    // stack_pop_node
                    uint32_t stack_node = stack_head->rid;
                    stack_head = stack_head->next;
                    router = get_node_by_rid(stack_node);
                    for (int j = 0; j < advertisement; j++) {
                        if (lsa[j].rid == stack_node) {
                            cannot_find = 1;
                        }
                    }
                    if (!cannot_find) {
                        delete_node(new_router, get_edge(new_router, router->v.id));
                        delete_node(router, get_edge(router, new_router->v.id));
                    } else {
                        cannot_find = 0;
                    }
                }
            }
        }

    }
    lsu_hdr->ttl--;
    struct sr_if *walker = sr->if_list;
    while (walker) {
        struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) packet;
        mac_copy(ethernet_hdr->ether_shost, walker->addr);
        ospf_hdr->csum = htons(checksum_ospf_get(ospf_hdr));
        if (strcmp(walker->name, interface) != 0) {
            sr_send_packet(sr, packet, len, walker->name);
        }
        walker = walker->next;
    }
    struct vertexList *s = get_node_by_rid(global_rid);
    dijkstra_algorithm(s);

    pwospf_lock(sr->ospf_subsys);

    struct vertexList *vl_walker = subnet_graph->vList;
    while (vl_walker) {
        if (vl_walker->v.type == SUBNET) {
            struct vertexList *list = get_vertex(vl_walker->v.parent);

            while (list && list->v.parent != 0){
                list = get_vertex(list->v.parent);
            }

            if (list) {
                int neighbor_id = 0;
                for (int i = 0; i < interface_number; i++) {
                    if (neighbors[i].rid == list->v.rid) {
                        neighbor_id = i;
                        break;
                    }
                }
                struct in_addr dest_addr;
                struct in_addr gw_addr;
                struct in_addr mask_addr;
                dest_addr.s_addr = vl_walker->v.subnet;
                gw_addr.s_addr = neighbors[neighbor_id].ip;
                mask_addr.s_addr = vl_walker->v.nmask;

                struct sr_rt *rtable  = sr->routing_table;
                int flag = 1;
                while (rtable) {
                    if (rtable->dest.s_addr == vl_walker->v.subnet){
                        flag = 0;
                        break;
                    }
                    rtable = rtable->next;
                }
                if (flag) {
                    sr_add_rt_entry(sr, dest_addr, gw_addr, mask_addr, neighbors[neighbor_id].interface);
                } else {
                    rtable = sr->routing_table;
                    while (rtable) {
                        if (rtable->dest.s_addr == vl_walker->v.subnet) {
                            rtable->gw.s_addr = neighbors[neighbor_id].ip;
                            memcpy(rtable->interface, neighbors[neighbor_id].interface, strlen(rtable->interface));
                        }
                        rtable = rtable->next;
                    }
                }
            }
        }

        vl_walker = vl_walker->next;
    }

    pwospf_unlock(sr->ospf_subsys);
}


void dijkstra_algorithm(struct vertexList *list) {

    // init_single_source
    struct vertexList *v = subnet_graph->vList;
    while (v) {
        v->v.d = INF;
        v->v.parent = -1;
        v->v.is_visited = 0;
        v = v->next;
    }
    list->v.d = 0;

    struct vertexList *graph_node = subnet_graph->vList;

    while (graph_node) {
        // get least
        struct vertexList *tmp_node = subnet_graph->vList;
        struct vertexList *least_node;
        int least = INF + 1;
        while (tmp_node) {
            if (!tmp_node->v.is_visited) {
                if (tmp_node->v.d < least) {
                    least = tmp_node->v.d;
                    least_node = tmp_node;
                }
            }
            tmp_node = tmp_node->next;
        }

        if (least_node) {
            least_node->v.is_visited = 1;
            // insert
            struct path *to_add = (struct path *) malloc(sizeof(struct path));
            memcpy(&to_add->v, &least_node->v, sizeof(struct vertex));
            to_add->next = NULL;

            struct adjList *e = least_node->v.head;
            while (e) {
                v = get_vertex(e->e.to);
                // relax
                if (v->v.d > least_node->v.d + e->e.weight) {
                    v->v.d = least_node->v.d + e->e.weight;
                    v->v.parent = least_node->v.id;
                }
                e = e->next;
            }
        }
        graph_node = graph_node->next;
    }
}

void add_vertex(uint32_t rid, uint32_t subnet, uint32_t nmask, int type) {
    struct vertexList *vl;
    vl = (struct vertexList *) malloc(sizeof(struct vertexList));
    subnet_graph->V++;
    vl->v.id = vid++;
    vl->v.rid = rid;
    vl->v.subnet = subnet;
    vl->v.nmask = nmask;
    vl->v.latestSeqNum = 0;
    vl->v.type = type;
    vl->v.is_visited = 0;
    vl->v.head = NULL;
    vl->next = subnet_graph->vList;
    subnet_graph->vList = vl;
}

void add_edge(struct vertexList *v, struct vertexList *w, int weight) {
    struct adjList *edge = (struct adjList *) malloc(sizeof(struct adjList));
    edge->e.from = v->v.id;
    edge->e.to = w->v.id;
    edge->e.weight = weight;
    edge->next = v->v.head;
    edge->prev = NULL;
    if (v->v.head != NULL) {
        v->v.head->prev = edge;
    }
    v->v.head = edge;
}

void delete_node(struct vertexList *list, struct adjList *node) {
    if (node->prev != NULL) {
        // is not head
        node->prev->next = node->next;
    } else {
        // is head
        list->v.head = node->next;
    }
    if (node->next != NULL) {
        // is not tail
        node->next->prev = node->prev;
    }
    free(node);
}

struct adjList *get_edge(struct vertexList *list, unsigned int to) {
    struct adjList *vlWalker = list->v.head;
    while (vlWalker) {
        if (vlWalker->e.to == to) {
            return vlWalker;
        }
        vlWalker = vlWalker->next;
    }

    return NULL;
}

struct vertexList *get_vertex(unsigned int id) {
    struct vertexList *vlWalker = subnet_graph->vList;
    while (vlWalker) {
        if (vlWalker->v.id == id)
            return vlWalker;
        vlWalker = vlWalker->next;
    }
    return NULL;
}

int is_edge_present(uint32_t from, uint32_t to) {
    struct vertexList *vlWalker = subnet_graph->vList;
    while (vlWalker) {
        struct adjList *edge = vlWalker->v.head;
        while (edge) {
            if (edge->e.from == from && edge->e.to == to) {
                return 1;
            }
            edge = edge->next;
        }
        vlWalker = vlWalker->next;
    }
    return 0;
}

struct vertexList *get_node_by_rid(uint32_t rid) {
    struct vertexList *vlWalker = subnet_graph->vList;
    while (vlWalker) {
        if (vlWalker->v.type == ROUTER && vlWalker->v.rid == rid)
            return vlWalker;
        vlWalker = vlWalker->next;
    }

    return NULL;
}

struct vertexList *get_node_by_subnet(uint32_t subnet) {
    struct vertexList *vlWalker = subnet_graph->vList;
    while (vlWalker) {
        if (vlWalker->v.type == SUBNET && vlWalker->v.subnet == subnet)
            return vlWalker;
        vlWalker = vlWalker->next;
    }

    return NULL;
}

int checksum_ospf_get(struct ospfv2_hdr *ospf_hdr) {
    ospf_hdr->csum = 0;
    return checksum_get((const char *) ospf_hdr, ntohs(ospf_hdr->len));
}

void arp_fill(struct sr_instance *sr) {
    struct sr_if *walker = sr->if_list;
    while (walker) {
        uint32_t sip = ntohl(walker->ip);
        if (get_interface_name_number(walker->name) == 0) {
            sip -= 1;
        } else if (get_interface_name_number(walker->name) == 1) {
            sip += 1;
        }
        uint8_t *packet = (uint8_t *) make_a_new_arp_packet(walker->addr, walker->ip, htonl(sip));
        int len = sizeof(struct sr_ethernet_hdr) + sizeof(struct sr_arphdr);
        sr_send_packet(sr, packet, len, walker->name);
        walker = walker->next;
    }
}


_Noreturn void *hello_handler(void *arg) {
    struct sr_instance *sr = (struct sr_instance *) arg;

    int len = sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct ospfv2_hdr) + sizeof(struct ospfv2_hello_hdr);

    while (1) {
        pthread_mutex_lock(&lock);

        struct sr_if *walker = sr->if_list;
        while (walker) {
            uint32_t ip_src = walker->ip;
            uint8_t *newPacket = (uint8_t *) malloc(len);
            // ethernet head
            struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) newPacket;
            ethernet_hdr->ether_type = htons(ETHERTYPE_IP);
            uint8_t broadcast_mac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
            mac_copy(ethernet_hdr->ether_dhost, broadcast_mac);
            mac_copy(ethernet_hdr->ether_shost,walker->addr);

            // ip head
            struct ip *ip_hdr = (struct ip *) (newPacket + sizeof(struct sr_ethernet_hdr));
            ip_hdr->ip_hl = sizeof(struct ip) / 4;
            ip_hdr->ip_v = 4;
            ip_hdr->ip_tos = 0;
            ip_hdr->ip_len = htons(len - sizeof(struct sr_ethernet_hdr));
            ip_hdr->ip_id = htons(0x2607);
            ip_hdr->ip_off = htons(IP_DF);
            ip_hdr->ip_ttl = 64;
            ip_hdr->ip_p = IPPROTO_OSPF;
            ip_hdr->ip_src.s_addr = ip_src;
            ip_hdr->ip_dst.s_addr = htonl(OSPF_AllSPFRouters);
            ip_hdr->ip_sum = htons(checksum_ip_get(ip_hdr));

            // ospf head
            struct ospfv2_hdr *ospf_hdr = (struct ospfv2_hdr *) (newPacket + sizeof(struct sr_ethernet_hdr) + sizeof(struct ip));
            ospf_hdr->version = OSPF_V2;
            ospf_hdr->type = OSPF_TYPE_HELLO;
            ospf_hdr->len = htons(len - sizeof(struct sr_ethernet_hdr) - sizeof(struct ip));
            ospf_hdr->rid = global_rid;
            ospf_hdr->aid = htonl(aid);
            ospf_hdr->autype = 0;
            ospf_hdr->audata = 0;
            struct ospfv2_hello_hdr *hello_packet = (struct ospfv2_hello_hdr *) (newPacket + sizeof(struct sr_ethernet_hdr)
                                                        + sizeof(struct ip) + sizeof(struct ospfv2_hdr));

            hello_packet->nmask = walker->mask;
            hello_packet->helloint = htons(OSPF_DEFAULT_HELLOINT);
            hello_packet->padding = 0;

            ospf_hdr->csum = htons(checksum_ospf_get(ospf_hdr));
            sr_send_packet(sr, newPacket, len, walker->name);
            walker = walker->next;
        }
        pthread_mutex_unlock(&lock);
        sleep(OSPF_DEFAULT_HELLOINT);
    }
}

void *timeout(void *arg) {
    struct hello_timeout *p = (struct hello_timeout *) arg;

    while (neighbors[p->nId].timeCounter < p->timeout_t) {
        sleep(1);
        pthread_mutex_lock(&all_lock);
        neighbors[p->nId].timeCounter++;
        pthread_mutex_unlock(&all_lock);
    }
    pthread_mutex_lock(&all_lock);
    neighbors[p->nId].is_alive = 0;
    pthread_mutex_unlock(&all_lock);
    send_lsu((void *) p->sr);
    pthread_exit(0);
}

_Noreturn void *lsu_handler(void *arg) {
    sleep(10);
    while (1) {
        send_lsu(arg);
        sleep(OSPF_DEFAULT_LSUINT);
    }
}

void send_lsu(void *arg) {
    struct sr_instance *sr = (struct sr_instance *) arg;
    pthread_mutex_lock(&lock);
    struct sr_if *walker = sr->if_list;
    int subnet_num = 0;
    pthread_mutex_lock(&all_lock);
    for (int i = 0; i < interface_number; i++){
        subnet_num++;
    }
    pthread_mutex_unlock(&all_lock);

    unsigned int len = sizeof(struct sr_ethernet_hdr) + sizeof(struct ip) + sizeof(struct ospfv2_hdr) +
            sizeof(struct ospfv2_lsu_hdr) + subnet_num * sizeof(struct ospfv2_lsu);

    while (walker) {
        uint32_t ip_src = walker->ip;
        uint8_t *packet = (uint8_t *) malloc(len);
        struct sr_ethernet_hdr *ethernet_hdr = (struct sr_ethernet_hdr *) packet;
        ethernet_hdr->ether_type = htons(ETHERTYPE_IP);
        uint8_t broadcast_mac[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        mac_copy(ethernet_hdr->ether_dhost, broadcast_mac);
        mac_copy(ethernet_hdr->ether_shost,walker->addr);

        struct ip *ip_hdr = (struct ip *) (packet + sizeof(struct sr_ethernet_hdr));
        ip_hdr->ip_hl = sizeof(struct ip) / 4;
        ip_hdr->ip_v = 4;
        ip_hdr->ip_tos = 0;
        ip_hdr->ip_len = htons(len - sizeof(struct sr_ethernet_hdr));
        ip_hdr->ip_id = htons(0x2607);
        ip_hdr->ip_off = htons(IP_DF);
        ip_hdr->ip_ttl = 64;
        ip_hdr->ip_p = IPPROTO_OSPF;
        ip_hdr->ip_src.s_addr = ip_src;
        ip_hdr->ip_dst.s_addr = htonl(OSPF_AllSPFRouters);
        ip_hdr->ip_sum = htons(htons(checksum_ip_get(ip_hdr)));

        struct ospfv2_hdr *ospf_hdr = (struct ospfv2_hdr *) (packet + sizeof(struct sr_ethernet_hdr)
                                                             + sizeof(struct ip));
        ospf_hdr->version = OSPF_V2;
        ospf_hdr->type = OSPF_TYPE_LSU;
        ospf_hdr->len = htons(len - sizeof(struct sr_ethernet_hdr) - sizeof(struct ip));
        ospf_hdr->rid = global_rid;
        ospf_hdr->aid = htonl(aid);
        ospf_hdr->autype = 0;
        ospf_hdr->audata = 0;

        struct ospfv2_lsu_hdr *lsu_hdr = (struct ospfv2_lsu_hdr *) (packet + sizeof(struct sr_ethernet_hdr)
                + sizeof(struct ip) + sizeof(struct ospfv2_hdr));
        lsu_hdr->seq = htons(lsu_seq);
        lsu_hdr->unused = 0;
        lsu_hdr->ttl = 64;
        lsu_hdr->num_adv = htonl(subnet_num);

        struct ospfv2_lsu *lsa = (struct ospfv2_lsu *) malloc(subnet_num * sizeof(struct ospfv2_lsu));
        struct ospfv2_lsu *lsa_pkt = (struct ospfv2_lsu *) (packet + sizeof(struct sr_ethernet_hdr)
                                                            + sizeof(struct ip) + sizeof(struct ospfv2_hdr)
                                                            + sizeof(struct ospfv2_lsu_hdr));

        struct sr_if *lsa_walker = sr->if_list;
        for(int i =0;;i++){
            lsa[i].subnet = lsa_walker->ip & lsa_walker->mask;
            lsa[i].mask = lsa_walker->mask;

            // get_neighborID_by_subnet
            unsigned int neighborID = 0;
            for (int j = 0; j < interface_number; ++j) {
                if (neighbors[j].subnet == lsa[i].subnet) {
                    neighborID = j;
                    break;
                }
            }

            if (neighbors[neighborID].rid == 0 && get_interface_name_number(neighbors[neighborID].interface) == 0) {
                lsa[i].subnet = 0;
                lsa[i].mask = 0;
            }
            if(neighbors[neighborID].is_alive){
                lsa[i].rid =neighbors[neighborID].rid;
            }else{
                lsa[i].rid = 0;
            }
            lsa_walker = lsa_walker->next;
            if(!lsa_walker){
                break;
            }
        }

        memcpy(lsa_pkt, lsa, subnet_num * sizeof(struct ospfv2_lsu));
        ospf_hdr->csum = htons(checksum_ospf_get(ospf_hdr));
        sr_send_packet(sr, packet, len, walker->name);
        walker = walker->next;
    }
    pthread_mutex_unlock(&lock);
    lsu_seq++;
}
int get_interface_name_number(char *iface) {
    return iface[3] - (int) '0';
}