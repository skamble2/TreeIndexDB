// Assignment 2 Buffer Manager
// Group 20
// Soham Kamble (A20517098) <skamble2@hawk.iit.edu>
// Karan Batra (A20518491)  <kbatra3@hawk.iit.edu>
// Leily Nourbaksh (A20516826) <snoorbakhsh@hawk.iit.edu>

#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

int maxBuffer;
int readIndex;
int writeCount;
int totalNumberOfHits;
typedef struct Page
{
	SM_PageHandle data;
	PageNumber pageNum;
	int is_dirty;
	int current_count;
	int hit_count;
	int freq_count;
} PageF;

/**************************************************************************************
 * Function Name: initBufferPool
 *
 * Description:
 *		Function is used to initialize the buffer pool
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		const char *const pageFileName: file name of the page
 *		const int numPages: number of pages requested
 *		ReplacementStrategy strategy: Replacement strategy
 *		void *stratData: strat data to be used
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
						 const int numPages, ReplacementStrategy strategy,
						 void *stratData)
{
	writeCount = totalNumberOfHits = maxBuffer = readIndex = 0;

	PageF *pages = malloc(sizeof(PageF) * numPages);

	bm->pageFile = (char *)pageFileName;
	bm->mgmtData = pages;
	bm->strategy = strategy;

	int i = 0;
	do
	{
		pages[i].data = NULL;
		pages[i].pageNum = -1;
		pages[i].is_dirty = pages[i].current_count = pages[i].hit_count = pages[i].freq_count = 0;
		i++;
	} while (i < numPages);
	bm->numPages = maxBuffer = numPages;
	printf("Buffer Pool Initialized\n");
	return RC_OK;
}

/**************************************************************************************
 * Function Name: shutdownBufferPool
 *
 * Description:
 *		This function will be used to shut down the buffer pool
 *
 * Parameters:
 *		BM_BufferPool * const bm: Handler for Buffer Pool
 *
 * Return:
 *		RC: return code
 *
 * Author:
 * 		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	PageF *pages = (PageF *)bm->mgmtData;
	forceFlushPool(bm);

	int i;
	i = 0;
	do
	{
		if (pages[i].current_count == 0)
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		}
		i++;
	} while (i < maxBuffer);

	free(pages);
	bm->mgmtData = NULL;
	return RC_OK;
}
/**************************************************************************************
 * Function Name: forceFlushPool
 *
 * Description:
 *		Function is used to force flush the buffer pool
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC forceFlushPool(BM_BufferPool *const bm)
{
	PageF *pFrame = (PageF *)bm->mgmtData;
	SM_FileHandle fh;
	if (pFrame != NULL)
	{
		int i = 0;
		do
		{
			if (pFrame[i].is_dirty == 1 && pFrame[i].current_count == 0)
			{
				openPageFile(bm->pageFile, &fh);
				writeBlock(pFrame[i].pageNum, &fh, pFrame[i].data);
				writeCount += 1;
				pFrame[i].is_dirty = 0;
			}
			i++;
		} while (i < maxBuffer);
		printf("forced flush pool");
		return RC_OK;
	}
	else
	{
		printf("error in forceFlushPool");
		return RC_ERROR_IN_FLUSH_POOL;
	}
}

/**************************************************************************************
 * Function Name: markDirty
 *
 * Description:
 *		This function will mark the requested page as dirty.
 *
 * Parameters:
 *		BM_BufferPool * const bm: Handler for Buffer Pool
 *		BM_PageHandle * const page: Handler for Buffer Page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 * 		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageF *pFrame = (PageF *)bm->mgmtData;
	int pageNum = page->pageNum;
	if (pFrame != NULL)
	{
		int i = 0;
		do
		{
			if (pFrame[i].pageNum == pageNum)
			{
				pFrame[i].is_dirty = 1;
				printf("marked buffer as dirty\n");
				return RC_OK;
			}
			i++;
		} while (i < maxBuffer);
		printf("error in markDirty");
		return RC_ERROR;
	}
	else
	{
		printf("error in markDirty");
		return RC_ERROR;
	}
}
/**************************************************************************************
 * Function Name: unpinPage
 *
 * Description:
 *		Function is used to upin the pafe from the buffer
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		BM_PageHandle *const page: Handler of Buffer page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageF *pFrame = (PageF *)bm->mgmtData;
	int pageNumberPointer = page->pageNum;
	int i = 0;
	do
	{
		if (pFrame[i].pageNum == pageNumberPointer)
		{
			pFrame[i].current_count -= 1;
			break;
		}
		i++;
	} while (i < maxBuffer);
	printf("page upined successfully");
	return RC_OK;
}

/**************************************************************************************
 * Function Name: forcePage
 *
 * Description:
 *		This function will write the page that is requested back to the file present on disk
 *
 * Parameters:
 *		BM_BufferPool * const bm: Handler for Buffer Pool
 *		BM_PageHandle * const page: Handler for Buffer Page
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PageF *pFrame = (PageF *)bm->mgmtData;
	SM_FileHandle fh;
	int pageNumberPointer = page->pageNum;
	int i = 0;
	do
	{
		if (pFrame[i].pageNum == pageNumberPointer)
		{
			if (openPageFile(bm->pageFile, &fh) == 0)
			{
				if (writeBlock(pFrame[i].pageNum, &fh, pFrame[i].data) == 0)
				{
					pFrame[i].is_dirty = 0;
					writeCount += 1;
				}
			}
		}
		i++;
	} while (i < maxBuffer);
	printf("force page successful");
	return RC_OK;
}

/**************************************************************************************
 * Function Name: FIFO
 *
 * Description:
 *		Function is used to implement First In First Out Strategy
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		void
 *
 * Author:
 *		Karan Batra <kbatra3@hawk.iit.edu> (A20518491)
 **************************************************************************************/

