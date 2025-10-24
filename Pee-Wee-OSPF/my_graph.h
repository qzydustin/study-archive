//
// Created by Zhenyu Qi on 12/3/21.
//

#ifndef PEEWEEOSPF_MY_GRAPH_H
#define PEEWEEOSPF_MY_GRAPH_H

struct edge {
    uint32_t from, to;
    int weight;
};

struct adjList {
    struct edge e;
    struct adjList *next;
    struct adjList *prev;
};

struct vertex {
    uint32_t rid;
    uint32_t id;
    int type;
    uint32_t subnet;
    uint32_t nmask;
    int d;
    uint32_t parent;
    int is_visited;
    int latestSeqNum;
    struct adjList *head;
};

struct vertexList {
    struct vertex v;
    struct vertexList *next;
};

struct path {
    struct vertex v;
    struct path *next;
};

struct graph {
    int V;
    struct vertexList *vList;
};

void add_vertex(uint32_t rid, uint32_t subnet, uint32_t nmask, int type);

void add_edge(struct vertexList *v, struct vertexList *w, int weight);

void dijkstra_algorithm(struct vertexList *list);

void delete_node(struct vertexList *list, struct adjList *node);

struct adjList *get_edge(struct vertexList *list, unsigned int to);

struct vertexList *get_vertex(unsigned int id);

int is_edge_present(uint32_t from, uint32_t to);

struct vertexList *get_node_by_rid(uint32_t rid);

struct vertexList *get_node_by_subnet(uint32_t subnet);

#endif //PEEWEEOSPF_MY_GRAPH_H
