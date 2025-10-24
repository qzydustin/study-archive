/*
 * btfile.C - function members of class BTreeFile
 *
 * Johannes Gehrke & Gideon Glass  951022  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation
 */

#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"

// global

Page *header_page_in_buf;
PageId header_pageID;
int tree_level;
int tree_keysize;
const char *btfilename;

typedef struct {
    AttrType file_AttrType;
    PageId root_pageId;
} Header_page;
Header_page header_page;


// Define your error message here
const char *BtreeErrorMsgs[] = {
        // Possible error messages
        // _OK
        // CANT_FIND_HEADER
        // CANT_PIN_HEADER,
        // CANT_ALLOC_HEADER
        // CANT_ADD_FILE_ENTRY
        // CANT_UNPIN_HEADER
        // CANT_PIN_PAGE
        // CANT_UNPIN_PAGE
        // INVALID_SCAN
        // SORTED_PAGE_DELETE_CURRENT_FAILED
        // CANT_DELETE_FILE_ENTRY
        // CANT_FREE_PAGE,
        // CANT_DELETE_SUBTREE,
        // KEY_TOO_LONG
        // INSERT_FAILED
        // COULD_NOT_CREATE_ROOT
        // DELETE_DATAENTRY_FAILED
        // DATA_ENTRY_NOT_FOUND
        // CANT_GET_PAGE_NO
        // CANT_ALLOCATE_NEW_PAGE
        // CANT_SPLIT_LEAF_PAGE
        // CANT_SPLIT_INDEX_PAGE
};

static error_string_table btree_table(BTREE, BtreeErrorMsgs);

BTreeFile::BTreeFile(Status &returnStatus, const char *filename) {
    // set header_pageID
    if (MINIBASE_DB->get_file_entry(filename, header_pageID) == OK) {
        // exist file in disk
        // read header_page into buf, and set header_page_in_buf
        Status state = MINIBASE_BM->pinPage(header_pageID, header_page_in_buf, 0, filename);
        btfilename = filename;
        returnStatus = state;
    } else {
        returnStatus = FAIL;
    }
}

BTreeFile::BTreeFile(Status &returnStatus, const char *filename,
                     const AttrType keytype, const int keysize) {
    // put your code here
    tree_keysize = keysize;
    Status state;
    if (MINIBASE_DB->get_file_entry(filename, header_pageID) == OK) {
        BTreeFile(returnStatus, filename);
    } else {
        // does not exist file in disk
        // create a btree index file
        if (keytype != attrInteger && keytype != attrString) {
            // We just implement these two kinds of keys in this assignment.
            returnStatus = FAIL;
        } else {
            // create a page
            // set header_page into file
            state = MINIBASE_BM->newPage(header_pageID, header_page_in_buf, 1);
            // newPage include pinPage, no need
            // state = MINIBASE_BM->pinPage(header_pageID, header_page_in_buf, 0, filename);
            state = MINIBASE_DB->add_file_entry(filename, header_pageID);
            header_page.file_AttrType = keytype;
            tree_level = 1;

            // set leaf page
            Page *leaf_page;
            state = MINIBASE_BM->newPage(header_page.root_pageId, leaf_page, 1);

            // init leaf page
            ((BTLeafPage *) leaf_page)->init(header_page.root_pageId);

            // set filename to global
            btfilename = filename;
            returnStatus = state;
        }
    }
}

BTreeFile::~BTreeFile() {
    MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
    MINIBASE_BM->freePage(header_pageID);

}

Status BTreeFile::destroyFile() {
    int current_level = 1;
    Status state;
    PageId curr_pageId = header_page.root_pageId;
    while (current_level < tree_level) {
        Page *index_page;
        MINIBASE_BM->pinPage(curr_pageId, index_page, 0, btfilename);
        PageId get_pageId = -1;
        RID get_rid;
        state = ((BTIndexPage *) index_page)->get_first(get_rid, new Keytype(), get_pageId);
        if (current_level + 1 == tree_level) {
            while (state == OK) {
                //delete all the leaf
                MINIBASE_BM->freePage(get_pageId);
                state = ((BTIndexPage *) index_page)->get_next(get_rid, new Keytype(), get_pageId);
            }
            //delete that index page
            MINIBASE_BM->unpinPage(curr_pageId, 1, btfilename);
            MINIBASE_BM->freePage(curr_pageId);
            //if the index page was root page
            if (curr_pageId == header_page.root_pageId) {
                MINIBASE_DB->delete_file_entry(btfilename);
                MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
                return OK;
            } else {
                MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
                return FAIL;
            }
        }
        curr_pageId = get_pageId;
        current_level++;
    }
    MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
    return FAIL;
}

