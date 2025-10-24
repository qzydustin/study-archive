/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/

#include "buf.h"

// Define buffer manager error messages here
//enum bufErrCodes  {...};

// Define error message here
static const char *bufErrMsgs[] = {
        // error message strings go here
        "Not enough memory to allocate hashFunction entry",
        "Inserting a duplicate entry in the hashFunction table",
        "Removing a non-existing entry from the hashFunction table",
        "Page not in hashFunction table",
        "Not enough memory to allocate queue node",
        "Poping an empty queue",
        "OOOOOOPS, something is wrong",
        "Buffer pool full",
        "Not enough memory in buffer manager",
        "Page not in buffer pool",
        "Unpinning an unpinned page",
        "Freeing a pinned page"};

// Create a static "error_string_table" object and register the error messages
// with minibase system
static error_string_table bufTable(BUFMGR, bufErrMsgs);

//*************************************************************
//** This is the implementation of BufMgr
//************************************************************

int hashFunction(PageId pageNumber) {
    // a,b is random
    int a = 3, b = 5;
    return (a * pageNumber + b) % HTSIZE;
};

BufMgr::BufMgr(int numbuf, Replacer *replacer) {
    // Initializes a buffer manager managing "numbuf" buffers.
    // Disregard the "replacer" parameter for now. In the full
    // implementation of minibase, it is a pointer to an object
    // representing one of several buffer pool replacement schemes.

    numBuffers = numbuf;
    bufPool = new Page[numBuffers];
    hashTable = new BucketList[numBuffers];
    bufDescr = new Descriptor[numBuffers];
}

BufMgr::~BufMgr() {
    // Flush all valid dirty pages to disk

    flushAllPages();
}

Status BufMgr::pinPage(PageId PageId_in_a_DB, Page *&page, int emptyPage) {
    // Check if this page is in buffer pool, otherwise
    // find a frame for this page, read in and pin it.
    // also write out the old page if it's dirty before reading
    // if emptyPage==TRUE, then actually no read is done to bring
    // the page

    Status status;
    int bucketNo = hashFunction(PageId_in_a_DB);
    BucketList *bucketList = &hashTable[bucketNo];

    while (bucketList != nullptr) {
        if (bucketList->pageNumber == PageId_in_a_DB) {
            // this page is in buffer pool
            bufDescr[bucketList->bufNumber].pin_count++;
            page = &bufPool[bucketList->bufNumber];
            return OK;
        }
        bucketList = bucketList->nextNode;
    }

    // this page is not in buffer pool and buffer pool has space
    for (unsigned int i = 0; i < numBuffers; i++) {
        if (bufDescr[i].pageNumber == INVALID_PAGE) {
            // find the space
            status = MINIBASE_DB->read_page(PageId_in_a_DB, &bufPool[i]);
            if (status != OK) {
                return MINIBASE_CHAIN_ERROR(BUFMGR, status);
            }
            bufDescr[i].pageNumber = PageId_in_a_DB;
            bufDescr[i].pin_count = 1;
            bufDescr[i].dirtyBit = false;
            bufDescr[i].loveIndex = 0;
            bufDescr[i].hateIndex = 0;

            // Point the page at the location in the buffer pool
            page = &bufPool[i];

            //update hashtable.
            bucketList = &hashTable[bucketNo];
            while (bucketList->nextNode) {
                bucketList = bucketList->nextNode;
            }
            auto *newBucketNode = new BucketList();
            newBucketNode->pageNumber = PageId_in_a_DB;
            newBucketNode->bufNumber = i;
            bucketList->nextNode = newBucketNode;
            return OK;
        }
    }

    // this page is not in buffer pool and buffer pool does not have space
    // find a buffer to replace old page with this new page
    int toReplaceBufNumber;
    bool findReplace = false;
    // have hate
    for (unsigned int i = 0; i < numBuffers; i++) {
        if (bufDescr[i].hateIndex > 0 &&
            bufDescr[i].pin_count == 0 &&
            bufDescr[i].loveIndex == 0) {
            if (!findReplace) {
                toReplaceBufNumber = i;
                findReplace = true;
            } else if (bufDescr[i].hateIndex > bufDescr[toReplaceBufNumber].hateIndex) {
                // MRU: choose hateIndex most(the most recent) to replace
                toReplaceBufNumber = i;
            }
        }
    }
    // no hate
    if (!findReplace) {
        for (unsigned int i = 0; i < numBuffers; i++) {
            if (bufDescr[i].loveIndex > 0 &&
                bufDescr[i].pin_count == 0) {
                if (!findReplace) {
                    toReplaceBufNumber = i;
                    findReplace = true;
                } else if (bufDescr[i].loveIndex < bufDescr[toReplaceBufNumber].loveIndex) {
                    // LRU: choose loveIndex least(the least recent) to replace
                    toReplaceBufNumber = i;
                }
            }
        }
    }

    // no frame found to replace, buffer is full
    if (!findReplace) {
        return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERFULL);
    }

    // update bufPool
    // write out old buffer
    status = MINIBASE_DB->write_page(bufDescr[toReplaceBufNumber].pageNumber, &bufPool[toReplaceBufNumber]);
    if (status != OK) {
        return MINIBASE_CHAIN_ERROR(BUFMGR, status);
    }

    // read in new buffer
    if (!emptyPage) {
        status = MINIBASE_DB->read_page(PageId_in_a_DB, &bufPool[toReplaceBufNumber]);
        if (status != OK) {
            return MINIBASE_CHAIN_ERROR(BUFMGR, status);
        }
    }

    // delete old node in bucketList
    int oldBucketNumber = hashFunction(bufDescr[toReplaceBufNumber].pageNumber);
    bucketList = &hashTable[oldBucketNumber];
    BucketList *lastOne;
    while (bucketList != nullptr) {
        if (bucketList->bufNumber == toReplaceBufNumber) {
            lastOne->nextNode = bucketList->nextNode;
        }
        lastOne = bucketList;
        bucketList = bucketList->nextNode;
    }
    // add new node to bucketList
    bucketList = &hashTable[bucketNo];
    while (bucketList->nextNode) {
        bucketList = bucketList->nextNode;
    }
    auto *newBucketNode = new BucketList();
    newBucketNode->pageNumber = PageId_in_a_DB;
    newBucketNode->bufNumber = toReplaceBufNumber;
    bucketList->nextNode = newBucketNode;

    // update bufDescription
    bufDescr[toReplaceBufNumber].pageNumber = PageId_in_a_DB;
    bufDescr[toReplaceBufNumber].pin_count = 1;
    bufDescr[toReplaceBufNumber].dirtyBit = false;
    bufDescr[toReplaceBufNumber].loveIndex = 0;
    bufDescr[toReplaceBufNumber].hateIndex = 0;

    // update page
    page = &bufPool[toReplaceBufNumber];

    return OK;
}

