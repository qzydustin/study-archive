/*
 * sorted_page.C - implementation of class SortedPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation
 */
#include <memory.h>
#include "sorted_page.h"
#include "btindex_page.h"
#include "btleaf_page.h"

const char *SortedPage::Errors[SortedPage::NR_ERRORS] = {
        //OK,
        //Insert Record Failed (SortedPage::insertRecord),
        //Delete Record Failed (SortedPage::deleteRecord,
};

/*
 *  Status SortedPage::insertRecord(AttrType key_type,
 *                                  char *recPtr,
 *                                    int recLen, RID& rid)
 *
 * Performs a sorted insertion of a record on an record page. The records are
 * sorted in increasing key order.
 * Only the  slot  directory is  rearranged.  The  data records remain in
 * the same positions on the  page.
 *  Parameters:
 *    o key_type - the type of the key.
 *    o recPtr points to the actual record that will be placed on the page
 *            (So, recPtr is the combination of the key and the other data
 *       value(s)).
 *    o recLen is the length of the record to be inserted.
 *    o rid is the record id of the record inserted.
 */


Status SortedPage::insertRecord(AttrType key_type,
                                char *recPtr,
                                int recLen,
                                RID &rid)
{
    // put your code here
   Status returnStatus;
   int curSlot, newSlot, tmpOffset, tmpLength;
   int compareResult, i;

   returnStatus = HFPage::insertRecord(recPtr, recLen, rid);
   if (returnStatus != OK) return returnStatus;

   // compare the key in slots
   curSlot = rid.slotNo;
   for(i=0; i<numberOfRecords(); i++){

       if(slot[i].length != INVALID_SLOT && i != curSlot){

            if(key_type == attrString) {

                char *curRec = new char[MAX_KEY_SIZE1];
                char *insRec = new char[MAX_KEY_SIZE1];
                memcpy(curRec, &data[slot[i].offset - slot[i].length], slot[i].length);
                memcpy(insRec, recPtr, recLen);

                compareResult = keyCompare(insRec, curRec, key_type);
            } else {

                compareResult = keyCompare(recPtr, &data[slot[i].offset - recLen], key_type);
            }

            if( compareResult <= 0 ){
                break;  // find the first rid that is equal or greater than the inserted record
            }
       }
   }

   newSlot = i;
   if(newSlot == slotCnt)
       return OK;

   tmpOffset = slot[curSlot].offset;
   tmpLength = slot[curSlot].length;
   // if the new slot is on ahead of the current slot, move all slots between them one step backward
    if (newSlot < curSlot)
    {
        for (int i=curSlot; i > newSlot; i--)
        {
            slot[i].offset = slot[i - 1].offset;
            slot[i].length = slot[i - 1].length;

        }
    }
    else    // if the new slot is after of the current slot, move all slots between them one step forward
    {
        for (int i=curSlot; i< (newSlot-1); i++)
        {
            slot[i].offset = slot[i + 1].offset;
            slot[i].length = slot[i + 1].length;

        }
        newSlot = newSlot - 1;
    }

    // finish the final switch
    slot[newSlot].offset = tmpOffset;
    slot[newSlot].length = tmpLength;
    return OK;
}


/*
 * Status SortedPage::deleteRecord (const RID& rid)
 *
 * Deletes a record from a sorted record page. It just calls
 * HFPage::deleteRecord().
 */

Status SortedPage::deleteRecord(const RID &rid)
{
    // put your code here
    return HFPage::deleteRecord(rid);
}

int SortedPage::numberOfRecords()
{
    // put your code here
    int numOfRecords = 0;
    for(int i=0; i<slotCnt; i++){
        if(slot[i].length != INVALID_SLOT)
            numOfRecords++;
    }

    return numOfRecords;
}
