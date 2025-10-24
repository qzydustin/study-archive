/*
 * btindex_page.C - implementation of class BTIndexPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation
 */
#include <memory.h>
#include "btindex_page.h"

// Define your Error Messge here
const char *BTIndexErrorMsgs[] = {
        //Possbile error messages,
        //OK,
        //Record Insertion Failure,
};

static error_string_table btree_table(BTINDEXPAGE, BTIndexErrorMsgs);

Status BTIndexPage::insertKey(const void *key,
                              AttrType key_type,
                              PageId pageNo,
                              RID &rid)
{
    // put your code here
    int entryLen;
    Status returnStatus;
    Datatype tmpDatatype; // use temporary datatype variable to hold pageNo
    struct KeyDataEntry *rec = new KeyDataEntry();

    tmpDatatype.pageNo = pageNo;

    //add key to the index node and return record length
    make_entry(rec, key_type, key, INDEX, tmpDatatype, &entryLen);
    returnStatus = SortedPage::insertRecord(key_type, (char *)rec, entryLen, rid);

    delete rec;
    return returnStatus;
}


Status BTIndexPage::deleteKey(const void *key, AttrType key_type, RID &curRid)
{
    // put your code here
    Status returnStatus;
    RID tmpRid;
    Keytype tmpKey;
    PageId tmpPageId;
    bool isEqual = 0;

    // start from the very first record to compare
    returnStatus = get_first(tmpRid, &tmpKey, tmpPageId);
    if(returnStatus != OK) return returnStatus;

    while(!isEqual && returnStatus==OK){
        isEqual = keyCompare(key, &tmpKey, key_type) == 0;
        if(isEqual) break;
        // move to the next record if not equal
        returnStatus = get_next(tmpRid, &tmpKey, tmpPageId);
    }

    // not find the record
    if(returnStatus != OK) return RECNOTFOUND;

    curRid = tmpRid;
    returnStatus = SortedPage::deleteRecord(curRid);

    return returnStatus;
}


Status BTIndexPage::get_page_no(const void *key,
                                AttrType key_type,
                                PageId &pageNo)
{
    // put your code here
    Keytype *tmpKey = new Keytype();
    RID tmpRid;
    PageId newPageId, oldPageId;
    Status returnStatus;
    int compareResult;
    bool isLE = 0;

    // initialize the new key to contain returned value
    returnStatus = get_first(tmpRid, tmpKey, newPageId);
    oldPageId = newPageId;

    // handle empty key
    if (key == NULL){
        pageNo = oldPageId;
        return OK;
    }
    // search for the child key greater than the given key
    while(!isLE && returnStatus==OK){
        compareResult = keyCompare(key, tmpKey, key_type);
        isLE = ( compareResult <= 0 );
        // break loop immediately when isLE has been satisfied
        if(isLE) break;

        // get next record
        oldPageId = newPageId;
        returnStatus = get_next(tmpRid, tmpKey, newPageId);
    }

    if(compareResult == 0)
        pageNo = newPageId;
    else
        pageNo = oldPageId;

    return OK;
}

Status BTIndexPage::get_first(RID &rid,
                              void *key,
                              PageId &pageNo)
{
    // put your code here
    int recLen = 0;
    struct KeyDataEntry *rec = new KeyDataEntry();
    Datatype tmpDatatype;

    // use rid to receive the first record rid
    HFPage::firstRecord(rid);
    // return the first record to rec
    HFPage::getRecord(rid, (char *)rec, recLen);
    get_key_data(key, &tmpDatatype, rec, recLen, INDEX);
    pageNo = tmpDatatype.pageNo;

    return OK;
}

Status BTIndexPage::get_next(RID &rid, void *key, PageId &pageNo)
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
    get_key_data(key, &tmpDatatype, rec, nextRecLen, INDEX);
    pageNo = tmpDatatype.pageNo;
    rid = nextRid;

    return OK;
}
