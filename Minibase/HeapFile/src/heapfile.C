#include "heapfile.h"

// ******************************************************
// Error messages for the heapfile layer

static const char *hfErrMsgs[] = {
        "bad record id",
        "bad record pointer",
        "end of file encountered",
        "invalid update operation",
        "no space on page for record",
        "page is empty - no records",
        "last record on page",
        "invalid slot number",
        "file has already been deleted",
};

static error_string_table hfTable( HEAPFILE, hfErrMsgs );

// ********************************************************
// Constructor
HeapFile::HeapFile( const char *name, Status& returnStatus )
{
    // fill in the body

    // return FAIL if the input HeapFile name overflows
    if( strlen(name) > MAX_NAME ){

        returnStatus = FAIL;
        return;
    }

    // construct a temporary heapfile if a null name has being input
    if( strlen(name) == 0 ){

        fileName = strdup("TEMP_HEAP");
    } else{

        fileName = new char[strlen(name)+1];
        memcpy((char *&)fileName, name, strlen(name));
    }

    // check whether the name already denotes a file 
    // by using get_file_entry under MINIBASE_DB
    returnStatus = MINIBASE_DB->get_file_entry(fileName, firstDirPageId);

    if( returnStatus == OK ){

        // name already denotes a file
    } else {

        // such named file does not exists, a new empty file is created.
        // initialize delete flag by false
        file_deleted = false;

        // Using buffer manager to creat a new directory page
        Page *dirPage;
        returnStatus = MINIBASE_BM->newPage(firstDirPageId, dirPage);

        // add the new directory page into DB
        returnStatus = MINIBASE_DB->add_file_entry(fileName, firstDirPageId);

        // creat a HFPage file for the new directory file
        ((HFPage *)dirPage)->init(firstDirPageId);
        // returnStatus = MINIBASE_DB->read_page(firstDirPageId, newDirPage);

        // unpin the new directory page, enable dirty flag to write it back to the disk
        returnStatus = MINIBASE_BM->unpinPage(firstDirPageId, 1, fileName);
    }

    returnStatus = OK;

}

// ******************
// Destructor
HeapFile::~HeapFile()
{
    // fill in the body

    // write all dirty pages from buffer into the disk
    MINIBASE_BM->flushAllPages();

    // delete heap file if it is temporary
    if( fileName == NULL )
        deleteFile();
}

// *************************************
// Return number of records in heap file
int HeapFile::getRecCnt()
{
    // fill in the body

    int totalRecCnt = 0;
    Status returnDirStatus = DONE, dataPageStatus = DONE;
    HFPage *readHFPage;
    PageId readHFPageID, nextPageID;

    RID readRID, prevRID;
    char *readRec;
    int readRecLen;
    DataPageInfo *readPageInfo;

    // start from the first page
    nextPageID = firstDirPageId;

    // search all directory files in the DBS, read each dir file into the momery
    while(nextPageID != INVALID_PAGE){


        // pin next dir page to tempPage in memory if it exists
        returnDirStatus = MINIBASE_BM->pinPage(nextPageID, (Page*&)readHFPage, 0, fileName);


        // search all records, one record corresponds to one data page, in a directory file
        readRID.pageNo = nextPageID;
        dataPageStatus = readHFPage->firstRecord(readRID);
        while(dataPageStatus == OK){

            readHFPage->returnRecord(readRID, readRec, readRecLen);
            readPageInfo = (DataPageInfo *)readRec;
            totalRecCnt += readPageInfo->recct;

            prevRID = readRID;
            dataPageStatus = readHFPage->nextRecord(prevRID, readRID);
        }


        // reach the end of one dir page, unpin the current dir page
        // move to the next dir file
        readHFPageID = nextPageID;
        nextPageID = readHFPage->getNextPage();
        returnDirStatus = MINIBASE_BM->unpinPage(readHFPageID, 0, fileName);
    }

    return totalRecCnt;
}