Status BTreeFile::insert(const void *key, const RID rid) {
    int curr_level = 1;
    PageId curr_pageId = header_page.root_pageId;

    Page *index_page;
    Page *leaf_page;
    Status state;
    char high_half_record[MAX_SPACE / 2] = "";
    PageId last_page_id = INVALID_PAGE;

    while (curr_level <= tree_level) {
        if (curr_level == tree_level) {
            // reach to the leaf
            state = MINIBASE_BM->pinPage(curr_pageId, leaf_page, 0, btfilename);
            if (((BTLeafPage *) leaf_page)->empty()) {
                // the leaf is empty
                ((BTLeafPage *) leaf_page)->init(curr_pageId);
            }
            RID get_rid;
            state = ((BTLeafPage *) leaf_page)->insertRec(key, header_page.file_AttrType, rid, get_rid);
            if (state == OK) {
                // have enough space
                MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
                return OK;
            } else {
                // do not have enough space, split

                state = MINIBASE_BM->pinPage(curr_pageId, leaf_page, 0, btfilename);

                int rec_len = 0;
                int whole_rec = ((BTLeafPage *) leaf_page)->numberOfRecords();

                // split the root node
                int mid_index = ((BTLeafPage *) leaf_page)->numberOfRecords() / 2;

                // create a new page
                PageId new_leaf_pageId = -1;
                Page *new_leaf_page;
                MINIBASE_BM->newPage(new_leaf_pageId, new_leaf_page, 1);
                ((BTLeafPage *) new_leaf_page)->init(new_leaf_pageId);

                // move high half record to new page
                RID old_rid;
                old_rid.pageNo = ((BTLeafPage *) leaf_page)->page_no();
                for (int i = mid_index; i < whole_rec; i++) {
                    old_rid.slotNo = i;
                    char record[MAX_SPACE] = "";
                    //get record & delete record from origin page
                    ((BTLeafPage *) leaf_page)->getRecord(old_rid, record, rec_len);
                    ((BTLeafPage *) leaf_page)->deleteRecord(old_rid);

                    //insert record to new page
                    RID new_rid;
                    ((BTLeafPage *) new_leaf_page)->HFPage::insertRecord(record, rec_len, new_rid);
                    if (i == mid_index) {
                        ((BTLeafPage *) new_leaf_page)->getRecord(new_rid, high_half_record, rec_len);
                    }
                }


                // update update prev and next part
                PageId origin_pageId = ((BTLeafPage *) leaf_page)->page_no();
                PageId new_pageId = ((BTLeafPage *) new_leaf_page)->page_no();
                PageId origin_next_page_id = ((BTLeafPage *) leaf_page)->getNextPage();
                if (origin_next_page_id != INVALID_PAGE) {
                    // origin page has next page
                    Page *origin_next_page;
                    state = MINIBASE_BM->pinPage(origin_next_page_id, origin_next_page, 0, btfilename);
                    // update prev and next
                    ((BTLeafPage *) origin_next_page)->setPrevPage(new_pageId);
                    ((BTLeafPage *) new_leaf_page)->setNextPage(origin_next_page_id);
                    state = MINIBASE_BM->unpinPage(origin_next_page_id, 1, btfilename);
                }
                ((BTLeafPage *) leaf_page)->setNextPage(new_pageId);
                ((BTLeafPage *) new_leaf_page)->setPrevPage(origin_pageId);


                if (origin_pageId == header_page.root_pageId) {
                    // origin page is a root
                    Page *new_root_page;
                    state = MINIBASE_BM->newPage(header_page.root_pageId, new_root_page, 1);
                    ((BTIndexPage *) new_root_page)->init(header_page.root_pageId);
                    RID getrid;

                    if (header_page.file_AttrType == attrInteger) {
                        // type is attrInteger
                        int start_key = -1;
                        ((BTIndexPage *) new_root_page)->insertKey(&start_key, header_page.file_AttrType,
                                                                   origin_pageId, getrid);
                        ((BTIndexPage *) new_root_page)->insertKey(((int *) high_half_record),
                                                                   header_page.file_AttrType,
                                                                   new_pageId, getrid);
                    } else {
                        // type is attrString
                        char start_key[] = "/0";
                        ((BTIndexPage *) new_root_page)->insertKey(start_key, header_page.file_AttrType,
                                                                   origin_pageId, getrid);
                        ((BTIndexPage *) new_root_page)->insertKey(high_half_record, header_page.file_AttrType,
                                                                   new_pageId, getrid);
                    }
                    state = MINIBASE_BM->unpinPage(header_page.root_pageId, 1, btfilename);
                    tree_level++;
                } else {
                    // origin page is not a root
                    Page *u_page;
                    state = MINIBASE_BM->pinPage(last_page_id, u_page, 0, btfilename);
                    if (header_page.file_AttrType == attrInteger) {
                        ((BTIndexPage *) u_page)->insertKey(((int *) high_half_record), header_page.file_AttrType,
                                                            new_pageId, get_rid);
                    } else {
                        ((BTIndexPage *) u_page)->insertKey(high_half_record, header_page.file_AttrType, new_pageId,
                                                            get_rid);
                    }
                    state = MINIBASE_BM->unpinPage(last_page_id, 1, btfilename);
                }
                state = MINIBASE_BM->unpinPage(origin_pageId, 1, btfilename);
                state = MINIBASE_BM->unpinPage(new_pageId, 1, btfilename);
                curr_level = 1;
                //reset the curr_pageId
                curr_pageId = header_page.root_pageId;
            }
        }
        if (curr_level < tree_level) {
            //this is used to iterate the tree to the correct Leaf node
            state = MINIBASE_BM->pinPage(curr_pageId, index_page, 0, btfilename);
            // backup curr_pageId, need to be used next loop
            last_page_id = curr_pageId;
            // update curr_pageId
            ((BTIndexPage *) index_page)->get_page_no(key, header_page.file_AttrType, curr_pageId);
        }
        curr_level++;
    }
    MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);
    return FAIL;
}