extern void FIFO(BM_BufferPool *const bm, PageF *page)
{

	PageF *pFrame = (PageF *)bm->mgmtData;
	int fileIndex = readIndex % maxBuffer;
	int fileBufferPool = fileIndex % maxBuffer;
	SM_FileHandle fh;

	int i = 0;
	do
	{
		if (pFrame[fileIndex].current_count != 0)
		{
			fileIndex += 1;
			fileIndex = (fileBufferPool == 0) ? 0 : fileIndex;
		}
		else
		{
			if (pFrame[fileIndex].is_dirty == 1)
			{
				if (openPageFile(bm->pageFile, &fh) == 0)
				{
					if (writeBlock(pFrame[fileIndex].pageNum, &fh, pFrame[fileIndex].data) == 0)
					{
						writeCount += 1;
					}
				}
			}
			pFrame[fileIndex].current_count = page->current_count;
			pFrame[fileIndex].data = page->data;
			pFrame[fileIndex].is_dirty = page->is_dirty;
			pFrame[fileIndex].pageNum = page->pageNum;

			break;
		}
		i++;
	} while (i < maxBuffer);
}

/**************************************************************************************
 * Function Name: LRU
 *
 * Description:
 *		Function is used to implement Last Recently Used Strategy
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		PageF *page: Pointer to Page file structure
 *
 * Return:
 *		void
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern void LRU(BM_BufferPool *const bm, PageF *page)
{
	PageF *pFrame = (PageF *)bm->mgmtData;
	int LHIndex, LHNumber;

	int i = 0;
	do
	{
		if (pFrame[i].current_count == 0)
		{
			LHIndex = i;
			LHNumber = pFrame[i].hit_count;
			break;
		}
		i++;
	} while (i < maxBuffer);

	i = LHIndex + 1;
	do
	{
		if (pFrame[i].hit_count < LHNumber)
		{
			LHIndex = i;
			LHNumber = pFrame[i].hit_count;
		}
		i++;
	} while (i < maxBuffer);

	if (pFrame[LHIndex].is_dirty == 1)
	{
		SM_FileHandle fh;
		if (openPageFile(bm->pageFile, &fh) == 0)
		{
			if (writeBlock(pFrame[LHIndex].pageNum, &fh, pFrame[LHIndex].data) == 0)
			{
				writeCount++;
			}
		}
	}

	pFrame[LHIndex].data = page->data;
	pFrame[LHIndex].current_count = page->current_count;
	pFrame[LHIndex].is_dirty = page->is_dirty;
	pFrame[LHIndex].hit_count = page->hit_count;
	pFrame[LHIndex].pageNum = page->pageNum;
}


/**************************************************************************************
 * Function Name: pinPage
 *
 * Description:
 *		Function is used to pin a page to the page file system
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		BM_PageHandle *const page: Handler for page file
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		RC: return code
 *
 * Author:
 *		Soham Kamble <skamble2@hawk.iit.edu> (A20517098)
 **************************************************************************************/