Status BufMgr::unpinPage(PageId page_num, int dirty, int hate) {
    // hate should be TRUE if the page is hated and FALSE otherwise
    // if pincount>0, decrement it and if it becomes zero,
    // put it in a group of replacement candidates.
    // if pincount=0 before this call, return error.

    unsigned int bufNumber;
    for (bufNumber = 0; bufNumber < numBuffers; bufNumber++) {
        if (bufDescr[bufNumber].pageNumber == page_num) {
            break;
        }
    }

    if (bufDescr[bufNumber].pin_count == 0)
        return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERPAGENOTPINNED);

    // decrease pin_count by 1
    bufDescr[bufNumber].pin_count--;

    if (dirty) {
        flushPage(page_num);
    }
    if (hate) {
        int biggestIndex = 0;
        for (unsigned int i = 0; i < numBuffers; i++)
            if (bufDescr[i].pageNumber != INVALID_PAGE) {
                if (bufDescr[i].hateIndex > biggestIndex) {
                    biggestIndex = bufDescr[i].hateIndex;
                }
            }
        bufDescr[bufNumber].hateIndex = biggestIndex + 1;

    } else {
        int biggestIndex = 0;
        for (unsigned int i = 0; i < numBuffers; i++)
            if (bufDescr[i].pageNumber != INVALID_PAGE) {
                if (bufDescr[i].loveIndex > biggestIndex) {
                    biggestIndex = bufDescr[i].loveIndex;
                }
            }
        bufDescr[bufNumber].loveIndex = biggestIndex + 1;

    }
    return OK;
}

Status BufMgr::newPage(PageId &firstPageId, Page *&firstpage, int howmany) {
    // call DB object to allocate a run of new pages and
    // find a frame in the buffer pool for the first page
    // and pin it. If buffer is full, ask DB to deallocate
    // all these pages and return error

    MINIBASE_DB->allocate_page(firstPageId, howmany);
    Status returnStatus = pinPage(firstPageId, firstpage, TRUE);
    if (returnStatus != OK) {
        MINIBASE_DB->deallocate_page(firstPageId, howmany);
        return MINIBASE_CHAIN_ERROR(BUFMGR, returnStatus);
    }
    return OK;
}

Status BufMgr::freePage(PageId globalPageId) {
    // User should call this method if it needs to delete a page
    // this routine will call DB to deallocate the page

    int bucketNo = hashFunction(globalPageId);
    BucketList *hashNode = &hashTable[bucketNo];
    while (hashNode != nullptr) {
        if (hashNode->pageNumber == globalPageId) {
            if (bufDescr[hashNode->bufNumber].pin_count == 0) {
                MINIBASE_DB->deallocate_page(globalPageId);
                return OK;
            }else{
                return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERPAGEPINNED);
            }
        }
        hashNode = hashNode->nextNode;
    }
    return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERPAGENOTFOUND);
}

Status BufMgr::flushPage(PageId pageid) {
    // Used to flush a particular page of the buffer pool to disk
    // Should call the write_page method of the DB class

    int hashKey = hashFunction(pageid);
    BucketList *hashNode = &hashTable[hashKey];

    // find the page in the buffer pool
    while (hashNode != nullptr) {
        // deallocate the page from the database
        if (hashNode->pageNumber == pageid) {
            MINIBASE_DB->write_page(pageid, &bufPool[hashNode->bufNumber]);
            bufDescr[hashNode->bufNumber].dirtyBit = false;
            return OK;
        }
        // next hash pair with the same key value
        hashNode = hashNode->nextNode;
    }
    return MINIBASE_FIRST_ERROR(BUFMGR, BUFFERPAGENOTFOUND);
}


Status BufMgr::flushAllPages() {
    // Flush all pages of the buffer pool to disk, as per flushPage.

    for (unsigned int i = 0; i < numBuffers; i++) {
        if (bufDescr[i].dirtyBit) {
            flushPage(bufDescr[i].pageNumber);
        }
    }
    return OK;
}

/*** Methods for compatibility with project 1 ***/
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page *&page, int emptyPage, const char *filename) {
    return pinPage(PageId_in_a_DB, page, emptyPage);
}

Status BufMgr::unpinPage(PageId globalPageId_in_a_DB, int dirty, const char *filename) {
    return unpinPage(globalPageId_in_a_DB, dirty);
}

unsigned int BufMgr::getNumUnpinnedBuffers() {
    int numUnPinPage = 0;
    for (unsigned int i = 0; i < numBuffers; i++) {
        if (bufDescr[i].pin_count == 0)
            numUnPinPage++;
    }
    return numUnPinPage;
}