// *****************************
// Insert a record into the file
Status HeapFile::insertRecord(char *recPtr, int recLen, RID& outRid)
{
    // fill in the body
    Status dirStatus = DONE, dataStatus = DONE;
    PageId nextDirPageID, dataPageID, unpinPageId;
    HFPage *dirPage, *dataPage;
    RID dirRecRID;

    DataPageInfo *dpInfo;
    int dpInfoLen;

    // return error code when encoutering an invalid input string
    if (recLen > 1000)
    {
        MINIBASE_FIRST_ERROR(HEAPFILE, *hfErrMsgs[4]);
        return HEAPFILE;
    }


    //  run a loop over all dir pages to find an available space
    nextDirPageID = firstDirPageId;
    dpInfo = new DataPageInfo();
    while(nextDirPageID != INVALID_PAGE){

        // pin next page
        dirStatus = MINIBASE_BM->pinPage(nextDirPageID, (Page *&) dirPage);

        dirStatus = dirPage->firstRecord(dirRecRID);

        // In each dir page, check every data page to find an available space
        while(dirStatus == OK){

            // get record in dir page containing the data page info
            dataStatus = dirPage->getRecord(dirRecRID, (char *&)dpInfo, dpInfoLen);

            // not reach the end of dir page
            // skip all data page without enough space
            if(dpInfo->availspace < recLen){

                dirStatus = dirPage->nextRecord(dirRecRID, dirRecRID);
            }


            // if find a data page having enough space
            if(dpInfo->availspace >= recLen && dataStatus == OK){

                // return record to get data page id and its pointer
                dataStatus = dirPage->returnRecord(dirRecRID, (char *&)dpInfo, dpInfoLen);
                dataPageID = dpInfo->pageId;
                dataStatus = MINIBASE_BM->pinPage(dataPageID, (Page *&)dataPage);

                // insert record into the data page
                dataPage->insertRecord(recPtr, recLen, outRid);

                // update data page record RID
                dpInfo->availspace = dataPage->available_space();
                dpInfo->recct++;

                // unpin data page and dir page
                MINIBASE_BM->unpinPage(dataPageID, 1);
                MINIBASE_BM->unpinPage(nextDirPageID, 1);

                return OK;
            }
        }

        // unpin the current dir file and move to the next
        unpinPageId = nextDirPageID;
        nextDirPageID = dirPage->getNextPage();
        dirStatus = MINIBASE_BM->unpinPage(unpinPageId, 0);
    }

    // if inserting record unsuccessfully on the existing data pages
    // creat a new data page and pin it

    dataStatus = newDataPage(dpInfo);
    dataPageID = dpInfo->pageId;
    dataStatus = MINIBASE_BM->pinPage(dataPageID, (Page *&)dataPage);

    // insert record into the data page and update its DataPageInfo
    dpInfo->recct++;
    dataStatus = dataPage->insertRecord(recPtr, recLen, outRid);
    dpInfo->availspace = dataPage->available_space();

    // alocatate space for the dpInfo in the dir page
    dirStatus = allocateDirSpace(dpInfo, nextDirPageID, dirRecRID);
    dirStatus = MINIBASE_BM->pinPage(nextDirPageID, (Page *&)dirPage);

    //unpin data page
    dataStatus = MINIBASE_BM->unpinPage(dataPageID, 1, fileName);
    dirStatus = MINIBASE_BM->unpinPage(nextDirPageID, 1, fileName);

    if(dataStatus != OK || dirStatus != OK){
        return dirStatus;
    }

    return OK;
}




// ***********************
// delete record from file
Status HeapFile::deleteRecord(const RID& rid)
{
    // fill in the body
    PageId dirPageId, dataPageId;
    HFPage *dirPage, *dataPage;
    Status returnStatus;
    RID dataInDir;


    returnStatus = findDataPage(rid,
                                dirPageId, dirPage,
                                dataPageId, dataPage, dataInDir);


    // if not found return error code
    if(returnStatus != OK){
        return returnStatus;
    }

    // delete record in the data page
    returnStatus = dataPage->deleteRecord(rid);
    if(dataPage->empty()){
        dirPage->deleteRecord(dataInDir);
    }

    // update data page and write it back to the disk
    returnStatus = MINIBASE_BM->unpinPage(dataPageId, 1);
    // update dir page and write it back to the disk
    returnStatus = MINIBASE_BM->unpinPage(dirPageId, 1);

//    if(dataPage->empty())
//        returnStatus = MINIBASE_BM->freePage(dataPageId);

    if(dirPage->empty())
        returnStatus = MINIBASE_BM->freePage(dirPageId);

    if(returnStatus != OK){
        return returnStatus;
    }

    return OK;
}

