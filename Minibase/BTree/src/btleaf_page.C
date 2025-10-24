/*
 * btleaf_page.C - implementation of class BTLeafPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation
 */
#include <memory.h>
#include "btleaf_page.h"

const char *BTLeafErrorMsgs[] = {
        // OK,
        // Insert Record Failed,
};
static error_string_table btree_table(BTLEAFPAGE, BTLeafErrorMsgs);

/*
 * Status BTLeafPage::insertRec(const void *key,
 *                             AttrType key_type,
 *                             RID dataRid,
 *                             RID& rid)
 *
 * Inserts a key, rid value into the leaf node. This is
 * accomplished by a call to SortedPage::insertRecord()
 * The function also sets up the recPtr field for the call
 * to SortedPage::insertRecord()
 *
 * Parameters:
 *   o key - the key value of the data record.
 *
 *   o key_type - the type of the key.
 *
 *   o dataRid - the rid of the data record. This is
 *               stored on the leaf page along with the
 *               corresponding key value.
 *
 *   o rid - the rid of the inserted leaf record data entry.
 */

Status BTLeafPage::insertRec(const void *key,
                             AttrType key_type,
                             RID dataRid,
                             RID &rid)
{
    // put your code here
    int entryLen;
    Status returnStatus;
    Datatype tmpDatatype; // use temporary datatype variable to hold pageNo
    struct KeyDataEntry *rec = new KeyDataEntry();

    tmpDatatype.rid = dataRid;

    //add key to the index node and return record length
    make_entry(rec, key_type, key, LEAF, tmpDatatype, &entryLen);
    returnStatus = SortedPage::insertRecord(key_type, (char *)rec, entryLen, rid);

    delete rec;
    return returnStatus;
}

/*
 *
 * Status BTLeafPage::get_data_rid(const void *key,
 *                                 AttrType key_type,
 *                                 RID & dataRid)
 *
 * This function performs a binary search to look for the
 * rid of the data record. (dataRid contains the RID of
 * the DATA record, NOT the rid of the data entry!)
 */

Status BTLeafPage::get_data_rid(void *key,
                                AttrType key_type,
                                RID &dataRid)
{
    // put your code here
    Keytype *tmpKey = new Keytype();
    RID tmpRid, testRid;
    Status returnStatus;
    int compareResult;
    bool isEqual = 0;

    // initialize the new key to contain returned value
    returnStatus = get_first(tmpRid, tmpKey, testRid);

    // handle empty key
    if (key == NULL){
        return OK;
    }
    // search for the child key greater than the given key
    while(!isEqual && returnStatus==OK){
        compareResult = keyCompare(key, tmpKey, key_type);
        isEqual = ( compareResult == 0 );
        // break loop immediately when isLE has been satisfied
        if(isEqual) break;

        // get next record
        returnStatus = get_next(tmpRid, tmpKey, testRid);
    }

    if(isEqual)
        dataRid = testRid;
    else
        return RECNOTFOUND;

    return OK;
}

/*
 * Status BTLeafPage::get_first (const void *key, RID & dataRid)
 * Status BTLeafPage::get_next (const void *key, RID & dataRid)
 *
 * These functions provide an
 * iterator interface to the records on a BTLeafPage.
 * get_first returns the first key, RID from the page,
 * while get_next returns the next key on the page.
 * These functions make calls to RecordPage::get_first() and
 * RecordPage::get_next(), and break the flat record into its
 * two components: namely, the key and datarid.
 */
Status BTLeafPage::get_first(RID &rid,
                             void *key,
                             RID &dataRid)
{
    // put your code here
    int recLen = 0;
    struct KeyDataEntry *rec = new KeyDataEntry();
    Datatype tmpDatatype;

    // use rid to receive the first record rid
    HFPage::firstRecord(rid);
    // return the first record to rec
    HFPage::getRecord(rid, (char *)rec, recLen);
    get_key_data(key, &tmpDatatype, rec, recLen, LEAF); // get data from leaf node
    dataRid = tmpDatatype.rid; // return RID of the data record

    return OK;
}

Status BTLeafPage::get_next(RID &rid,
                            void *key,
                            RID &dataRid)
{
    // put your code here
    int nextRecLen = 0;
    RID nextRid;
    Status returnStatus;

    // return the next record
    returnStatus = HFPage::nextRecord(rid, nextRid);
    if( returnStatus == DONE ) return DONE;

    // use next record rid
    struct KeyDataEntry *rec = new KeyDataEntry();
    Datatype tmpDatatype;
    // copy out next record to rec
    HFPage::getRecord(nextRid, (char *)rec, nextRecLen);
    get_key_data(key, &tmpDatatype, rec, nextRecLen, LEAF); // get data from leaf node
    dataRid = tmpDatatype.rid;  // return RID of the data record
    rid = nextRid;

    return OK;
}
