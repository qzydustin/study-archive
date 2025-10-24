#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "hfpage.h"
#include "buf.h"
#include "db.h"

// **********************************************************
// page class constructor

void HFPage::init(PageId pageNo) {
    // fill in the body
    slot[0].offset = INVALID_SLOT;
    slot[0].length = EMPTY_SLOT;
    slotCnt = 0;
    usedPtr = MAX_SPACE - DPFIXED;
    freeSpace = MAX_SPACE - DPFIXED;
    prevPage = INVALID_PAGE;
    nextPage = INVALID_PAGE;
    curPage = pageNo;
}

// **********************************************************
// dump page utility
void HFPage::dumpPage() {
    int i;

    cout << "dumpPage, this: " << this << endl;
    cout << "curPage= " << curPage << ", nextPage=" << nextPage << endl;
    cout << "usedPtr=" << usedPtr << ",  freeSpace=" << freeSpace
         << ", slotCnt=" << slotCnt << endl;

    for (i = 0; i < slotCnt; i++) {
        cout << "slot[" << i << "].offset=" << slot[i].offset
             << ", slot[" << i << "].length=" << slot[i].length << endl;
    }
}

// **********************************************************
PageId HFPage::getPrevPage() {
    // fill in the body
    return prevPage;
}

// **********************************************************
void HFPage::setPrevPage(PageId pageNo) {
    // fill in the body
    prevPage = pageNo;
}

// **********************************************************
PageId HFPage::getNextPage() {
    // fill in the body
    return nextPage;
}

// **********************************************************
void HFPage::setNextPage(PageId pageNo) {
    // fill in the body
    nextPage = pageNo;
}

// **********************************************************
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns DONE if sufficient space does not exist
// RID of the new record is returned via rid parameter.
// ***
// ***
Status HFPage::insertRecord(char *recPtr, int recLen, RID &rid) {
    // fill in the body

    // try to find an empty slot
    bool hasEmptySlot = false;
    int emptySlotIndex;
    for (int i = 0; i < slotCnt; i++) {
        hasEmptySlot = slot[i].length == EMPTY_SLOT;
        if (hasEmptySlot) {
            emptySlotIndex = i;
            break;
        }
    }
    if (available_space() < recLen) {
        return DONE;
    }
    if (!hasEmptySlot) {
        emptySlotIndex = slotCnt;
        freeSpace -= sizeof(slot_t);
        slotCnt++;
    }

    // insert the record
    int offset = usedPtr;
    memmove(data + offset - recLen, recPtr, recLen);

    // update
    slot[emptySlotIndex].offset = offset;
    slot[emptySlotIndex].length = recLen;
    freeSpace -= recLen;
    usedPtr -= recLen;

    // return RID
    rid.slotNo = emptySlotIndex;
    rid.pageNo = curPage;
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
// ***
// ***
Status HFPage::deleteRecord(const RID &rid) {
    // fill in the body()
    // check
    if (rid.pageNo != curPage || rid.slotNo < 0 || rid.slotNo > slotCnt) {
        return FAIL;
    }

    // save the record length and offset, need to be used later
    int keyLength = slot[rid.slotNo].length;
    int keyOffset = slot[rid.slotNo].offset;
    // mark slot empty, recycle record space
    slot[rid.slotNo].offset = INVALID_SLOT;
    slot[rid.slotNo].length = EMPTY_SLOT;
    freeSpace += keyLength;
    // compact remaining records
    memmove(data + usedPtr + keyLength, data + usedPtr, keyOffset - usedPtr - keyLength);
    // update slot offset
    for (int i = 0; i < slotCnt; i++) {
        bool isEmpty = slot[i].length == EMPTY_SLOT;
        bool beforeKey = slot[i].offset < keyOffset;
        if (!isEmpty && beforeKey) {
            // slot which before keySlot should change the offset to fit the changed record
            slot[i].offset += keyLength;
        }
    }
    usedPtr += keyLength;

    // recycle empty slot at last, so we need search empty slot from end to first
    for (int i = slotCnt - 1; i > 0; i--) {
        bool isEmpty = slot[i].length == EMPTY_SLOT;
        if (isEmpty) {
            slotCnt--;
            freeSpace += sizeof(slot_t);
        } else {
            // if the empty slot is in the mid, do not delete
            break;
        }
    }
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID &firstRid) {
    // fill in the body
    firstRid.pageNo = curPage;
    for (int i = 0; i < slotCnt; i++) {  // remove +1 after slotCnt
        bool isEmpty = slot[i].length == EMPTY_SLOT;    // use offset to determine emptyness
        if (!isEmpty) {
            firstRid.slotNo = i;
            return OK;
        }
    }
    return DONE;
}

// **********************************************************
// returns RID of next record on the page
// returns DONE if no more records exist on the page; otherwise OK
Status HFPage::nextRecord(RID curRid, RID &nextRid) {
    // fill in the body

    // check RID
    if (curRid.slotNo > slotCnt || curRid.slotNo < 0 || curRid.pageNo != curPage) {
        return FAIL;
    }
    // start at next slot, end at slotCnt(included)
    for (int i = curRid.slotNo + 1; i < slotCnt; i++) { // remove +1 after slotCnt
        bool isEmpty = slot[i].length == EMPTY_SLOT; // use offset to determine emptyness
        if (!isEmpty) {
            nextRid.slotNo = i;
            nextRid.pageNo = curPage;
            return OK;
        }
    }
    return DONE;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char *recPtr, int &recLen) {
    // fill in the body

    // check RID
    if (rid.slotNo > slotCnt || rid.slotNo < 0 || rid.pageNo != curPage) {
        return FAIL;
    }
    slot_t keySlot = slot[rid.slotNo];
    recLen = keySlot.length;
    memcpy(recPtr, data + keySlot.offset - recLen, recLen); // move left a record length
    return OK;
}

// **********************************************************
// returns length and pointer to record with RID rid.  The difference
// between this and getRecord is that getRecord copies out the record
// into recPtr, while this function returns a pointer to the record
// in recPtr.
Status HFPage::returnRecord(RID rid, char *&recPtr, int &recLen) {
    // fill in the body
    // check RID
    if (rid.slotNo > slotCnt || rid.slotNo < 0 || rid.pageNo != curPage) {
        return FAIL;
    }
    slot_t keySlot = slot[rid.slotNo];
    // recPtr points to record address
    recLen = keySlot.length;
    recPtr = data + keySlot.offset - recLen;
    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void) {
    // fill in the body
    freeSpace = usedPtr + sizeof(slot_t) * (1 - slotCnt);
    // search empty slot
    bool hasEmptySlot = false;
    for (int i = 0; i < slotCnt; i++) {   // remove +1 after slotCnt
        hasEmptySlot = slot[i].length == EMPTY_SLOT;
        if (hasEmptySlot) {
            return freeSpace;
        }
    }
    // no empty slot
    if (!hasEmptySlot) {
        // if free space is too small to create a slot, available slot = 0
        if (freeSpace < sizeof(slot_t)) {
            return 0;
        } else {
            return freeSpace - sizeof(slot_t);
        }
    }
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void) {
    // fill in the body
    return (slotCnt == 0 ? 1 : 0);
}