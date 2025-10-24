//
// Created by Zhenyu Qi on 10/13/21.
//

#ifndef INC_2_TOOL_H
#define INC_2_TOOL_H


int get_checksum(const char *header, int length);

int is_checksum_correct(const char *header, int length);

void copy_mac(uint8_t *dest, const uint8_t *src);


#endif //INC_2_TOOL_H
