//
// Created by Zhenyu Qi on 10/13/21.
//

#ifndef INC_2_ICMP_H
#define INC_2_ICMP_H

struct icmp {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
} __attribute__ ((packed));

#endif //INC_2_ICMP_H
