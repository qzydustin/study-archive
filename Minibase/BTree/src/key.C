/*
 * key.C - implementation of <key,data> abstraction for BT*Page and
 *         BTreeFile code.
 *
 * Gideon Glass & Johannes Gehrke  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation
 */

#include <string.h>
#include <assert.h>
#include <bits/stdc++.h>
#include "bt.h"

/*
 * See bt.h for more comments on the functions defined below.
 */

/*
 * Reminder: keyCompare compares two keys, key1 and key2
 * Return values:
 *   - key1  < key2 : negative
 *   - key1 == key2 : 0
 *   - key1  > key2 : positive
 */

int keyCompare(const void *key1, const void *key2, AttrType t)
{
    // put your code here
    int compareResult;

    if(t == attrString){
        char *k1 = (char *)key1;
        char *k2 = (char *)key2;
        compareResult = strcmp(k1, k2);
    }else if(t == attrInteger){
        int *k1 = (int *)key1;
        int *k2 = (int *)key2;
        compareResult = *k1 - *k2;
    }else
        return FAIL;

    if(compareResult < 0) return -1;
    else if (compareResult == 0) return 0;
    else return 1;
}

// pack a <Keytype, Datatype> pair in the memory
void make_entry(KeyDataEntry *target,
                AttrType key_type, const void *key,
                nodetype ndtype, Datatype data,
                int *pentry_len) {
    // put your code here
    int keyLen = get_key_length(key, key_type);
    memcpy(target, (char *) key, keyLen); // copy key data

    // add end symbol to the string data
    if (key_type == attrString) {
        char end = '\0';
        memcpy(((char *)target + keyLen), &end, sizeof(char));
        keyLen++;
    }

    // index node
    if (ndtype == INDEX) {
        memcpy(((char *) target + keyLen), &(data.pageNo), sizeof(PageId));
        *pentry_len = sizeof(PageId) + keyLen;
    }
    // leaf node
    else if (ndtype == LEAF) {
        memcpy(((char *) target + keyLen), &(data.rid), sizeof(RID));
        *pentry_len = sizeof(RID) + keyLen;
    }
}



/*
 * get_key_data: unpack a <key,data> pair into pointers to respective parts.
 * Needs a)  memory chunk holding the pair (*psource) and,b) the length
 * of the data chunk (to calculate data start of the <data> part).
 */


void get_key_data(void *targetkey, Datatype *targetdata,
                  KeyDataEntry *psource, int entry_len, nodetype ndtype)
{
    // put your code here
    // find the length of key
    int keyLen, dataLen;
    bool isChar;

    // find the length of data part
    if (ndtype == INDEX)
        dataLen = sizeof(PageId);
    else
        dataLen = sizeof(RID);

    // determine the data type
    keyLen = entry_len - dataLen;
    if (keyLen == sizeof(int))
        isChar = false;
    else
        isChar = true;

    // move key and data to the assigned ptr
    if(isChar){
        memcpy(targetkey, (char *)psource, keyLen);
        memcpy(targetdata, (char *)psource + keyLen, dataLen);
    } else {
        memcpy(targetkey, (int *)psource, keyLen);
        memcpy(targetdata, (int *)psource + 1, dataLen);
    }
}

/*
 * get_key_length: return key length in given key_type
 */
int get_key_length(const void *key, const AttrType key_type)
{
    // put your code here
    if(key_type == attrString)
        return strlen((char *)key);
    else if(key_type == attrInteger)
        return sizeof(int);
    else
        return FAIL;
}

/*
 * get_key_data_length: return (key+data) length in given key_type
 */
int get_key_data_length(const void *key, const AttrType key_type,
                        const nodetype ndtype)
{
    // put your code here
    int keyDataLen = 0;

    if( ndtype == INDEX )
        keyDataLen = get_key_length(key, key_type) + sizeof(PageId);
    else if  (ndtype == LEAF)
        keyDataLen = get_key_length(key, key_type) + sizeof(RID);
    else
        return FAIL;

    return keyDataLen;
}