Status BTreeFile::Delete(const void *key, const RID rid) {
    int curr_level = 1;
    PageId curr_pageId = header_page.root_pageId;

    Page *index_page;
    Page *leaf_page;
    Status state;

    while (curr_level <= tree_level) {

        if (curr_level == tree_level) {
            // reach to the leaf
            state = MINIBASE_BM->pinPage(curr_pageId, leaf_page, 0, btfilename);

            // RID old_rid;
            // old_rid.pageNo = ((BTLeafPage *) leaf_page)->page_no();

            //loop till you find the low key value in that page
            Keytype *get_key = new Keytype;
            RID get_rid;
            RID get_data_rid;
            state = ((BTLeafPage *) leaf_page)->get_first(get_rid, get_key, get_data_rid);
            while (state == OK) {
                int compare_result = keyCompare(get_key, key, header_page.file_AttrType);
                if (compare_result == 0) {
                    ((BTLeafPage *) leaf_page)->deleteRecord(get_rid);
                    MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);

                    return OK;
                }
                state = ((BTLeafPage *) leaf_page)->get_next(get_rid, get_key, get_data_rid);
            }
        }
        if (curr_level < tree_level) {
            //this is used to iterate the tree to the correct Leaf node
            state = MINIBASE_BM->pinPage(curr_pageId, index_page, 0, btfilename);
            // update curr_pageId
            ((BTIndexPage *) index_page)->get_page_no(key, header_page.file_AttrType, curr_pageId);
        }
        curr_level++;
    }
    MINIBASE_BM->unpinPage(header_pageID, TRUE, btfilename);

    return FAIL;
}

IndexFileScan *BTreeFile::new_scan(const void *lo_key, const void *hi_key) {
    BTreeFileScan *scan = new BTreeFileScan;
    scan->lo_key = lo_key;
    scan->hi_key = hi_key;
    scan->current_rid.pageNo = INVALID_PAGE;
    scan->Attr_type = header_page.file_AttrType;
    PageId curr_pageId = header_page.root_pageId;
    Page *index_page;
    MINIBASE_BM->pinPage(curr_pageId, index_page, 0, btfilename);
    ((BTIndexPage *)index_page)->get_page_no(lo_key, header_page.file_AttrType, scan->leftest_pageID);
    MINIBASE_BM->unpinPage(header_page.root_pageId, 1, btfilename);
    return scan;
}

int keysize() {
    return tree_keysize;
}
