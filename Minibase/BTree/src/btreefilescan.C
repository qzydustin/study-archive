/*
 * btreefilescan.C - function members of class BTreeFileScan
 *
 * Spring 14 CS560 Database Systems Implementation
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu)
 */

#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"


/*
 * Note: BTreeFileScan uses the same errors as BTREE since its code basically
 * BTREE things (traversing trees).
 */


BTreeFileScan::~BTreeFileScan() {
    MINIBASE_BM->unpinPage(current_rid.pageNo, FALSE, FALSE);
}

Status BTreeFileScan::get_next(RID &rid, void *keyptr) {
    if (Attr_type == attrString) {

        memset(keyptr, '\0', MAX_KEY_SIZE1);
    }
    Page *leftest_page;
    Status state;
    if (current_rid.pageNo == INVALID_PAGE) //this is the 1st rid means the lowest
    {
        state = MINIBASE_BM->pinPage(leftest_pageID, leftest_page, 0);

        state = ((BTLeafPage *)leftest_page)->get_first(current_rid, keyptr, rid);

        if (lo_key == nullptr && hi_key == nullptr) {
            state = MINIBASE_BM->unpinPage(leftest_pageID, FALSE, FALSE);
            return OK;
        }

        if (lo_key == nullptr && hi_key != nullptr) {
            state = MINIBASE_BM->unpinPage(leftest_pageID, FALSE, FALSE);
            if (keyCompare(hi_key, keyptr, Attr_type) < 0)
                return DONE;
            return OK;
        }

        if (lo_key != nullptr) {
            while (state == OK) {
                RID backup_current_rid;
                RID backup_rid;
                char backup_keyptr[MAX_KEY_SIZE1] = {'\0'};

                if (keyCompare(lo_key, keyptr, Attr_type) <= 0) {
                    // back up current rid and datarid
                    backup_current_rid= current_rid;
                    backup_rid = rid;
                    memcpy(backup_keyptr,keyptr,MAX_KEY_SIZE1);

                    ((BTLeafPage *)leftest_page)->get_next(current_rid, keyptr, rid);

                    if (keyCompare(backup_keyptr, keyptr, Attr_type) == 0) {
                        continue;
                    } else {
                        // rollback to last loop
                        current_rid = backup_current_rid;
                        rid=backup_rid;
                        memcpy(keyptr,backup_keyptr,MAX_KEY_SIZE1);

                        MINIBASE_BM->unpinPage(leftest_pageID, FALSE, FALSE);
                        if (hi_key != nullptr){
                            if (keyCompare(hi_key, keyptr, Attr_type) < 0){
                                return DONE;
                            }
                        }
                        return OK;
                    }
                }
                state = ((BTLeafPage *)leftest_page)->get_next(current_rid, keyptr, rid);
            }
            return DONE;
        }
    } else {
        state = MINIBASE_BM->pinPage(current_rid.pageNo, leftest_page, 0);

        state = ((BTLeafPage *)leftest_page)->get_next(current_rid, keyptr, rid);

        // search for repeated key
        while (state == OK) {
            Status  buackup_state = state;
            RID backup_current_rid = current_rid;
            RID backup_rid = rid;
            char tmp_keyptr[MAX_KEY_SIZE1] = {'\0'};

            state = ((BTLeafPage *)leftest_page)->get_next(current_rid, tmp_keyptr, rid);

            if (keyCompare(tmp_keyptr, keyptr, Attr_type) != 0) {
                // rollback to last loop
                rid=backup_rid;
                current_rid = backup_current_rid ;
                state = buackup_state ;
                break;
            }
        }


        MINIBASE_BM->unpinPage(current_rid.pageNo, FALSE, FALSE);

        if (hi_key != nullptr)
            if (keyCompare(hi_key, keyptr, Attr_type) < 0)
                return DONE;

        if (state == OK)
            return OK;


        // reach the end of leaf nodes
        if (((BTLeafPage *)leftest_page)->getNextPage() == -1)
            return DONE;


        state = MINIBASE_BM->pinPage(((BTLeafPage *)leftest_page)->getNextPage(), leftest_page, 0);
        state = ((BTLeafPage *)leftest_page)->get_first(current_rid, keyptr, rid);
        state = MINIBASE_BM->unpinPage(current_rid.pageNo, FALSE, FALSE);

        return OK;
    }
}


Status BTreeFileScan::delete_current() {
    // put your code here
    Page *leaf_page;
    MINIBASE_BM->pinPage(current_rid.pageNo, leaf_page, 0);
    PageId nextPage = ((BTLeafPage *) leaf_page)->getNextPage();
    if (nextPage == INVALID_PAGE) {
        leftest_pageID = nextPage;
    }
    return MINIBASE_BM->unpinPage(current_rid.pageNo, 1, FALSE);

}

int BTreeFileScan::keysize() {
    // put your code here
    if (Attr_type == attrInteger)
        return sizeof(int);
    else if (Attr_type == attrString)
        return MAX_KEY_SIZE1;
    else
        return -1;
}
