/*
 * implementation of class Scan for HeapFile project.
 * $Id: scan.C,v 1.1 1997/01/02 12:46:42 flisakow Exp $
 */


#include "heapfile.h"
#include "scan.h"
#include "hfpage.h"
#include "buf.h"
#include "db.h"

// *******************************************
// The constructor pins the first page in the file
// and initializes its private data members from the private data members from hf
Scan::Scan(HeapFile *hf, Status &status) {
    // do constructor work in init function
    status = init(hf);
}

// *******************************************
// The deconstructor unpin all pages.
Scan::~Scan() {
    // do deconstructor work in reset function
    reset();
}

// *******************************************
// Retrieve the next record in a sequential scan.
// Also returns the RID of the retrieved record.
Status Scan::getNext(RID &rid, char *recPtr, int &recLen) {
    // put your code here
    Status status;
    // check whether it has next record, if there is no next record, need nextPage
    if (nxtUserStatus != OK) {
        status = nextDataPage();
        if (status == OK) {
            // do nothing
        }
            // if no next page, return Done
        else {
            return DONE;
        }
    }
    // succeed to find the next record
    // get the record
    status = dataPage->getRecord(userRid, recPtr, recLen);
    if (status == OK) {
        // update the rid to next one
        rid = userRid;
        // store status
        nxtUserStatus = dataPage->nextRecord(userRid, userRid);
        return OK;
    } else {
        return FAIL;
    }

}

// *******************************************
// Do all the constructor work.
Status Scan::init(HeapFile *hf) {
    // put your code here
    dirPageId = 0;
    dataPageId = 0;
    dirPage = nullptr;
    dataPage = nullptr;
    nxtUserStatus = 0;
    _hf = hf;
    scanIsDone = False;
    return firstDataPage();;
}

// *******************************************
// Reset everything and unpin all pages.
Status Scan::reset() {

    // reset dirPage
    // if dirPage pinned
    if (dirPageId != INVALID_PAGE) {
        MINIBASE_BM->unpinPage(dirPageId);
        dirPageId = INVALID_PAGE;
    }
    // reset dataPage
    // if dataPage pinned
    if (dataPageId != INVALID_PAGE) {
        MINIBASE_BM->unpinPage(dataPageId);
        dataPageId = INVALID_PAGE;
    }

    dirPage = nullptr;
    dataPage = nullptr;
    scanIsDone = false;
    nxtUserStatus = DONE;
    return DONE;
}

// *******************************************
// Copy data about first page in the file.
Status Scan::firstDataPage() {
    // put your code here

    // set dirPage
    dirPageId = _hf->firstDirPageId;
    MINIBASE_BM->pinPage(dirPageId, reinterpret_cast<Page *&>(dirPage));

    // set dataPage
    dirPage->firstRecord(dataPageRid);
    // get record
//    auto *dataPageInfo = new DataPageInfo();
    DataPageInfo *dataPageInfo = new DataPageInfo();
    int recLen;
    // returnRecord can save memory
    dirPage->returnRecord(dataPageRid, reinterpret_cast<char *&>(dataPageInfo), recLen);
    // get record.id
    dataPageId = dataPageInfo->pageId;
    MINIBASE_BM->pinPage(dataPageId, reinterpret_cast<Page *&>(dataPage));

    // set dataPage
    dataPage->firstRecord(userRid);
    return OK;
}

// *******************************************
// Retrieve the next data page.
Status Scan::nextDataPage() {
    Status status;
    RID nextDirPageRID{};

    // get next DirPage RID
    status = dirPage->nextRecord(dataPageRid, nextDirPageRID);
    // do not find the next record
    if (status == OK) {
        // continue
    } else if (status == DONE && nextDirPage() == OK) {
        // if it is the last record, and it has next page
        // need to check next page's first record
        // get next page's first record, continue
        dirPage->firstRecord(nextDirPageRID);
    } else {
        // no next, exit
        return reset();
    }
    // unpin
    MINIBASE_BM->unpinPage(dataPageId);

    // get next pageRid
    dataPageRid = nextDirPageRID;

    // get DataPageInfo
    auto *dataPageInfo = new DataPageInfo();
    int len;
    dirPage->returnRecord(dataPageRid, reinterpret_cast<char *&>(dataPageInfo), len);

    // pin
    dataPageId = dataPageInfo->pageId;
    MINIBASE_BM->pinPage(dataPageId, reinterpret_cast<Page *&>(dataPage));

    dataPage->firstRecord(userRid);

    return OK;
}

// *******************************************
// Retrieve the next directory page.
Status Scan::nextDirPage() {
    MINIBASE_BM->unpinPage(dirPageId);
    // if it is the last page, cannot find next page
    dirPageId = dirPage->getNextPage();
    if (dirPageId == INVALID_PAGE) {
        // do I need to "dirPageId = dirPage->getNextPage();"?
        // need to do that to pass the test.
        return DONE;
    } else {
        MINIBASE_BM->pinPage(dirPageId, reinterpret_cast<Page *&>(dirPage));
        return OK;
    }
}