extern RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
				  const PageNumber pageNum)
{
	PageF *pages = (PageF *)bm->mgmtData;

	if (pages[0].pageNum < 0)
	{
		SM_FileHandle fh;
		openPageFile(bm->pageFile, &fh);
		pages[0].data = (SM_PageHandle)malloc(PAGE_SIZE);
		ensureCapacity(pageNum, &fh);
		readBlock(pageNum, &fh, pages[0].data);
		pages[0].pageNum = pageNum;
		pages[0].current_count++;
		readIndex = totalNumberOfHits = 0;
		pages[0].hit_count = totalNumberOfHits;
		pages[0].freq_count = 0;
		page->pageNum = pageNum;
		page->data = pages[0].data;

		return RC_OK;
	}
	else
	{
		int i;
		bool isBufferFull = true;

		i = 0;
		do
		{
			if (!(pages[i].pageNum == -1))
			{
				if (pages[i].pageNum == pageNum)
				{
					pages[i].current_count++;
					isBufferFull = false;
					totalNumberOfHits++;

					if (bm->strategy == RS_LRU)
					{
						pages[i].hit_count = totalNumberOfHits;
					}

					page->pageNum = pageNum;
					page->data = pages[i].data;

					break;
				}
			}
			else
			{
				SM_FileHandle fh;
				openPageFile(bm->pageFile, &fh);
				pages[i].data = (SM_PageHandle)malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, pages[i].data);
				pages[i].pageNum = pageNum;
				pages[i].current_count = 1;
				pages[i].freq_count = 0;
				readIndex++;
				totalNumberOfHits++;
				pages[i].hit_count = (bm->strategy == RS_LRU) ? totalNumberOfHits : ((bm->strategy == RS_CLOCK) ? 1 : pages[i].hit_count);

				page->pageNum = pageNum;
				page->data = pages[i].data;

				isBufferFull = false;
				break;
			}

			i++;
		} while (i < maxBuffer);

		if (isBufferFull)
		{
			PageF *newPage = (PageF *)malloc(sizeof(PageF));

			SM_FileHandle fh;
			openPageFile(bm->pageFile, &fh);
			newPage->data = (SM_PageHandle)malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, newPage->data);
			newPage->pageNum = pageNum;
			newPage->is_dirty = 0;
			newPage->current_count = 1;
			newPage->freq_count = 0;
			readIndex++;
			totalNumberOfHits++;

			bm->strategy == RS_LRU ? newPage->hit_count = totalNumberOfHits : bm->strategy == RS_CLOCK ? newPage->hit_count = 1
																									   : 0;

			page->pageNum = pageNum;
			page->data = newPage->data;

			if (bm->strategy == RS_FIFO)
			{
				FIFO(bm, newPage);
			}
			else if (bm->strategy == RS_LRU)
			{
				LRU(bm, newPage);
			}

			else if (bm->strategy == RS_LRU_K)
			{
				printf("\n LRU-k algorithm not implemented");
			}
			else
			{
				printf("\nAlgorithm Not Implemented\n");
			}
		}
		return RC_OK;
	}
}

/**************************************************************************************
 * Function Name: getFrameContents
 *
 * Description:
 *		Function is used to fetch the frame contents
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		page Number of the requested page
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern PageNumber *getFrameContents(BM_BufferPool *const bm)
{
	PageNumber *fData = malloc(sizeof(PageNumber) * maxBuffer);
	PageF *pFrame = (PageF *)bm->mgmtData;

	int i = 0;
	do
	{
		fData[i] = (pFrame[i].pageNum != -1) ? pFrame[i].pageNum : NO_PAGE;
		i++;
	} while (i < maxBuffer);
	printf("file data returned successfully");
	return fData;
}

/**************************************************************************************
 * Function Name: getDirtyFlags
 *
 * Description:
 *		Function is used to get the dirty flags
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		Flag
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern bool *getDirtyFlags(BM_BufferPool *const bm)
{
	bool *Flags = malloc(sizeof(bool) * maxBuffer);
	PageF *pFrame = (PageF *)bm->mgmtData;

	int i = 0;
	do
	{
		Flags[i] = (pFrame[i].is_dirty == 1) ? true : false;
		i++;
	} while (i < maxBuffer);

	printf("flags returned successfully\n");
	return Flags;
}

/**************************************************************************************
 * Function Name: getFixCounts
 *
 * Description:
 *		Function is used to get the number of fix counts
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		number of fix counts
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern int *getFixCounts(BM_BufferPool *const bm)
{
	int *fixCounts = malloc(sizeof(int) * maxBuffer);
	PageF *pFrame = (PageF *)bm->mgmtData;

	int i = 0;
	do
	{
		fixCounts[i] = (pFrame[i].current_count != -1) ? pFrame[i].current_count : 0;
		i++;
	} while (i < maxBuffer);
	printf("fixCounts returned");
	return fixCounts;
}

/**************************************************************************************
 * Function Name: getNumReadIO
 *
 * Description:
 *		Function is used to get the number of read operations.
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		number of read operations
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern int getNumReadIO(BM_BufferPool *const bm)
{
	printf("read index: %d", readIndex + 1);
	return (readIndex + 1);
}

/**************************************************************************************
 * Function Name: getNumWriteIO
 *
 * Description:
 *		Function is used to get the number of write operations.
 *
 * Parameters:
 *		BM_BufferPool *const bm: Handler for Buffer pool
 *		pageNumber *page: page number of the page that is requested
 *
 * Return:
 *		void
 *
 * Author:
 *		Leily Nourbaksh <snoorbakhsh@hawk.iit.edu> (A20516826)
 **************************************************************************************/

extern int getNumWriteIO(BM_BufferPool *const bm)
{
	printf("write count: %d", writeCount);
	return writeCount;
}