// *******************************************
// updates the specified record in the heapfile.
Status HeapFile::updateRecord (const RID& rid, char *recPtr, int recLen)
{
    // fill in the body

    Status returnStatus;
    PageId returnDirPageId, returnDataPageId;
    HFPage *returnDirPage, *returnDataPage;
    RID returnRid;

    char *returnRecPtr;
    int returnRecLen;

    // get the return record
    returnStatus = findDataPage(rid, returnDirPageId, returnDirPage,
                                returnDataPageId, returnDataPage, returnRid);

    // if fail to find the recording, unpin dir and data pages
    if(returnStatus != OK){

        MINIBASE_BM->unpinPage(returnDirPageId, 0);
        MINIBASE_BM->unpinPage(returnDataPageId, 0);

        return returnStatus;
    }

    // find the record in the return dir and data pages
    returnStatus = returnDataPage->returnRecord(rid, returnRecPtr, returnRecLen);

    // if recording lengths differ, return error
    if(returnRecLen != recLen){

        MINIBASE_BM->unpinPage(returnDirPageId, 0);
        MINIBASE_BM->unpinPage(returnDataPageId, 0);

        MINIBASE_FIRST_ERROR(HEAPFILE, *hfErrMsgs[4]);
        return HEAPFILE;
    }
    memmove(returnRecPtr, recPtr, recLen);

    // unpin dir page and data page
    returnStatus = MINIBASE_BM->unpinPage(returnDirPageId);
    returnStatus = MINIBASE_BM->unpinPage(returnDataPageId);

    if(returnStatus != OK){
        return returnStatus;
    }

    return OK;
}

// ***************************************************
// read record from file, returning pointer and length
Status HeapFile::getRecord (const RID& rid, char *recPtr, int& recLen)
{
    // fill in the body

    Status returnStatus;
    PageId returnDirPageId, returnDataPageId;
    HFPage *returnDirPage, *returnDataPage;
    RID returnRid;

    // get the return record
    returnStatus = findDataPage(rid, returnDirPageId, returnDirPage,
                                returnDataPageId, returnDataPage, returnRid);

    // if fail to find the recording, unpin dir and data pages
    if(returnStatus != OK){

        MINIBASE_BM->unpinPage(returnDirPageId, 0);
        MINIBASE_BM->unpinPage(returnDataPageId, 0);

        return returnStatus;
    }

    // store record pointer and length
    returnStatus = returnDataPage->getRecord(rid, recPtr, recLen);
    // unpin dir page and data page
    returnStatus = MINIBASE_BM->unpinPage(returnDirPageId, 0);
    returnStatus = MINIBASE_BM->unpinPage(returnDataPageId, 0);

    if(returnStatus != OK){
        return returnStatus;
    }
    return OK;
}

// **************************
// initiate a sequential scan
Scan *HeapFile::openScan(Status& status)
{
    // fill in the body
    Scan *open_scan = new Scan(this, status);
    return open_scan;
}

// ****************************************************
// Wipes out the heapfile from the database permanently.
Status HeapFile::deleteFile()
{
    // fill in the body
    if(file_deleted == true)
        return DONE;

    Status returnStatus;
    PageId dirPageId, freePageId;
    HFPage *dirPage;
    RID returnRid;

    char *returnRecPtr;
    int returnRecLen = 0;
    DataPageInfo *dpInfo = new DataPageInfo();

    // search all dir pages
    dirPageId = firstDirPageId;

    while(dirPageId != INVALID_PAGE){

        // start from the first record in the first dir page
        returnStatus = MINIBASE_BM->pinPage(dirPageId, (Page *&)dirPage);
        returnStatus = dirPage->firstRecord(returnRid);

        while(returnStatus == OK){

            // free data page according to the return ID from recording
            returnStatus = dirPage->getRecord(returnRid, returnRecPtr, returnRecLen);
            dpInfo = (DataPageInfo *&)returnRecPtr;
            returnStatus = MINIBASE_BM->freePage(dpInfo->pageId);

            returnStatus = dirPage->nextRecord(returnRid, returnRid);
        }

        // iteration reaches the end of one dir page
        freePageId = dirPageId;
        dirPageId = dirPage->getNextPage();
        returnStatus = MINIBASE_BM->unpinPage(freePageId);
        returnStatus = MINIBASE_BM->freePage(freePageId);
    }

    // free all dir and data pages, change file deleted flag and delete all entries
    file_deleted = true;
    returnStatus = MINIBASE_DB->delete_file_entry(fileName);

    if(returnStatus != OK){
        return returnStatus;
    }

    return OK;
}

// ****************************************************************
// Get a new datapage from the buffer manager and initialize dpinfo
// (Allocate pages in the db file via buffer manager)
Status HeapFile::newDataPage(DataPageInfo *dpinfop)
{
    // fill in the body
    Status returnStatus;
    HFPage *newPage;
    PageId newPageId;

    // use newPage API to add a page in the buffer pool
    returnStatus = MINIBASE_BM->newPage(newPageId, (Page *&)newPage, 1);

    newPage->init(newPageId);

    // update dpinfop in the dir page
    dpinfop->pageId = newPageId;
    dpinfop->availspace = newPage->available_space();
    dpinfop->recct = 0;

    // unpin the new data page
    returnStatus = MINIBASE_BM->unpinPage(newPageId);

    if(returnStatus != OK){
        return returnStatus;
    }

    return OK;
}

