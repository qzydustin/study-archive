//
// Created by Zhenyu Qi on 10/13/21.
//

#ifndef INC_2_ROUTING_TABLE_H
#define INC_2_ROUTING_TABLE_H


struct sr_rt *routing_search_prefix(struct sr_instance *sr,
                                    struct in_addr ip);

#endif //INC_2_ROUTING_TABLE_H