// ************************************************************************
// Internal HeapFile function (used in getRecord and updateRecord): returns
// pinned directory page and pinned data page of the specified user record
// (rid).
//
// If the user record cannot be found, rpdirpage and rpdatapage are
// returned as NULL pointers.
//
Status HeapFile::findDataPage(const RID& rid,
                              PageId &rpDirPageId, HFPage *&rpdirpage,
                              PageId &rpDataPageId,HFPage *&rpdatapage,
                              RID &rpDataPageRid)
{
    // fill in the body
    Status returnStatus;
    PageId dirPageId, unpinPageId;
    HFPage *dirPage;

    RID returnRid;
    int returnRecLen = 0;
    DataPageInfo *dpInfo = new DataPageInfo();

    // initialize return page pointer as NULL
    rpdirpage = NULL;
    rpdatapage = NULL;

    // start from the first dir page
    dirPageId = firstDirPageId;
    while(dirPageId != INVALID_PAGE){

        // start from the first record in the first dir page
        returnStatus = MINIBASE_BM->pinPage(dirPageId, (Page *&)dirPage);
        returnStatus = dirPage->firstRecord(returnRid);

        // empty record in the dir page
        if(returnStatus != OK){
            return DONE;
        }

        while(returnStatus == OK){

            // retrieve recording in dir page
            returnStatus = dirPage->getRecord(returnRid, (char *&)dpInfo, returnRecLen);
            // find the data page, return its pointer and pageId, as well as its dir

            if(dpInfo->pageId == rid.pageNo && returnStatus == OK){

                rpDirPageId = dirPageId;
                rpDataPageId = dpInfo->pageId;
                rpdirpage = dirPage;
                returnStatus = MINIBASE_BM->pinPage(rpDataPageId, (Page *&)rpdatapage);
                rpDataPageRid = returnRid;
                return OK;
            }

            returnStatus = dirPage->nextRecord(returnRid, returnRid);
        }

        unpinPageId = dirPageId;
        dirPageId = dirPage->getNextPage();
        returnStatus = MINIBASE_BM->unpinPage(unpinPageId);
    }


    // no data page has been found
    dirPageId = 0;

    if(returnStatus != DONE){
        return returnStatus;
    }

    return DONE;
}

// *********************************************************************
// Allocate directory space for a heap file page

Status HeapFile::allocateDirSpace(struct DataPageInfo * dpinfop,
                                  PageId &allocDirPageId,
                                  RID &allocDataPageRid)
{
    // fill in the body
    Status returnStatus;
    PageId dirPageId, unpinPageId;
    HFPage *dirPage, *oldLastPage;

    // search through current dir pages to find a space
    dirPageId = firstDirPageId;
    unpinPageId = dirPageId;
    while(dirPageId != INVALID_PAGE){

        // start from the given dir page Id
        returnStatus = MINIBASE_BM->pinPage(dirPageId, (Page *&)dirPage);

        // dir page doe not have enough space for inserting a dpinfop
        if(dirPage->available_space() < int(sizeof(DataPageInfo))){
            unpinPageId = dirPageId;
            dirPageId = dirPage->getNextPage();
            returnStatus = MINIBASE_BM->unpinPage(unpinPageId, 0);
        }

        // dir page has enough space for inserting a dpinfop
        if(dirPage->available_space() >= int(sizeof(DataPageInfo))){

            returnStatus = dirPage->insertRecord((char *)dpinfop, sizeof(DataPageInfo), allocDataPageRid);
            allocDirPageId = dirPageId;
            returnStatus = MINIBASE_BM->unpinPage(dirPageId, 1);
            return returnStatus;
        }
    }

    // not find available space in the previous steps,
    // create a new dir page and link it to the last dir page has been visited

    returnStatus = MINIBASE_BM->newPage(dirPageId, (Page *&)dirPage);
    dirPage->init(dirPageId);

    // insert dpinfop into the new dir page
    dirPage->setPrevPage(unpinPageId);
    returnStatus = dirPage->insertRecord((char *)dpinfop, sizeof(DataPageInfo), allocDataPageRid);
    allocDirPageId = dirPageId;

    // link the previous dir page to the new dir page
    returnStatus = MINIBASE_BM->pinPage(unpinPageId, (Page *&)oldLastPage);
    oldLastPage->setNextPage(dirPageId);

    // unpin all pages used in this function
    returnStatus = MINIBASE_BM->unpinPage(unpinPageId);
    returnStatus = MINIBASE_BM->unpinPage(dirPageId);

    if(returnStatus != OK){
        return returnStatus;
    }
    return OK;
}

// *******************************************
